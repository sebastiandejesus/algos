#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <stddef.h>

#define SUCCESS            0
#define ERROR              (-1)
#define NOTFOUND           1
#define ADDR               long long unsigned int
#define INIT_LL_SIZE_VAL   0
#define CONST_CAST(T, OBJ) (T)OBJ
#define FUNC               __func__

#ifdef SYNC
    #include <pthread.h>
    #define ll_LOCK(M)   do { pthread_mutex_lock((M)); } while (0)
    #define ll_UNLOCK(M) do { pthread_mutex_unlock((M)); } while (0)
#endif

#ifdef __GNUC__
/* lib attributes */
#define LIB_EXPORT    __attribute__((visibility("default")))
#define LIB_LOCAL     __attribute__((visibility("hidden")))
#define LIB_INTERNAL  __attribute__((visibility("internal")))

/* func attributes */
#define CONST   __attribute__((const))
#define NOTHROW __attribute__((nothrow))

/* struct attributes */
#define PACKED  __attribute__((packed))
#define ALIGNED __attribute__((aligned(16)))
#else
#define LIB_EXPORT   " "
#define LIB_LOCAL    " "
#define LIB_INTERNAL " "

/* func attributes */
#define CONST   " "
#define NOTHROW " "

/* struct attributes */
#define PACKED  " "
#define ALIGNED " "
#endif /* __GNUC__ */

#endif /* CONSTANTS_H */
