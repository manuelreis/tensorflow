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

#ifndef TENSORFLOW_PLATFORM_DEFAULT_MUTEX_H_
#define TENSORFLOW_PLATFORM_DEFAULT_MUTEX_H_

// IWYU pragma: private, include "third_party/tensorflow/core/platform/mutex.h"
// IWYU pragma: friend third_party/tensorflow/core/platform/mutex.h

#include <chrono>
#include <condition_variable>
#include <mutex>
#include "tensorflow/core/platform/thread_annotations.h"
//(dleoni) Include the following library in order to print the name of the function using mutexes
#include <iostream>
#include <string.h>
#include <stdio.h>

namespace tensorflow {

#undef mutex_lock

enum LinkerInitialized { LINKER_INITIALIZED };

// A class that wraps around the std::mutex implementation, only adding an
// additional LinkerInitialized constructor interface.
class LOCKABLE mutex : public std::mutex {
 public:
  mutex() {}
  // The default implementation of std::mutex is safe to use after the linker
  // initializations
  explicit mutex(LinkerInitialized x) {}

  //(dleoni) Debug trick: print the name of the function that is using a mutex
  //void lock() ACQUIRE() { std::mutex::lock(); }
  void lock(const char *caller_name) ACQUIRE(caller_name) { if(strcmp(caller_name, "0")) std::cout << "CALLER:" << caller_name << std::endl; std::mutex::lock(); }
  bool try_lock(const char *caller_name) EXCLUSIVE_TRYLOCK_FUNCTION(true) {
    std::cout << "CALLER:" << caller_name << std::endl;
    return std::mutex::try_lock();
  };
  void unlock() RELEASE() { std::mutex::unlock(); }
};

class SCOPED_LOCKABLE mutex_lock : public std::unique_lock<std::mutex> {
 public:
  //(dleoni) Debug trick: print the name of the function that is using a mutex
  mutex_lock(class mutex& m) ACQUIRE(m) : std::unique_lock<std::mutex>(m) {}
  mutex_lock(class mutex& m, std::try_to_lock_t t) ACQUIRE(m)
      : std::unique_lock<std::mutex>(m, t) {}
  mutex_lock(mutex_lock&& ml) noexcept
      : std::unique_lock<std::mutex>(std::move(ml)) {}
  mutex_lock(class mutex& m, const char *caller_name) ACQUIRE(m, caller_name) : std::unique_lock<std::mutex>(m) {if(strcmp(caller_name, "0")) std::cout << "CALLER:" << caller_name << std::endl;}
  mutex_lock(class mutex& m, std::try_to_lock_t t, const char *caller_name) ACQUIRE(m, caller_name)
      : std::unique_lock<std::mutex>(m, t) {std::cout << "CALLER:" << caller_name << std::endl;}
  mutex_lock(mutex_lock&& ml, const char *caller_name) noexcept
      : std::unique_lock<std::mutex>(std::move(ml)) {std::cout << "CALLER:" << caller_name << std::endl;}
  ~mutex_lock() RELEASE() {}
};

// Catch bug where variable name is omitted, e.g. mutex_lock (mu);
#define mutex_lock(x) static_assert(0, "mutex_lock_decl_missing_var_name");

using std::condition_variable;

inline ConditionResult WaitForMilliseconds(mutex_lock* mu,
                                           condition_variable* cv, int64 ms) {
  std::cv_status s = cv->wait_for(*mu, std::chrono::milliseconds(ms));
  return (s == std::cv_status::timeout) ? kCond_Timeout : kCond_MaybeNotified;
}

}  // namespace tensorflow

#endif  // TENSORFLOW_PLATFORM_DEFAULT_MUTEX_H_
