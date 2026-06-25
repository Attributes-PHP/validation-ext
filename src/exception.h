#ifndef VALIDATION_EXT_EXCEPTION_H
#define VALIDATION_EXT_EXCEPTION_H

#include "php.h"

/* Class entries */
extern zend_class_entry *validation_ext_BaseException_ce;
extern zend_class_entry *validation_ext_ValidationException_ce;

/* Method declaration */
PHP_METHOD(ValidationException, __construct);
PHP_METHOD(ValidationException, getErrors);

/* Argument info for ValidationException methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ValidationException___construct, 0, 0, 1)
    ZEND_ARG_INFO(0, allErrors)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ValidationException_getErrors, 0, 0, 0)
ZEND_END_ARG_INFO()

/* Registration functions */
void register_all_exception_classes();
void register_BaseException_class();
void register_ValidationException_class();

#endif /* VALIDATION_EXT_EXCEPTION_H */
