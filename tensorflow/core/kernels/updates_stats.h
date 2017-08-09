#ifndef UPDATES_STATS_H
#define UPDATES_STATS_H 1

#  include <stdio.h>

# define CACHE_LINE_SIZE 64

typedef struct padded_statistics {
    unsigned long number_of_updates;
    float update_time;
    float lock_waiting_time;
    float max_lock_waiting_time;
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
        	int i=0; \
		std::string output = ""; \
		for (; i < 80; i++) { \
            		if (stats_array[0][i].number_of_updates == 0) { continue; } \
			float average_update_time = stats_array[0][i].update_time / stats_array[0][i].number_of_updates; \
			float average_lock_waiting_time = stats_array[0][i].lock_waiting_time / stats_array[0][i].number_of_updates; \
			output += std::to_string(i) + "|" + std::to_string(stats_array[0][i].max_lock_waiting_time); \
			output += ","; \
            	} \
		printf("%s", output.c_str()); \
}

#endif

