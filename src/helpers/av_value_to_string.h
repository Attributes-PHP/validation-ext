#ifndef AV_HELPERS_VALUE_TO_STRING_H
#define AV_HELPERS_VALUE_TO_STRING_H

#include <Zend/zend_types.h>

/*
 * Converts any PHP zval into a zend_string for use in error message
 * templates (the {value} placeholder). See av_value_to_string.c for the
 * per-type conversion rules.
 */
zend_string* av_value_to_string(zval *value);

#endif /* AV_HELPERS_VALUE_TO_STRING_H */
