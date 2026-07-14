#ifndef ATTRIBUTES_VALIDATION_VALIDATE_FUNCTION_H
#define ATTRIBUTES_VALIDATION_VALIDATE_FUNCTION_H

#include "php.h"

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Attributes_Validation_validate, 0, 2, Attributes\\Validation\\BaseModel, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
    ZEND_ARG_OBJ_INFO(0, model, Attributes\\Validation\\BaseModel, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(validate);

#endif /* ATTRIBUTES_VALIDATION_VALIDATE_FUNCTION_H */
