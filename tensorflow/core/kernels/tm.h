#ifndef TM_H
#define TM_H 1

#  include <stdio.h>

#ifndef REDUCED_TM_API

#  define MAIN(argc, argv)              int main (int argc, char** argv)
#  define MAIN_RETURN(val)              return val

#  define GOTO_SIM()                    /* nothing */
#  define GOTO_REAL()                   /* nothing */
#  define IS_IN_SIM()                   (0)

#  define SIM_GET_NUM_CPU(var)          /* nothing */

#  define TM_PRINTF                     printf
#  define TM_PRINT0                     printf
#  define TM_PRINT1                     printf
#  define TM_PRINT2                     printf
#  define TM_PRINT3                     printf

#  define P_MEMORY_STARTUP(numThread)   /* nothing */
#  define P_MEMORY_SHUTDOWN()           /* nothing */

#  include <assert.h>
#  include <math.h>

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

#  define P_MALLOC(size)                malloc(size)
#  define P_FREE(ptr)                   free(ptr)
#  define TM_MALLOC(size)               malloc(size)
#  define FAST_PATH_FREE(ptr)            free(ptr)
#  define SLOW_PATH_FREE(ptr)             free(ptr)

#  define SETUP_NUMBER_TASKS(n)
#  define SETUP_NUMBER_THREADS(n)
#  define PRINT_STATS()
#  define AL_LOCK(idx)

#endif

#ifdef REDUCED_TM_API
#    define local_thread_d()         get_tid()
#else
#    define local_thread_d()         thread_getId()
#endif

#  include <immintrin.h>
#  include <rtmintrin.h>
#  include "tensorflow/core/platform/mutex.h"

# define CACHE_LINE_SIZE 64

typedef struct padded_statistics {
    unsigned long commits;
    unsigned long aborts;
    unsigned long commits_with_lock;
    unsigned long explicit_aborts;
    unsigned long conflict_aborts;
    unsigned long capacity_aborts;
    unsigned long other_aborts;
    unsigned long nested_aborts;
    unsigned long debug_aborts;
    unsigned long retry_aborts;
    unsigned long number_of_transactions;
    unsigned long number_of_operations;
    float total_transactions_time;
    float minimum_transaction_time;
    float maximum_transaction_time;
    float total_operations_time;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;

// (dleoni) The matrix containing statistics about transactions and operations with:
// - one row for each transaction type
// - one column for each thread
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t stats_array[2][80];

extern __thread int htm_budget;

// (dleoni) The local thread unique identifier; used to track the 
// per-thread statistics about transactions
extern __thread unsigned local_thread_id;

#  define TM_STARTUP(numThread, bId)
#  define TM_SHUTDOWN() { \
	int j= 0; \
	float absolute_minimum_transaction_duration = 0.0f; \
	float absolute_maximum_transaction_duration = 0.0f; \
	float absolute_average_transaction_duration = 0.0f; \
	float absolute_average_operation_duration = 0.0f; \
	float absolute_maximum_operation_duration = 0.0f; \
	unsigned long absolute_commits = 0; \
	unsigned long absolute_aborts = 0; \
	unsigned long absolute_commits_with_lock = 0; \
	unsigned long absolute_explicit_aborts = 0; \
	unsigned long absolute_conflict_aborts = 0; \
	unsigned long absolute_capacity_aborts = 0; \
	unsigned long absolute_other_aborts = 0; \
	unsigned long absolute_nested_aborts = 0; \
	unsigned long absolute_debug_aborts = 0; \
	unsigned long absolute_retry_aborts = 0; \
	int actual_transactions_type_number = 0; \
	for (; j < 2; j++) { \
	        unsigned long commits = 0; \
        	unsigned long aborts = 0; \
        	unsigned long commits_with_lock = 0; \
        	unsigned long explicit_aborts = 0; \
        	unsigned long conflict_aborts = 0; \
        	unsigned long capacity_aborts = 0; \
        	unsigned long other_aborts = 0; \
		unsigned long nested_aborts = 0; \
		unsigned long debug_aborts = 0; \
		unsigned long retry_aborts = 0; \
		float minimum_transaction_duration = 0.0f; \
		float maximum_transaction_duration = 0.0f; \
		float maximum_operation_duration = 0.0f; \
		float total_average_transaction_time = 0.0f; \
		float total_average_operation_time = 0.0f; \
		float average_transaction_duration = 0.0f; \
		float average_operation_duration = 0.0f; \
       		int i = 0; \
		int threads_count = 0; \
        	for (; i < 80; i++) { \
            		if (stats_array[j][i].commits + stats_array[j][i].commits_with_lock == 0) { continue; } \
                	commits += stats_array[j][i].commits; \
                	aborts += stats_array[j][i].aborts; \
                	commits_with_lock += stats_array[j][i].commits_with_lock; \
                	explicit_aborts += stats_array[j][i].explicit_aborts; \
                	conflict_aborts += stats_array[j][i].conflict_aborts; \
                	capacity_aborts += stats_array[j][i].capacity_aborts; \
                	other_aborts += stats_array[j][i].other_aborts; \
			nested_aborts += stats_array[j][i].nested_aborts; \
  			debug_aborts += stats_array[j][i].debug_aborts; \
			retry_aborts += stats_array[j][i].retry_aborts; \
			float average_transaction_time = stats_array[j][i].total_transactions_time / stats_array[j][i].number_of_transactions; \
			float average_operation_time = stats_array[j][i].total_operations_time / stats_array[j][i].number_of_operations; \
			total_average_transaction_time += average_transaction_time; \
			total_average_operation_time += average_operation_time; \
			if ((minimum_transaction_duration == 0.0f) || (stats_array[j][i].minimum_transaction_time < minimum_transaction_duration)) { \
				minimum_transaction_duration = stats_array[j][i].minimum_transaction_time; \
			} \
			if (stats_array[j][i].maximum_transaction_time > maximum_transaction_duration) { \
				maximum_transaction_duration = stats_array[j][i].maximum_transaction_time; \
			} \
			if (stats_array[j][i].total_operations_time > maximum_operation_duration) { \
				maximum_operation_duration = stats_array[j][i].total_operations_time; \
			} \
			threads_count++; \
            	} \
		/*printf("THREADS_COUNT transaction %d:%d\n",j,threads_count); */ \
		if (threads_count) { \
			average_transaction_duration = (total_average_transaction_time / threads_count); \
			average_operation_duration = (total_average_operation_time / threads_count); \
			absolute_average_transaction_duration += average_transaction_duration; \
			absolute_average_operation_duration += average_operation_duration; \
			if ((absolute_minimum_transaction_duration == 0.0f) || (minimum_transaction_duration < absolute_minimum_transaction_duration)) { \
				absolute_minimum_transaction_duration = minimum_transaction_duration; \
			} \
			if (maximum_transaction_duration > absolute_maximum_transaction_duration) { \
				absolute_maximum_transaction_duration = maximum_transaction_duration; \
			} \
			absolute_maximum_operation_duration += maximum_operation_duration; \
			actual_transactions_type_number++; \
		} \
		/*if (j == 0) { \
                       	printf("\nTM statistics for the transactions in training_ops.cc\n"); \
                } \
                else { \
                       	printf("\nTM statistics for the transactions in scatter_op.cc\n"); \
                } */\
	        printf("TM commits: %lu\nTotal aborts: %lu\nTotal Lock Commits: %lu\nExplicit Aborts: %lu\nConflict Aborts: %lu\nCapacity Aborts: %lu\nOther Aborts: %lu\nNested Aborts: %lu\nDebug Aborts: %lu\nRetry Aborts: %lu\n", commits, aborts, commits_with_lock, explicit_aborts, conflict_aborts, capacity_aborts, other_aborts, nested_aborts, debug_aborts, retry_aborts); \
		printf("Average transaction duration(ms): %f\nMinimum transaction duration(ms): %f\nMaximum transaction duration(ms): %f\n", average_transaction_duration, minimum_transaction_duration, maximum_transaction_duration); \
		printf("Average operation duration(ms): %f\nMaximum operation duration(ms): %f", average_operation_duration, maximum_operation_duration); \
		printf("|"); \
		absolute_commits += commits; \
                absolute_aborts += aborts; \
                absolute_commits_with_lock += commits_with_lock; \
                absolute_explicit_aborts += explicit_aborts; \
                absolute_conflict_aborts += conflict_aborts; \
                absolute_capacity_aborts += capacity_aborts; \
                absolute_other_aborts += other_aborts; \
		absolute_nested_aborts += nested_aborts; \
                absolute_debug_aborts += debug_aborts; \
                absolute_retry_aborts += retry_aborts; \
	} \
	if (actual_transactions_type_number > 0) { \
	        absolute_average_transaction_duration = (absolute_average_transaction_duration / actual_transactions_type_number); \
	        absolute_average_operation_duration = (absolute_average_operation_duration / actual_transactions_type_number); \
	} \
	/*printf("Aggregate TM statistics of different types of transactions\n"); */\
	printf("TM commits: %lu\nTotal aborts: %lu\nTotal Lock Commits: %lu\nExplicit Aborts: %lu\nConflict Aborts: %lu\nCapacity Aborts: %lu\nOther Aborts: %lu\nNested Aborts: %lu\nDebug Aborts: %lu\nRetry Aborts: %lu\n", absolute_commits, absolute_aborts, absolute_commits_with_lock, absolute_explicit_aborts, absolute_conflict_aborts, absolute_capacity_aborts, absolute_other_aborts, absolute_nested_aborts, absolute_debug_aborts, absolute_retry_aborts); \
	printf("Average transaction duration(ms): %f\nMinimum transaction duration(ms): %f\nMaximum transaction duration(ms): %f\n", absolute_average_transaction_duration, absolute_minimum_transaction_duration, absolute_maximum_transaction_duration); \
	printf("Average operation duration(ms): %f\nMaximum operation duration(ms): %f", absolute_average_operation_duration, absolute_maximum_operation_duration); \
}

# define TM_THREAD_ENTER()
# define TM_THREAD_EXIT()

# define IS_LOCKED(mutex)        mutex->is_locked()

# define RETRY_POLICY 5
# define HTM_RETRIES 5
# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define TM_BEGIN(mutex, transaction) { \
        while (1) { \
            while (IS_LOCKED(mutex)) { \
                __asm__ ( "pause;"); \
            } \
            unsigned int status = _xbegin(); \
            if (status == _XBEGIN_STARTED) { \
                if (IS_LOCKED(mutex)) { \
                    _xabort(30); \
                } \
                break; \
            } \
            else if (status == _XABORT_CAPACITY) { \
                stats_array[transaction][local_thread_id].capacity_aborts++;\
                SPEND_BUDGET(&htm_budget); \
            } else \
            { \
                if (status & _XABORT_CONFLICT) {\
                        stats_array[transaction][local_thread_id].conflict_aborts++;\
                }\
                else if (status & _XABORT_EXPLICIT) {\
                        stats_array[transaction][local_thread_id].explicit_aborts++;\
                }\
                else if (status & _XABORT_NESTED) {\
                        stats_array[transaction][local_thread_id].nested_aborts++;\
                }\
                else if (status & _XABORT_DEBUG) {\
                        stats_array[transaction][local_thread_id].debug_aborts++;\
                }\
                else if (status & _XABORT_RETRY) {\
                        stats_array[transaction][local_thread_id].retry_aborts++;\
                }\
                else {\
                        stats_array[transaction][local_thread_id].other_aborts++;\
                }\
                htm_budget--; \
            } \
            stats_array[transaction][local_thread_id].aborts++; \
            if (htm_budget <= 0) {   \
		mutex->lock(); \
		break; \
            } \
        } \
};


# define TM_END(mutex, transaction, transaction_start_time){ \
    if (htm_budget > 0) { \
        _xend(); \
        stats_array[transaction][local_thread_id].commits++; \
    } else {    \
        mutex->unlock(); \
        stats_array[transaction][local_thread_id].commits_with_lock++; \
    } \
    auto transaction_end_time = std::chrono::system_clock::now(); \
    float transaction_duration = std::chrono::duration<float , std::milli>(transaction_end_time - transaction_start_time).count(); \
    stats_array[transaction][local_thread_id].total_transactions_time += transaction_duration; \
    stats_array[transaction][local_thread_id].number_of_transactions++; \
     if ((stats_array[transaction][local_thread_id].minimum_transaction_time == 0.0f) || (transaction_duration < stats_array[transaction][local_thread_id].minimum_transaction_time)) { \
         stats_array[transaction][local_thread_id].minimum_transaction_time = transaction_duration; \
     } \
     if (transaction_duration > stats_array[transaction][local_thread_id].maximum_transaction_time) { \
         stats_array[transaction][local_thread_id].maximum_transaction_time = transaction_duration; \
     } \
};

#    define TM_BEGIN_RO()                 TM_BEGIN(0)
#    define TM_RESTART()                  _xabort(0xab);;
#    define TM_EARLY_RELEASE(var)

# define FAST_PATH_RESTART() _xabort(0xab);
# define FAST_PATH_SHARED_READ(var) (var)
# define FAST_PATH_SHARED_READ_P(var) (var)
# define FAST_PATH_SHARED_READ_D(var) (var)
# define FAST_PATH_SHARED_WRITE(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_D(var, val) ({var = val; var;})

# define SLOW_PATH_RESTART() FAST_PATH_RESTART()
# define SLOW_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define SLOW_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define SLOW_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define SLOW_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define SLOW_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define SLOW_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define SLOW_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})


#endif

