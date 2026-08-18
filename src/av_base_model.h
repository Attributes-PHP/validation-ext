#ifndef AV_BASEMODEL_H
#define AV_BASEMODEL_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_BaseModel_ce;

/* Argument info for BaseModel methods */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_BaseModel_beforeValidation, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
    ZEND_ARG_OBJ_INFO(0, configs, Attributes\\Validation\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_BaseModel_afterValidation, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
    ZEND_ARG_OBJ_INFO(0, configs, Attributes\\Validation\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_BaseModel, beforeValidation);
ZEND_METHOD(AV_BaseModel, afterValidation);

static const zend_function_entry class_AV_BaseModel_methods[] = {
    ZEND_ME(AV_BaseModel, beforeValidation, arginfo_class_AV_BaseModel_beforeValidation, ZEND_ACC_PROTECTED)
    ZEND_ME(AV_BaseModel, afterValidation, arginfo_class_AV_BaseModel_afterValidation, ZEND_ACC_PROTECTED)
    ZEND_FE_END
};

/* Registration function */
void av_register_BaseModel_class(void);
void av_call_before_validation_hook(zval *model, zval *raw_data, zval *configs);
void av_call_after_validation_hook(zval *model, zval *raw_data, zval *configs);

#endif /* AV_BASEMODEL_H */
