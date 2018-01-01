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
    unsigned long conflicts;
    unsigned long self_conflicts;
    unsigned long trans_conflicts;
    unsigned long nontrans_conflicts;
    unsigned long capacity_aborts;
    unsigned long other_aborts;
    unsigned long user_aborts;
    unsigned long persistent_failures;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;

// (dleoni) The matrix containing statistics about transactions and operations with:
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t stats_array[];

extern __thread int htm_budget;

// (dleoni) The local thread unique identifier; used to track the 
// per-thread statistics about transactions
extern __thread int local_thread_id;

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
	        unsigned long tle_commits = 0; \
	        unsigned long gl_commits = 0; \
        	unsigned long conflicts = 0; \
        	unsigned long self_conflicts = 0; \
        	unsigned long trans_conflicts = 0; \
        	unsigned long nontrans_conflicts = 0; \
        	unsigned long capacity_aborts = 0; \
        	unsigned long other_aborts = 0; \
		unsigned long user_aborts = 0; \
		unsigned long persistent_failures = 0; \
       		int i = 0; \
        	for (; i < 80; i++) { \
            		if (stats_array[i].tle_commits + stats_array[i].gl_commits == 0) { continue; } \
                	tle_commits += stats_array[i].tle_commits; \
                	gl_commits += stats_array[i].gl_commits; \
                	conflicts += stats_array[i].conflicts; \
                	self_conflicts += stats_array[i].self_conflicts; \
                        trans_conflicts += stats_array[i].trans_conflicts; \
                        nontrans_conflicts += stats_array[i].nontrans_conflicts; \
                	capacity_aborts += stats_array[i].capacity_aborts; \
                	other_aborts += stats_array[i].other_aborts; \
			user_aborts += stats_array[i].user_aborts; \
  			persistent_failures += stats_array[i].persistent_failures; \
            	} \
	        printf("Total TLE Commits: %lu\nTotal Global Lock Commits: %lu\nTotal Conflicts: %lu\nTotal Self-Induced Conflicts: %lu\nTotal Transactional Conflicts: %lu\nTotal Non-Transactional Conflicts: %lu\nTotal Capacity Aborts: %lu\nTotal Other Aborts: %lu\nTotal User Aborts: %lu\nTotal Persistent Failures: %lu\n", tle_commits, gl_commits, conflicts, self_conflicts, trans_conflicts, nontrans_conflicts, capacity_aborts, other_aborts, user_aborts, persistent_failures); \
}

# define IS_LOCKED(mutex)        mutex->is_locked()

# define RETRY_POLICY 5
# define HTM_RETRIES 100
# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define TM_BEGIN(mutex) { \
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
                    	stats_array[local_thread_id].persistent_failures++; \
			/*printf("SPEND BUDGET:%d\n", htm_budget); \
                        fflush(stdout); \*/ \
                } \
		if(__TM_is_conflict(&TM_buff)){ \
                        stats_array[local_thread_id].conflicts++; \
			if(__TM_is_self_conflict(&TM_buff)) {stats_array[local_thread_id].self_conflicts++; }\
			else if(__TM_is_trans_conflict(&TM_buff)) stats_array[local_thread_id].trans_conflicts++; \
			else if(__TM_is_nontrans_conflict(&TM_buff)) stats_array[local_thread_id].nontrans_conflicts++; \
			htm_budget--; \
			/*printf("ABORT:%d\n", htm_budget); \
                        fflush(stdout); \ */\
                } \
		else if (__TM_is_user_abort(&TM_buff)) { \
			stats_array[local_thread_id].user_aborts++; \
                        htm_budget--; \
			/*printf("USER:%d\n", htm_budget); \
                        fflush(stdout); \ */\
                } \
		else if(__TM_is_footprint_exceeded(&TM_buff)){ \
			stats_array[local_thread_id].capacity_aborts++; \
                        htm_budget--; \
			/*printf("CAPACITY:%d\n", htm_budget); \
                        fflush(stdout); \ */\
                } \
		else{ \
			stats_array[local_thread_id].other_aborts++; \
                        htm_budget--; \
			/*printf("OTHER:%d\n", htm_budget); \
                        fflush(stdout); \ */\
                } \
            } \
            if (htm_budget <= 0) {   \
		mutex->lock(); \
                break; \
            } \
        } \
};


# define TM_END(mutex){ \
    if (htm_budget > 0) { \
        __TM_end; \
        stats_array[local_thread_id].tle_commits++; \
	printf("COMMIT\n"); \
	fflush(stdout); \
    } else {    \
        mutex->unlock(); \
        stats_array[local_thread_id].gl_commits++; \
	printf("NO COMMITTA\n"); \
        fflush(stdout); \	
    } \
};

# define TM_BEGIN_ADAM(mutex1, mutex2, mutex3) { \
        while (1) { \
            while (IS_LOCKED(mutex1) || IS_LOCKED(mutex2) || IS_LOCKED(mutex3)) { \
                __asm volatile ("" : : : "memory"); \
            } \
	    TM_buff_type TM_buff; \
            unsigned int status = __TM_begin(&TM_buff); \
            if (status == _HTM_TBEGIN_STARTED) { \
                if (IS_LOCKED(mutex1) || IS_LOCKED(mutex2) || IS_LOCKED(mutex3)) { \
                    __TM_abort(); \
                } \
                break; \
            } \
            else { \
		if(__TM_is_failure_persistent(&TM_buff)){ \
			SPEND_BUDGET(&htm_budget); \
                    	stats_array[local_thread_id].persistent_failures++; \
			printf("SPEND BUDGET:%d\n", &htm_budget); \
			fflush(stdout); \
                } \
		if(__TM_is_conflict(&TM_buff)){ \
                        stats_array[local_thread_id].conflicts++; \
			if(__TM_is_self_conflict(&TM_buff)) {stats_array[local_thread_id].self_conflicts++; }\
			else if(__TM_is_trans_conflict(&TM_buff)) stats_array[local_thread_id].trans_conflicts++; \
			else if(__TM_is_nontrans_conflict(&TM_buff)) stats_array[local_thread_id].nontrans_conflicts++; \
			htm_budget--; \
			printf("CONFLICT:%d\n", htm_budget); \
                        fflush(stdout); \
                } \
		else if (__TM_is_user_abort(&TM_buff)) { \
			stats_array[local_thread_id].user_aborts++; \
                        htm_budget--; \
			printf("USER ABORT:%d\n", htm_budget) ; \
                        fflush(stdout); \
                } \
		else if(__TM_is_footprint_exceeded(&TM_buff)){ \
			stats_array[local_thread_id].capacity_aborts++; \
                        htm_budget--; \
			printf("CAPACITY:%d\n", htm_budget); \
                        fflush(stdout); \
                } \
		else{ \
			stats_array[local_thread_id].other_aborts++; \
                        htm_budget--; \
			printf("OTHER:%d\n", htm_budget); \
                        fflush(stdout); \
                } \
            } \
            if (htm_budget <= 0) {   \
		mutex1->lock(); \
		mutex2->lock(); \
		mutex3->lock(); \
                break; \
            } \
        } \
};             

# define TM_END_ADAM(mutex1, mutex_2, mutex3){ \
    if (htm_budget > 0) { \
        __TM_end; \
        stats_array[local_thread_id].tle_commits++; \
    } else {    \
        mutex1->unlock(); \
        mutex2->unlock(); \
        mutex3->unlock(); \
        stats_array[local_thread_id].gl_commits++; \
    } \
};


#endif
