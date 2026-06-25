#include "src/options.h"
#include "Zend/zend_API.h"

zend_class_entry *validation_ext_Options_ce;

/* Method entries */
static zend_function_entry validation_ext_Options_methods[] = {
    ZEND_ME(Options, __construct, arginfo_Options___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(Options, isStrict, arginfo_Options_isStrict, ZEND_ACC_PUBLIC)
    ZEND_ME(Options, isToStopAtFirstError, arginfo_Options_isToStopAtFirstError, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

/* Method implementations */
PHP_METHOD(Options, __construct)
{
    zend_bool is_strict = 0;
    zend_bool is_stop_first_error = 0;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_BOOL(is_strict)
        Z_PARAM_BOOL(is_stop_first_error)
    ZEND_PARSE_PARAMETERS_END();
    
    zend_update_property_bool(validation_ext_Options_ce, Z_OBJ_P(getThis()), "isStrict", sizeof("isStrict") - 1, is_strict);
    zend_update_property_bool(validation_ext_Options_ce, Z_OBJ_P(getThis()), "isToStopAtFirstError", sizeof("isToStopAtFirstError") - 1, is_stop_first_error);
}

/* Method implementations */
PHP_METHOD(Options, isStrict)
{
    zval rv;
    zval *property = zend_read_property(validation_ext_Options_ce, Z_OBJ_P(getThis()), "isStrict", sizeof("isStrict") - 1, 0, &rv);
    RETURN_ZVAL(property, 1, 0);
}

PHP_METHOD(Options, isToStopAtFirstError)
{
    zval rv;
    zval *property = zend_read_property(validation_ext_Options_ce, Z_OBJ_P(getThis()), "isToStopAtFirstError", sizeof("isToStopAtFirstError") - 1, 0, &rv);
    RETURN_ZVAL(property, 1, 0);
}

/* Registration function */
void register_Options_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\Options", validation_ext_Options_methods);
    
    validation_ext_Options_ce = zend_register_internal_class(&ce);
    
    /* Mark BaseModel as final */
    validation_ext_Options_ce->ce_flags |= ZEND_ACC_FINAL;
    
    /* Declare properties - must be done AFTER registration */
    zend_declare_property_bool(validation_ext_Options_ce, "isStrict", sizeof("isStrict") - 1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(validation_ext_Options_ce, "isToStopAtFirstError", sizeof("isToStopAtFirstError") - 1, 0, ZEND_ACC_PUBLIC);
}
