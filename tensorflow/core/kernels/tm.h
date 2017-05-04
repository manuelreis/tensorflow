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
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t stats_array[];


extern __thread int local_thread_id;

#  define TM_STARTUP(numThread, bId)
#  define TM_SHUTDOWN() { \
        unsigned long commits = 0; \
        unsigned long aborts = 0; \
        unsigned long commits_with_lock = 0; \
        unsigned long explicit_aborts = 0; \
        unsigned long conflict_aborts = 0; \
        unsigned long capacity_aborts = 0; \
        unsigned long other_aborts = 0; \
        int i = 0; \
        for (; i < 80; i++) { \
            if (stats_array[i].commits + stats_array[i].commits_with_lock == 0) { break; } \
                commits += stats_array[i].commits; \
                aborts += stats_array[i].aborts; \
                commits_with_lock += stats_array[i].commits_with_lock; \
                explicit_aborts += stats_array[i].explicit_aborts; \
                conflict_aborts += stats_array[i].conflict_aborts; \
                capacity_aborts += stats_array[i].capacity_aborts; \
                other_aborts += stats_array[i].other_aborts; \
            } \
        printf("TM commits: %lu\nTotal aborts: %lu\nTotal Lock Commits: %lu\nExplicit Aborts: %lu\nConflict Aborts: %lu\nCapacity Aborts: %lu\n Other Aborts: %lu\n", commits, aborts, commits_with_lock, explicit_aborts, conflict_aborts, capacity_aborts, other_aborts); \
}

# define TM_THREAD_ENTER()
# define TM_THREAD_EXIT()

# define IS_LOCKED(mutex)        mutex->is_locked()

# define RETRY_POLICY 5
# define HTM_RETRIES 5
# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define TM_BEGIN(mutex) { \
	printf("enter tm_begin\n"); \
        while (1) { \
            while (IS_LOCKED(mutex)) { \
                __asm__ ( "pause;"); \
		printf("a\n"); \
            } \
            unsigned int status = _xbegin(); \
            if (status == _XBEGIN_STARTED) { \
                if (IS_LOCKED(mutex)) { \
                    _xabort(30); \
		    printf("b\n"); \
                } \
                break; \
            } \
            else if (status == _XABORT_CAPACITY) { \
                stats_array[local_thread_id].capacity_aborts++;\
                SPEND_BUDGET(&htm_budget); \
                printf("c\n"); \
            } else \
            { \
                if (status & _XABORT_CONFLICT) {\
                        stats_array[local_thread_id].conflict_aborts++;\
                         printf("d\n"); \
                }\
                else if (status & _XABORT_EXPLICIT) {\
                        stats_array[local_thread_id].explicit_aborts++;\
                        printf("e\n"); \
                }\
                else if (status & _XABORT_NESTED) {\
                        printf("nested abort\n"); \
                        stats_array[local_thread_id].other_aborts++;\
                }\
                else if (status & _XABORT_DEBUG) {\
                        printf("debug abort\n"); \
                        stats_array[local_thread_id].other_aborts++;\
                }\
                else if (status & _XABORT_RETRY) {\
                        printf("retry abort\n"); \
                        stats_array[local_thread_id].other_aborts++;\
                }\
                else {\
                        stats_array[local_thread_id].other_aborts++;\
                        printf("other abort code..\n"); \
                }\
                htm_budget--; \
            } \
            stats_array[local_thread_id].aborts++; \
            if (htm_budget <= 0) {   \
                printf("g\n"); \
		while(!mutex->try_lock()){ \
                     printf("Pause try lock\n"); \
                     __asm__ ("pause;"); \
		} \
            } \
        } \
	printf("exit tm_begin\n"); \
};


# define TM_END(mutex){ \
    if (htm_budget > 0) { \
        _xend(); \
        stats_array[local_thread_id].commits++; \
    } else {    \
        mutex->unlock(); \
        stats_array[local_thread_id].commits_with_lock++; \
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
