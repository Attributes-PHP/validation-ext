#ifndef VALIDATION_EXT_CALL_FUNCTION_H
#define VALIDATION_EXT_CALL_FUNCTION_H

#include "php.h"

PHP_FUNCTION(call);

/* Argument info for call function */
ZEND_BEGIN_ARG_INFO_EX(arginfo_call, 0, 0, 2)
    ZEND_ARG_INFO(0, function)
    ZEND_ARG_ARRAY_INFO(0, params, 0)
    ZEND_ARG_ARRAY_INFO(0, dependencies, 1)
    ZEND_ARG_INFO(0, strict)
ZEND_END_ARG_INFO()

#endif /* VALIDATION_EXT_CALL_FUNCTION_H */
