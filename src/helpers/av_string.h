#ifndef ATTRIBUTES_VALIDATION_HELPERS_STRING_H
#define ATTRIBUTES_VALIDATION_HELPERS_STRING_H

#include <Zend/zend_types.h>
#include "testing.h"

typedef struct {
    char *value;
    size_t length;
} attributes_validation_string;

/* Registration function */
zend_string* attributes_validation_to_pascal_case(zend_string *str);
zend_string* attributes_validation_to_camel_case(zend_string *str);
zend_string* attributes_validation_to_snake_case(zend_string *str);
zend_string* attributes_validation_to_kebab_case(zend_string *str);

STATIC_ZEND_INLINE bool is_uppercase(char c);
STATIC_ZEND_INLINE bool is_lowercase(char c);
STATIC_ZEND_INLINE bool is_alphanumeric(char c);
STATIC_INLINE void to_pascal_case(attributes_validation_string *input, attributes_validation_string *output);
STATIC_INLINE void to_camel_case(attributes_validation_string *input, attributes_validation_string *output);
STATIC_INLINE void to_snake_case(attributes_validation_string *input, attributes_validation_string *output);
STATIC_INLINE void to_kebab_case(attributes_validation_string *input, attributes_validation_string *output);

#endif /* ATTRIBUTES_VALIDATION_HELPERS_STRING_H */
