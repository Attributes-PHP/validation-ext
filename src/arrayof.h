#ifndef VALIDATION_EXT_ArrayOf_H
#define VALIDATION_EXT_ArrayOf_H

#include "php.h"

/* Class entry */
extern zend_class_entry *validation_ext_ArrayOf_ce;

/* Method declaration */
PHP_METHOD(ArrayOf, __construct);

/* Argument info for ArrayOf constructor */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ArrayOf___construct, 0, 0, 0)
    ZEND_ARG_VARIADIC_INFO(0, type)
ZEND_END_ARG_INFO()

/* Registration function */
void register_ArrayOf_class();

#endif /* VALIDATION_EXT_ArrayOf_H */
