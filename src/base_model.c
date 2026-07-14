#include "base_model.h"
#include "Zend/zend_API.h"
#include "Zend/zend_interfaces.h"

zend_class_entry *Attributes_Validation_BaseModel_ce;

/* Method implementations */
ZEND_METHOD(Attributes_Validation_BaseModel, beforeValidation)
{
    RETURN_NULL();
}

ZEND_METHOD(Attributes_Validation_BaseModel, afterValidation)
{
    RETURN_NULL();
}

/* Registration function */
void attributes_validation_register_BaseModel_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation", "BaseModel", class_Attributes_Validation_BaseModel_methods);
    Attributes_Validation_BaseModel_ce = zend_register_internal_class_ex(&ce, NULL);
    Attributes_Validation_BaseModel_ce->ce_flags |= ZEND_ACC_ABSTRACT;
}

/**
 * Calls beforeValidation method from BaseModel e.g. $rawData = $model->beforeValidation($rawData, $configs)
 * This hook can modify the rawData before validation begins
 **/
void attributes_validation_call_before_validation_hook(zval *model, zval *raw_data, zval *configs)
{
    zval before_validation_result;
    zend_class_entry *model_ce = Z_OBJCE_P(model);

    ZVAL_UNDEF(&before_validation_result);
    zend_call_method_with_2_params(Z_OBJ_P(model), model_ce, NULL, "beforeValidation", &before_validation_result, raw_data, configs);
    if (EG(exception)) {
        zval_ptr_dtor(&before_validation_result);
        return;
    }

    if (Z_TYPE(before_validation_result) == IS_ARRAY) {
        zval old_data;
        ZVAL_COPY(&old_data, raw_data);
        zval_ptr_dtor(raw_data);
        ZVAL_COPY(raw_data, &before_validation_result);
        zval_ptr_dtor(&old_data);
    }

    zval_ptr_dtor(&before_validation_result);
}

/** Calls afterValidation method from BaseModel e.g. $model->afterValidation($rawData) **/
void attributes_validation_call_after_validation_hook(zval *model, zval *raw_data, zval *configs)
{
    zend_call_method_with_2_params(Z_OBJ_P(model), Z_OBJCE_P(model), NULL, "afterValidation", NULL, raw_data, configs);
}
