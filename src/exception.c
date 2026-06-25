#include "src/exception.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"

zend_class_entry *validation_ext_BaseException_ce;
zend_class_entry *validation_ext_ValidationException_ce;

/* Method declarations for ValidationException */
static zend_function_entry validation_ext_ValidationException_methods[] = {
    ZEND_ME(ValidationException, __construct, arginfo_ValidationException___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(ValidationException, getErrors, arginfo_ValidationException_getErrors, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

void register_all_exception_classes()
{
    register_BaseException_class();
    register_ValidationException_class();
}

void register_BaseException_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\BaseException", NULL);
    
    validation_ext_BaseException_ce = zend_register_internal_class_ex(&ce, zend_ce_exception);
}

PHP_METHOD(ValidationException, __construct)
{
    zval *allErrors;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(allErrors)
    ZEND_PARSE_PARAMETERS_END();

    zval message;
    ZVAL_STRING(&message, "Invalid data");
    zend_call_method_with_1_params(Z_OBJ_P(getThis()), validation_ext_BaseException_ce, NULL, "__construct", NULL, &message);
    zval_dtor(&message);

    zend_update_property(validation_ext_ValidationException_ce, Z_OBJ_P(getThis()), "allErrors", sizeof("allErrors") - 1, allErrors);
}

PHP_METHOD(ValidationException, getErrors)
{
    zval *allErrors;
    
    allErrors = zend_read_property(validation_ext_ValidationException_ce, Z_OBJ_P(getThis()), "allErrors", sizeof("allErrors") - 1, 0, NULL);
    
    if (Z_TYPE_P(allErrors) == IS_NULL) {
        array_init(return_value);
        return;
    }
    
    RETURN_ZVAL(allErrors, 1, 0);
}

void register_ValidationException_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\ValidationException", validation_ext_ValidationException_methods);
    
    validation_ext_ValidationException_ce = zend_register_internal_class_ex(&ce, validation_ext_BaseException_ce);
    
    /* Declare the private $allErrors property */
    zend_declare_property_null(validation_ext_ValidationException_ce, "allErrors", sizeof("allErrors") - 1, ZEND_ACC_PRIVATE);
}
