#include "src/arrayof.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *validation_ext_ArrayOf_ce;

/* Method entries */
static zend_function_entry validation_ext_ArrayOf_methods[] = {
    ZEND_ME(ArrayOf, __construct, arginfo_ArrayOf___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_FE_END
};

/* Method implementations */
PHP_METHOD(ArrayOf, __construct)
{
    zval type_array;
    zval *args;
    
    /* Initialize type as an empty array */
    array_init(&type_array);
    
    /* For variadic ...$type, get all arguments as an array */
    if (ZEND_NUM_ARGS() > 0) {
        args = safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval), 0);
        if (_zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == SUCCESS) {
            uint32_t i;
            for (i = 0; i < ZEND_NUM_ARGS(); i++) {
                add_next_index_zval(&type_array, &args[i]);
            }
        }
        efree(args);
    }
    
    /* Store the array in the $type property */
    zend_update_property(validation_ext_ArrayOf_ce, Z_OBJ_P(getThis()), "type", sizeof("type") - 1, &type_array);
    zval_ptr_dtor(&type_array);
}

/* Registration function */
void register_ArrayOf_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\ArrayOf", validation_ext_ArrayOf_methods);
    
    validation_ext_ArrayOf_ce = zend_register_internal_class(&ce);
    
    /* Register $type property */
    zend_declare_property_null(validation_ext_ArrayOf_ce, "type", sizeof("type") - 1, ZEND_ACC_PUBLIC);
    
    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(
        validation_ext_ArrayOf_ce,
        ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER
    );
}
