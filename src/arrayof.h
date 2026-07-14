#ifndef VALIDATION_EXT_ArrayOf_H
#define VALIDATION_EXT_ArrayOf_H

#include "php.h"

/* Class entry */
extern zend_class_entry *Attributes_Validation_ArrayOf_ce;

/* Method declaration */
ZEND_METHOD(Attributes_Validation_ArrayOf, __construct);

/* Argument info for ArrayOf constructor */
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Attributes_Validation_ArrayOf___construct, 0, 0, 0)
    ZEND_ARG_VARIADIC_TYPE_INFO(0, type, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Method entries */
static const zend_function_entry class_Attributes_Validation_ArrayOf_methods[] = {
    ZEND_ME(Attributes_Validation_ArrayOf, __construct, arginfo_class_Attributes_Validation_ArrayOf___construct, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

/* Registration function */
void attributes_validation_register_ArrayOf_class(void);

#endif /* VALIDATION_EXT_ArrayOf_H */
