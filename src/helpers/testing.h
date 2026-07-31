#ifndef ATTRIBUTES_VALIDATION_HELPERS_TESTING_H
#define ATTRIBUTES_VALIDATION_HELPERS_TESTING_H

#ifdef TESTING

#define STATIC
#define STATIC_INLINE
#define STATIC_ZEND_INLINE

#else

#define STATIC static
#define STATIC_INLINE static inline
#define STATIC_ZEND_INLINE static zend_always_inline

#endif
#endif
