#ifndef ATTRIBUTES_VALIDATION_HELPERS_STRING_H
#define ATTRIBUTES_VALIDATION_HELPERS_STRING_H

#include "zend_types.h"

/* Registration function */
zend_string* attributes_validation_to_pascal_case(zend_string *str);
zend_string* attributes_validation_to_camel_case(zend_string *str);
zend_string* attributes_validation_to_snake_case(zend_string *str);
zend_string* attributes_validation_to_kebab_case(zend_string *str);

#endif /* ATTRIBUTES_VALIDATION_HELPERS_STRING_H */
