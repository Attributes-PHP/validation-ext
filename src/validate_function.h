#ifndef VALIDATION_EXT_VALIDATE_FUNCTION_H
#define VALIDATION_EXT_VALIDATE_FUNCTION_H

#include "php.h"

/* PHP function declarations */
PHP_FUNCTION(validate);

/* Argument info for validate function */
ZEND_BEGIN_ARG_INFO_EX(arginfo_validate, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, rawData, 0)
    ZEND_ARG_INFO(0, model)
    ZEND_ARG_INFO(0, strict)
    ZEND_ARG_INFO(0, stopAtFirstError)
ZEND_END_ARG_INFO()

static void create_options(zval *options, zend_bool strict, zend_bool stop_at_first_error);
static void call_before_validation_hook(zval **raw_data, zval *model, zval *options);
static void call_after_validation_hook(zval **raw_data, zval *model, zval *options);

#endif /* VALIDATION_EXT_VALIDATE_FUNCTION_H */
