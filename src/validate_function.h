#ifndef VALIDATION_EXT_VALIDATE_FUNCTION_H
#define VALIDATION_EXT_VALIDATE_FUNCTION_H

#include "php.h"

PHP_FUNCTION(validate);

/* Argument info for validate function */
ZEND_BEGIN_ARG_INFO_EX(arginfo_validate, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, rawData, 0)
    ZEND_ARG_INFO(0, model)
    ZEND_ARG_INFO(0, strict)
    ZEND_ARG_INFO(0, stopAtFirstError)
ZEND_END_ARG_INFO()

#endif /* VALIDATION_EXT_VALIDATE_FUNCTION_H */
