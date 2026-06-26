#include "src/base_model.h"
#include "Zend/zend_API.h"
#include "src/options.h"

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
    zval *rawData;
    zval *options;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(rawData)
        Z_PARAM_OBJECT_OF_CLASS(options, validation_ext_Options_ce)
    ZEND_PARSE_PARAMETERS_END();

    // Return rawData by default
    RETURN_ZVAL(rawData, 1, 0);
}

PHP_METHOD(BaseModel, afterValidation)
{
    zval *rawData;
    zval *options;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(rawData)
        Z_PARAM_OBJECT_OF_CLASS(options, validation_ext_Options_ce)
    ZEND_PARSE_PARAMETERS_END();

    // Void method - does not return anything
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
