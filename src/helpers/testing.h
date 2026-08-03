#ifndef AV_HELPERS_TESTING_H
#define AV_HELPERS_TESTING_H

#ifdef TESTING

#define STATIC
#define STATIC_INLINE
#define STATIC_ZEND_INLINE
#define INLINE

#else

#define STATIC static
#define STATIC_INLINE static inline
#define STATIC_ZEND_INLINE static zend_always_inline
#define INLINE inline

#endif
#endif
