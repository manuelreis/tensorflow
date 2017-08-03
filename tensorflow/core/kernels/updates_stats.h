#ifndef UPDATES_STATS_H
#define UPDATES_STATS_H 1

#  include <stdio.h>

# define CACHE_LINE_SIZE 64

typedef struct padded_statistics {
    unsigned long number_of_updates;
    float update_time;
    float lock_waiting_time;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;

// (dleoni) The matrix containing statistics about the application of the update, with:
// - one row for each update phase type (scatter_op and training_ops)
// - one column for each thread
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t stats_array[2][80];

// (dleoni) The local thread unique identifier; used to track the 
// per-thread statistics about the update operations
extern __thread unsigned local_thread_id;

#  define print_updates_stats() { \
	int j= 0; \
	float absolute_average_update_duration = 0.0f; \
	float absolute_maximum_update_duration = 0.0f; \
	float absolute_average_lock_waiting = 0.0f; \
	float absolute_maximum_lock_waiting = 0.0f; \
	int actual_updates_type_number = 0; \
	for (; j < 2; j++) { \
		float maximum_update_duration = 0.0f; \
		float total_average_update_time = 0.0f; \
		float average_update_duration = 0.0f; \
		float maximum_lock_waiting = 0.0f; \
		float total_average_lock_waiting = 0.0f; \
		float average_lock_waiting = 0.0f; \
       		int i = 0; \
		int threads_count = 0; \
        	for (; i < 80; i++) { \
            		if (stats_array[j][i].number_of_updates == 0) { continue; } \
			float average_update_time = stats_array[j][i].update_time / stats_array[j][i].number_of_updates; \
			float average_lock_waiting_time = stats_array[j][i].lock_waiting_time / stats_array[j][i].number_of_updates; \
			total_average_update_time += average_update_time; \
			total_average_lock_waiting += average_lock_waiting_time; \
			if (stats_array[j][i].update_time > maximum_update_duration) { \
				maximum_update_duration = stats_array[j][i].update_time; \
			} \
			if (stats_array[j][i].lock_waiting_time > maximum_lock_waiting) { \
				maximum_lock_waiting = stats_array[j][i].lock_waiting_time; \
			} \
			threads_count++; \
            	} \
		/*printf("THREADS_COUNT transaction %d:%d\n",j,threads_count); */ \
		if (threads_count) { \
			average_update_duration = (total_average_update_time / threads_count); \
			average_lock_waiting = (total_average_lock_waiting / threads_count); \
			absolute_average_update_duration += average_update_duration; \
			absolute_maximum_update_duration += maximum_update_duration; \
			absolute_average_lock_waiting += average_lock_waiting; \
			absolute_maximum_lock_waiting += maximum_lock_waiting; \
			actual_updates_type_number++; \
		} \
		/*if (j == 0) { \
                       	printf("\nTM statistics for the transactions in training_ops.cc\n"); \
                } \
                else { \
                       	printf("\nTM statistics for the transactions in scatter_op.cc\n"); \
                } */\
		printf("Average update duration(ms): %f\nMaximum update duration(ms): %f\n", average_update_duration, maximum_update_duration); \
		printf("Average lock waiting(ms): %f\nMaximum lock waiting(ms): %f", average_lock_waiting, maximum_lock_waiting); \
		printf("|"); \
	} \
	if (actual_updates_type_number > 0) { \
	        absolute_average_update_duration = (absolute_average_update_duration / actual_updates_type_number); \
	        absolute_average_lock_waiting = (absolute_average_lock_waiting / actual_updates_type_number); \
	} \
	/*printf("Aggregate TM statistics of different types of transactions\n"); */\
	printf("Average update duration(ms): %f\nMaximum update duration(ms): %f\n", absolute_average_update_duration, absolute_maximum_update_duration); \
	printf("Average lock waiting (ms): %f\nMaximum lock waiting(ms): %f", absolute_average_lock_waiting, absolute_maximum_lock_waiting); \
}

#endif

