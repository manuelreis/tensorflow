#ifndef TM_H
#define TM_H 1

#  include <stdio.h>
#  include <assert.h>
#  include <math.h>
#  include <htmxlintrin.h>
#  include "tensorflow/core/platform/mutex.h"

# define CACHE_LINE_SIZE 128

// (dleoni) Data structure to store statistics related to hardware transactions

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

// (dleoni) The matrix containing statistics about transactions and operations
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t stats_array[80];

// (dleoni) The number of times each thread can try to execute a transaction before choosing the fallback path
extern __thread int htm_budget;

// (dleoni) The local thread unique identifier; used to track the per-thread statistics about transactions
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


#define TM_SHUTDOWN() { \
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
	float maximum_operation_duration = -1.0f; \
	float total_average_transaction_time = 0.0f; \
	float total_average_operation_time = 0.0f; \
	float average_transaction_duration = 0.0f; \
	float average_operation_duration = 0.0f; \
	int i = 0; \
	int threads_count = 0; \
	for (; i < 80; i++) { \
		if (stats_array[i].tle_commits + stats_array[i].gl_commits == 0) { continue; } \
		tle_commits += stats_array[i].tle_commits; \
    		gl_commits += stats_array[i].gl_commits; \
    		aborts += stats_array[i].aborts; \
    		conflicts += stats_array[i].conflicts; \
    		self_conflicts += stats_array[i].self_conflicts; \
    		trans_conflicts += stats_array[i].trans_conflicts; \
    		nontrans_conflicts += stats_array[i].nontrans_conflicts; \
    		capacity_aborts += stats_array[i].capacity_aborts; \
    		other_aborts += stats_array[i].other_aborts; \
		user_aborts += stats_array[i].user_aborts; \
  		persistent_failures += stats_array[i].persistent_failures; \
		float average_transaction_time = stats_array[i].total_transactions_time / stats_array[i].number_of_transactions; \
		float average_operation_time = stats_array[i].total_operations_time / stats_array[i].number_of_operations; \
		total_average_transaction_time += average_transaction_time; \
		total_average_operation_time += average_operation_time; \
		if ((minimum_transaction_duration == 0.0f) || (stats_array[i].minimum_transaction_time < minimum_transaction_duration)) { \
			minimum_transaction_duration = stats_array[i].minimum_transaction_time; \
		} \
		if (stats_array[i].maximum_transaction_time > maximum_transaction_duration) { \
			maximum_transaction_duration = stats_array[i].maximum_transaction_time; \
	} \
		if (stats_array[i].total_operations_time > maximum_operation_duration) { \
			maximum_operation_duration = stats_array[i].total_operations_time; \
	} \
		threads_count++; \
	} \
	if (threads_count) { \
		average_transaction_duration = (total_average_transaction_time / threads_count); \
		average_operation_duration = (total_average_operation_time / threads_count); \
	} \
	printf("Total TLE Commits: %lu\nTotal Global Lock Commits: %lu\nTotal Aborts: %lu\nTotal Conflicts: %lu\nTotal Self-Induced Conflicts: %lu\nTotal Transactional Conflicts: %lu\nTotal Non-Transactional Conflicts: %lu\nTotal Capacity Aborts: %lu\nTotal Other Aborts: %lu\nTotal User Aborts: %lu\nTotal Persistent Failures: %lu\n", tle_commits, gl_commits, aborts, conflicts, self_conflicts, trans_conflicts, nontrans_conflicts, capacity_aborts, other_aborts, user_aborts, persistent_failures); \
	/*printf("Average transaction duration(ms): %f\nMinimum transaction duration(ms): %f\nMaximum transaction duration(ms): %f\n", average_transaction_duration, minimum_transaction_duration, maximum_transaction_duration); \
	printf("Average operation duration(ms): %f\nMaximum operation duration(ms): %f\n", average_operation_duration, maximum_operation_duration);*/ \
}

# define IS_LOCKED(mutex)        mutex->is_locked()

# define RETRY_POLICY 5

# define HTM_RETRIES 5

# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

#endif
