#ifndef AV_HELPERS_STRING_H
#define AV_HELPERS_STRING_H

#include <Zend/zend_types.h>
#include "testing.h"

typedef struct {
    char *value;
    size_t length;
} av_string;

/* Registration function */
zend_string* av_to_pascal_case(zend_string *str);
zend_string* av_to_camel_case(zend_string *str);
zend_string* av_to_snake_case(zend_string *str);
zend_string* av_to_kebab_case(zend_string *str);

STATIC_ZEND_INLINE bool is_uppercase(char c);
STATIC_ZEND_INLINE bool is_lowercase(char c);
STATIC_ZEND_INLINE bool is_alphanumeric(char c);
STATIC_INLINE void to_pascal_case(av_string *input, av_string *output);
STATIC_INLINE void to_camel_case(av_string *input, av_string *output);
STATIC_INLINE void to_snake_case(av_string *input, av_string *output);
STATIC_INLINE void to_kebab_case(av_string *input, av_string *output);

#endif /* AV_HELPERS_STRING_H */
