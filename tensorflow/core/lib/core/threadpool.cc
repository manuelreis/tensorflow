/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/core/lib/core/threadpool.h"

#define EIGEN_USE_THREADS
#include "third_party/eigen3/unsupported/Eigen/CXX11/Tensor"
#include "tensorflow/core/platform/context.h"
#include "tensorflow/core/platform/denormal.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/setround.h"
#include "tensorflow/core/platform/tracing.h"
#include "tensorflow/core/platform/types.h"
// (dleoni) Include header for the thread pinning
#include <sched.h>
#include "tensorflow/core/platform/cpu_info.h"

// (dleoni) The local thread unique identifier
__thread unsigned local_thread_id(0);

namespace tensorflow {
namespace thread {

struct EigenEnvironment {
  typedef Thread EnvThread;
  struct TaskImpl {
    std::function<void()> f;
    Context context;
    uint64 trace_id;
  };
  struct Task {
    std::unique_ptr<TaskImpl> f;
  };

  Env* const env_;
  const ThreadOptions thread_options_;
  const string name_;
  // (dleoni) Set two counters to determine the core whom the next thread
  // for both the intra and the inter parallelism pools will be pinned to
  // The main process is the only one that increments those variables when
  // it creates the two pools, so we don't need any synchronization
  unsigned intra_pool_next_core_;
  unsigned inter_pool_next_core_;
  

  EigenEnvironment(Env* env, const ThreadOptions& thread_options,
                   const string& name)
      : env_(env), thread_options_(thread_options), name_(name), intra_pool_next_core_(0), inter_pool_next_core_(std::thread::hardware_concurrency()-1) {}

  EnvThread* CreateThread(std::function<void()> f) {
    return env_->StartThread(thread_options_, name_, [=, &local_thread_id]() {
      unsigned core = 0;
      unsigned num_cores = std::thread::hardware_concurrency();
      // Set the processor flag to flush denormals to zero.
      port::ScopedFlushDenormal flush;
      // Set the processor rounding mode to ROUND TO NEAREST.
      port::ScopedSetRound round(FE_TONEAREST);
      // (dleoni) Check if the thread belongs to the intra-op or to the inter-op pool
      if (name_ == "tf_Eigen") {
        // (dleoni) The thread belongs to the intra-op pool => get the number of core
        // from the corresponding variable
        core = intra_pool_next_core_;
        // Update the counter for the next core to be used for this pool
        intra_pool_next_core_ = (intra_pool_next_core_ + 1) % num_cores;
      }
      else {
        core = inter_pool_next_core_;
        // Update the counter for the next core to be used for this pool
        inter_pool_next_core_ = inter_pool_next_core_ - 1;
        if (inter_pool_next_core_ < 0) {
          inter_pool_next_core_ = num_cores -1;
        }
      }
      // (dleoni) Pin the thread to the core indicated in the field "core"
      int ret = 0;
      cpu_set_t new_cpu_set;
      CPU_ZERO(&new_cpu_set);
      CPU_SET(core, &new_cpu_set);
      ret = sched_setaffinity(0, sizeof(cpu_set_t), &new_cpu_set);
      if (ret != 0) {
        VLOG(WARNING) << "sched_setaffinity";
      }
      // (dleoni) Set the local thread identifier to the index of the core it
      // is pinned to; this is used to capture statistics about the transactions
      // performed by this thread 
      local_thread_id = core;
      f();
    });
  }

  Task CreateTask(std::function<void()> f) {
    uint64 id = 0;
    if (port::Tracing::IsActive()) {
      id = port::Tracing::UniqueId();
      port::Tracing::RecordEvent(port::Tracing::EventCategory::kScheduleClosure,
                                 id);
    }
    return Task{
        std::unique_ptr<TaskImpl>(new TaskImpl{
            std::move(f), Context(ContextKind::kThread), id,
        }),
    };
  }

  void ExecuteTask(const Task& t) {
    WithContext wc(t.f->context);
    if (t.f->trace_id != 0) {
      port::Tracing::ScopedActivity region(
          port::Tracing::EventCategory::kRunClosure, t.f->trace_id);
      t.f->f();
    } else {
      t.f->f();
    }
  }
};

struct ThreadPool::Impl : Eigen::ThreadPoolTempl<EigenEnvironment> {
  Impl(Env* env, const ThreadOptions& thread_options, const string& name,
       int num_threads, bool low_latency_hint)
      : Eigen::ThreadPoolTempl<EigenEnvironment>(
            num_threads, low_latency_hint,
            EigenEnvironment(env, thread_options, name)) {}

  void ParallelFor(int64 total, int64 cost_per_unit,
                   std::function<void(int64, int64)> fn) {
    CHECK_GE(total, 0);
    CHECK_EQ(total, (int64)(Eigen::Index)total);
    Eigen::ThreadPoolDevice device(this, this->NumThreads());
    device.parallelFor(
        total, Eigen::TensorOpCost(0, 0, cost_per_unit),
        [&fn](Eigen::Index first, Eigen::Index last) { fn(first, last); });
  }
};

ThreadPool::ThreadPool(Env* env, const string& name, int num_threads)
    : ThreadPool(env, ThreadOptions(), name, num_threads, true) {}

ThreadPool::ThreadPool(Env* env, const ThreadOptions& thread_options,
                       const string& name, int num_threads)
    : ThreadPool(env, thread_options, name, num_threads, true) {}

ThreadPool::ThreadPool(Env* env, const ThreadOptions& thread_options,
                       const string& name, int num_threads,
                       bool low_latency_hint) {
  cpu_set_t new_cpu_set;
  cpu_set_t old_cpu_set;
  int ret = 0;
  CHECK_GE(num_threads, 1);
  // (dleoni) Check if the main thread has already been pinned to a core: if not,
  // pin it to the core 0
  ret = sched_getaffinity(0, sizeof(cpu_set_t), &old_cpu_set);
  if (ret != 0) {
    VLOG(WARNING) << "sched_getaffinity";
  }
  if (!CPU_COUNT(&old_cpu_set) || CPU_COUNT(&old_cpu_set) > 1) {
    // (dleoni) The main thread has not been pinned yet, so pin it to the current
    // core now 
    CPU_ZERO(&new_cpu_set);
    CPU_SET(0, &new_cpu_set);
    ret = sched_setaffinity(0, sizeof(cpu_set_t), &new_cpu_set);
  }
  if (ret != 0) {
    VLOG(WARNING) << "sched_setaffinity";
  }
  impl_.reset(new ThreadPool::Impl(env, thread_options, "tf_" + name,
                                   num_threads, low_latency_hint));
}

ThreadPool::~ThreadPool() {}

void ThreadPool::Schedule(std::function<void()> fn) {
  CHECK(fn != nullptr);
  impl_->Schedule(std::move(fn));
}

void ThreadPool::ParallelFor(int64 total, int64 cost_per_unit,
                             std::function<void(int64, int64)> fn) {
  impl_->ParallelFor(total, cost_per_unit, std::move(fn));
}

void ThreadPool::ParallelForWithWorkerId(
    int64 total, int64 cost_per_unit,
    const std::function<void(int64, int64, int)>& fn) {
  impl_->ParallelFor(total, cost_per_unit,
                     [this, &fn](int64 start, int64 limit) {
                       // ParallelFor may use the current thread to do some
                       // work synchronously. When calling CurrentThreadId()
                       // from outside of the thread pool, we get -1, so we can
                       // shift every id up by 1.
                       int id = CurrentThreadId() + 1;
                       fn(start, limit, id);
                     });
}

int ThreadPool::NumThreads() const { return impl_->NumThreads(); }

int ThreadPool::CurrentThreadId() const { return impl_->CurrentThreadId(); }

}  // namespace thread
}  // namespace tensorflow
