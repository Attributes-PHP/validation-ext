#include "src/base_model.h"
#include "Zend/zend_API.h"

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
    // Empty implementation for now
}

PHP_METHOD(BaseModel, afterValidation)
{
    // Empty implementation for now
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
