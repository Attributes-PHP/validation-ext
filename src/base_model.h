#ifndef VALIDATION_EXT_BASEMODEL_H
#define VALIDATION_EXT_BASEMODEL_H

#include "php.h"

/* Class entry */
extern zend_class_entry *validation_ext_BaseModel_ce;

/* Method declarations */
PHP_METHOD(BaseModel, beforeValidation);
PHP_METHOD(BaseModel, afterValidation);

/* Argument info for BaseModel methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_BaseModel_beforeValidation, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, rawData, 0)
    ZEND_ARG_OBJ_INFO(0, options, Attributes\\Validation\\Options, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_BaseModel_afterValidation, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, rawData, 0)
    ZEND_ARG_OBJ_INFO(0, options, Attributes\\Validation\\Options, 0)
ZEND_END_ARG_INFO()

/* Registration function */
void register_BaseModel_class();

#endif /* VALIDATION_EXT_BASEMODEL_H */
