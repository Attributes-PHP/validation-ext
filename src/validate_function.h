#ifndef VALIDATION_EXT_VALIDATE_FUNCTION_H
#define VALIDATION_EXT_VALIDATE_FUNCTION_H

#include "php.h"

/* PHP function declarations */
PHP_FUNCTION(validate);

/* Argument info for validate function */
ZEND_BEGIN_ARG_INFO_EX(arginfo_validate, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, rawData, 0)
    ZEND_ARG_OBJ_INFO(0, model, Attributes\\Validation\\BaseModel, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopAtFirstError, _IS_BOOL, 1, "null")
ZEND_END_ARG_INFO()

#endif /* VALIDATION_EXT_VALIDATE_FUNCTION_H */
