#ifndef TINY_H
#define TINY_H

#define RO                              1
#define RW                              0

#if defined(TM_GCC) 
# include "../../abi/gcc/tm_macros.h"
#elif defined(TM_DTMC) 
# include "../../abi/dtmc/tm_macros.h"
#elif defined(TM_INTEL)
# include "../../abi/intel/tm_macros.h"
#elif defined(TM_ABI)
# include "../../abi/tm_macros.h"
#endif /* defined(TM_ABI) */

#if defined(TM_GCC) || defined(TM_DTMC) || defined(TM_INTEL) || defined(TM_ABI)
# define TM_COMPILER
/* Add some attributes to library function */
TM_PURE 
void exit(int status);
TM_PURE 
void perror(const char *s);
#else /* Compile with explicit calls to tinySTM */

#include "stm.h"
#include "mod_ab.h"

/*
 * Useful macros to work with transactions. Note that, to use nested
 * transactions, one should check the environment returned by
 * stm_get_env() and only call sigsetjmp() if it is not null.
 */

#define TM_START(tid, ro)               { stm_tx_attr_t _a = {{.id = tid, .read_only = ro}}; sigjmp_buf *_e = stm_start(_a); if (_e != NULL) sigsetjmp(*_e, 0)

//#define TM_START(id, ro)                   { sigjmp_buf *_e = stm_get_env(); stm_tx_attr_t _a = {id, ro}; sigsetjmp(*_e, 0); stm_start(_e, &_a)
#define TM_LOAD(addr)                   stm_load((stm_word_t *)addr)
#define TM_STORE(addr, value)           stm_store((stm_word_t *)addr, (stm_word_t)value)
#define TM_COMMIT                       stm_commit(); }

#define TM_INIT                         stm_init(); mod_ab_init(0, NULL)
#define TM_EXIT                         stm_exit()
#define TM_INIT_THREAD                  stm_init_thread()
#define TM_EXIT_THREAD                  stm_exit_thread()

#endif /* Compile with explicit calls to tinySTM */

#define DEFAULT_DURATION                10000
#define DEFAULT_NB_ACCOUNTS             1024
#define DEFAULT_NB_THREADS              1
#define DEFAULT_READ_ALL                20
#define DEFAULT_SEED                    0
#define DEFAULT_WRITE_ALL               0
#define DEFAULT_READ_THREADS            0
#define DEFAULT_WRITE_THREADS           0
#define DEFAULT_DISJOINT                0

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

#endif
