#ifndef VALIDATION_EXT_BASEMODEL_H
#define VALIDATION_EXT_BASEMODEL_H

#include "php.h"

/* Class entry */
extern zend_class_entry *Attributes_Validation_BaseModel_ce;

/* Argument info for BaseModel methods */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_BaseModel_beforeValidation, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_BaseModel_afterValidation, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Attributes_Validation_BaseModel_getModelConfigs, 0, 0, Attributes\\Validation\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Attributes_Validation_BaseModel, beforeValidation);
ZEND_METHOD(Attributes_Validation_BaseModel, afterValidation);
ZEND_METHOD(Attributes_Validation_BaseModel, getModelConfigs);

static const zend_function_entry class_Attributes_Validation_BaseModel_methods[] = {
    ZEND_ME(Attributes_Validation_BaseModel, beforeValidation, arginfo_class_Attributes_Validation_BaseModel_beforeValidation, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_BaseModel, afterValidation, arginfo_class_Attributes_Validation_BaseModel_afterValidation, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_BaseModel, getModelConfigs, arginfo_class_Attributes_Validation_BaseModel_getModelConfigs, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

/* Registration function */
void register_BaseModel_class(void);
void call_before_validation_hook(zval *model, zval *raw_data);
void call_after_validation_hook(zval *model, zval *raw_data);

#endif /* VALIDATION_EXT_BASEMODEL_H */
