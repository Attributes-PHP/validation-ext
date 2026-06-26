#include "src/base_model.h"
#include "Zend/zend_API.h"
#include "src/model_configs.h"
#include "Zend/zend_interfaces.h"

zend_class_entry *validation_ext_BaseModel_ce;

/* Method entries */
static zend_function_entry validation_ext_BaseModel_methods[] = {
    ZEND_ME(BaseModel, beforeValidation, arginfo_BaseModel_beforeValidation, ZEND_ACC_PUBLIC)
    ZEND_ME(BaseModel, afterValidation, arginfo_BaseModel_afterValidation, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

/* Method implementations */
PHP_METHOD(BaseModel, beforeValidation)
{
    // Silence is golden
}

PHP_METHOD(BaseModel, afterValidation)
{
    // Silence is golden
}

/* Registration function */
void register_BaseModel_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\BaseModel", validation_ext_BaseModel_methods);
    
    validation_ext_BaseModel_ce = zend_register_internal_class(&ce);
    
    /* Mark BaseModel as abstract */
    validation_ext_BaseModel_ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
}

/**
 * Calls beforeValidation method from BaseModel e.g. $rawData = $model->beforeValidation($rawData, $configs)
 * This hook can modify the rawData before validation begins
 **/
void call_before_validation_hook(zval *model, zval *raw_data, zval *configs)
{
    zval *before_validation_result = emalloc(sizeof(zval));
    ZVAL_UNDEF(before_validation_result);
    zend_class_entry *model_ce = Z_OBJCE_P(model);

    zend_call_method_with_2_params(Z_OBJ_P(model), model_ce, NULL, "beforeValidation", before_validation_result, raw_data, configs);
    if (Z_TYPE_P(before_validation_result) != IS_ARRAY) {
        zval_ptr_dtor(before_validation_result);
        return;
    }

    zval_ptr_dtor(raw_data);
    raw_data = before_validation_result;
}

/** Calls afterValidation method from BaseModel e.g. $model->afterValidation($rawData, $configs) **/
void call_after_validation_hook(zval *model, zval *raw_data, zval *configs)
{
    zend_call_method_with_2_params(Z_OBJ_P(model), Z_OBJCE_P(model), NULL, "afterValidation", NULL, raw_data, configs);
}
