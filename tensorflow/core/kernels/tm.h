#ifndef TM_H
#define TM_H 1

#  include <stdio.h>
#  include <assert.h>
#  include <math.h>
#  include <htmxlintrin.h>
#  include "tensorflow/core/platform/mutex.h"

# define CACHE_LINE_SIZE 128

typedef struct padded_statistics {
    unsigned long tle_commits;
    unsigned long gl_commits;
    unsigned long aborts;
    unsigned long conflicts;
    unsigned long self_conflicts;
    unsigned long trans_conflicts;
    unsigned long nontrans_conflicts;
    unsigned long capacity_aborts;
    unsigned long other_aborts;
    unsigned long user_aborts;
    unsigned long persistent_failures;
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

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_self_conflict(void* const TM_buff)
{ 
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_SELF_INDUCED_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_trans_conflict(void* const TM_buff)
{ 
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_TRANSACTION_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_nontrans_conflict(void* const TM_buff)
{ 
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}

#  define TM_SHUTDOWN() { \
	int j= 0; \
	float absolute_minimum_transaction_duration = 0.0f; \
	float absolute_maximum_transaction_duration = 0.0f; \
	float absolute_average_transaction_duration = 0.0f; \
	float absolute_average_operation_duration = 0.0f; \
	float absolute_maximum_operation_duration = 0.0f; \
	unsigned long absolute_tle_commits = 0; \
	unsigned long absolute_gl_commits = 0; \
	unsigned long absolute_aborts = 0; \
	unsigned long absolute_conflicts = 0; \
	unsigned long absolute_self_conflicts = 0; \
	unsigned long absolute_trans_conflicts = 0; \
	unsigned long absolute_nontrans_conflicts = 0; \
	unsigned long absolute_capacity_aborts = 0; \
	unsigned long absolute_other_aborts = 0; \
	unsigned long absolute_user_aborts = 0; \
	unsigned long absolute_persistent_failures = 0; \
	int actual_transactions_type_number = 0; \
	for (; j < 2; j++) { \
	        unsigned long tle_commits = 0; \
	        unsigned long gl_commits = 0; \
        	unsigned long aborts = 0; \
        	unsigned long conflicts = 0; \
        	unsigned long self_conflicts = 0; \
        	unsigned long trans_conflicts = 0; \
        	unsigned long nontrans_conflicts = 0; \
        	unsigned long capacity_aborts = 0; \
        	unsigned long other_aborts = 0; \
		unsigned long user_aborts = 0; \
		unsigned long persistent_failures = 0; \
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
            		if (stats_array[j][i].tle_commits + stats_array[j][i].gl_commits == 0) { continue; } \
                	tle_commits += stats_array[j][i].tle_commits; \
                	gl_commits += stats_array[j][i].gl_commits; \
                	aborts += stats_array[j][i].aborts; \
                	conflicts += stats_array[j][i].conflicts; \
                	self_conflicts += stats_array[j][i].self_conflicts; \
                        trans_conflicts += stats_array[j][i].trans_conflicts; \
                        nontrans_conflicts += stats_array[j][i].nontrans_conflicts; \
                	capacity_aborts += stats_array[j][i].capacity_aborts; \
                	other_aborts += stats_array[j][i].other_aborts; \
			user_aborts += stats_array[j][i].user_aborts; \
  			persistent_failures += stats_array[j][i].persistent_failures; \
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
	        printf("Total TLE Commits: %lu\nTotal Global Lock Commits: %lu\nTotal Aborts: %lu\nTotal Conflicts: %lu\nTotal Self-Induced Conflicts: %lu\nTotal Transactional Conflicts: %lu\nTotal Non-Transactional Conflicts: %lu\nTotal Capacity Aborts: %lu\nTotal Other Aborts: %lu\nTotal User Aborts: %lu\nTotal Persistent Failures: %lu\n", tle_commits, gl_commits, aborts, conflicts, self_conflicts, trans_conflicts, nontrans_conflicts, capacity_aborts, other_aborts, user_aborts, persistent_failures); \
		printf("Average transaction duration(ms): %f\nMinimum transaction duration(ms): %f\nMaximum transaction duration(ms): %f\n", average_transaction_duration, minimum_transaction_duration, maximum_transaction_duration); \
		printf("Average operation duration(ms): %f\nMaximum operation duration(ms): %f", average_operation_duration, maximum_operation_duration); \
		printf("|"); \
		absolute_tle_commits += tle_commits; \
		absolute_gl_commits += gl_commits; \
                absolute_aborts += aborts; \
                absolute_conflicts += conflicts; \
                absolute_self_conflicts += self_conflicts; \
                absolute_trans_conflicts += trans_conflicts; \
                absolute_nontrans_conflicts += nontrans_conflicts; \
                absolute_capacity_aborts += capacity_aborts; \
                absolute_other_aborts += other_aborts; \
		absolute_user_aborts += user_aborts; \
                absolute_persistent_failures += persistent_failures; \
	} \
	if (actual_transactions_type_number > 0) { \
	        absolute_average_transaction_duration = (absolute_average_transaction_duration / actual_transactions_type_number); \
	        absolute_average_operation_duration = (absolute_average_operation_duration / actual_transactions_type_number); \
	} \
	/*printf("Aggregate TM statistics of different types of transactions\n"); */\
	printf("Total TLE Commits: %lu\nTotal Global Lock Commits: %lu\nTotal Aborts: %lu\nTotal Conflicts: %lu\nTotal Self-Induced Conflicts: %lu\nTotal Transactional Conflicts: %lu\nTotal Non-Transactional Conflicts: %lu\nTotal Capacity Aborts: %lu\nTotal Other Aborts: %lu\nTotal User Aborts: %lu\nTotal Persistent Failures: %lu\n", absolute_tle_commits, absolute_gl_commits, absolute_aborts, absolute_conflicts, absolute_self_conflicts, absolute_trans_conflicts, absolute_nontrans_conflicts, absolute_capacity_aborts, absolute_other_aborts, absolute_user_aborts, absolute_persistent_failures); \
	printf("Average transaction duration(ms): %f\nMinimum transaction duration(ms): %f\nMaximum transaction duration(ms): %f\n", absolute_average_transaction_duration, absolute_minimum_transaction_duration, absolute_maximum_transaction_duration); \
	printf("Average operation duration(ms): %f\nMaximum operation duration(ms): %f", absolute_average_operation_duration, absolute_maximum_operation_duration); \
}

# define IS_LOCKED(mutex)        mutex->is_locked()

# define RETRY_POLICY 5
# define HTM_RETRIES 5
# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define TM_BEGIN(mutex, transaction) { \
        while (1) { \
            while (IS_LOCKED(mutex)) { \
                __asm volatile ("" : : : "memory"); \
            } \
	    TM_buff_type TM_buff; \
            unsigned int status = __TM_begin(&TM_buff); \
            if (status == _HTM_TBEGIN_STARTED) { \
                if (IS_LOCKED(mutex)) { \
                    __TM_abort(); \
                } \
                break; \
            } \
            else { \
		if(__TM_is_failure_persistent(&TM_buff)){ \
                    SPEND_BUDGET(&htm_budget); \
                    /*stats_array[transaction][local_thread_id].persistent_failures++;*/\
                } \
		if(__TM_is_conflict(&TM_buff)){ \
                        /*stats_array[transaction][local_thread_id].conflicts++; \
                        if(__TM_is_self_conflict(&TM_buff)) {stats_array[transaction][local_thread_id].self_conflicts++; }\
                        else if(__TM_is_trans_conflict(&TM_buff)) stats_array[transaction][local_thread_id].trans_conflicts++; \
                        else if(__TM_is_nontrans_conflict(&TM_buff)) stats_array[transaction][local_thread_id].nontrans_conflicts++; */ \
                        htm_budget--; \
                } \
		else if (__TM_is_user_abort(&TM_buff)) { \
                        /*stats_array[transaction][local_thread_id].user_aborts++; */ \
                        htm_budget--; \
                } \
		else if(__TM_is_footprint_exceeded(&TM_buff)){ \
                        /*stats_array[transaction][local_thread_id].capacity_aborts++; */ \
                        htm_budget--; \
                } \
		else{ \
                        /* stats_array[transaction][local_thread_id].other_aborts++; */ \
                        htm_budget--; \
                } \
            } \
	    /*stats_array[transaction][local_thread_id].aborts++; */ \
            if (htm_budget <= 0) {   \
		mutex->lock(); \
                break; \
            } \
        } \
};


# define TM_END(mutex, transaction, transaction_start_time){ \
    if (htm_budget > 0) { \
        __TM_end; \
        stats_array[transaction][local_thread_id].tle_commits++; \
    } else {    \
        mutex->unlock(); \
        stats_array[transaction][local_thread_id].gl_commits++; \
    } \
    /*auto transaction_end_time = std::chrono::system_clock::now(); \
    float transaction_duration = std::chrono::duration<float , std::milli>(transaction_end_time - transaction_start_time).count(); \
    stats_array[transaction][local_thread_id].total_transactions_time += transaction_duration; \
    stats_array[transaction][local_thread_id].number_of_transactions++; \
     if ((stats_array[transaction][local_thread_id].minimum_transaction_time == 0.0f) || (transaction_duration < stats_array[transaction][local_thread_id].minimum_transaction_time)) { \
         stats_array[transaction][local_thread_id].minimum_transaction_time = transaction_duration; \
     } \
     if (transaction_duration > stats_array[transaction][local_thread_id].maximum_transaction_time) { \
         stats_array[transaction][local_thread_id].maximum_transaction_time = transaction_duration; \
     }*/ \
};

#endif
