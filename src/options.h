#ifndef VALIDATION_EXT_OPTIONS_H
#define VALIDATION_EXT_OPTIONS_H

#include "php.h"

/* Class entry */
extern zend_class_entry *validation_ext_Options_ce;

/* Method declarations */
PHP_METHOD(Options, __construct);
PHP_METHOD(Options, isStrict);
PHP_METHOD(Options, isToStopAtFirstError);

/* Argument info for Options methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_Options___construct, 0, 0, 2)
    ZEND_ARG_INFO(0, isStrict)
    ZEND_ARG_INFO(0, isToStopAtFirstError)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Options_isStrict, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Options_isToStopAtFirstError, 0, 0, 0)
ZEND_END_ARG_INFO()

/* Registration function */
void register_Options_class();

#endif /* VALIDATION_EXT_BASEMODEL_H */
