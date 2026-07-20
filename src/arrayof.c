#include "arrayof.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "zend_variables.h"
#include "Zend/zend_exceptions.h"

zend_class_entry *Attributes_Validation_ArrayOf_ce;

ZEND_METHOD(Attributes_Validation_ArrayOf, __construct)
{
    zval type_array;
    int num_args;
    zval *args;
    
    ZEND_PARSE_PARAMETERS_START(0, -1) // 0 required, -1 means unlimited optional args
        Z_PARAM_VARIADIC('*', args, num_args) // '*' captures all arguments into a C-array
    ZEND_PARSE_PARAMETERS_END();

    if (num_args <= 0) {
        zend_throw_exception_ex(zend_ce_error, 0, "No $types specified in Attributes\\Validation\\ArrayOf::__construct(...$types). If mixed remove #ArrayOf");
        RETURN_THROWS();
    }

    array_init(&type_array);

    for (int i = 0; i < num_args; i++) {
        zval copied_arg;
        ZVAL_COPY(&copied_arg, &args[i]);
        add_next_index_zval(&type_array, &copied_arg);
    }

    /* Store the array in the $type property */
    zend_update_property(Attributes_Validation_ArrayOf_ce, Z_OBJ_P(getThis()), "type", sizeof("type") - 1, &type_array);
    zval_ptr_dtor(&type_array);
}

void attributes_validation_register_ArrayOf_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Types", "ArrayOf", class_Attributes_Validation_ArrayOf_methods);
    Attributes_Validation_ArrayOf_ce = zend_register_internal_class_ex(&ce, NULL);
    
    /* Register $type property */
    zend_declare_property_null(Attributes_Validation_ArrayOf_ce, "type", sizeof("type") - 1, ZEND_ACC_PUBLIC);
    
    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(
        Attributes_Validation_ArrayOf_ce,
        ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER
    );

}
