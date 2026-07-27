#include "alias.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *Attributes_Validation_Fields_Alias_ce;

ZEND_METHOD(Attributes_Validation_Fields_Alias, __construct)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(Attributes_Validation_Fields_Alias_ce, Z_OBJ_P(getThis()), "name", sizeof("name") - 1, ZSTR_VAL(name));
}

void attributes_validation_register_Alias_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Alias", class_Attributes_Validation_Fields_Alias_methods);
    Attributes_Validation_Fields_Alias_ce = zend_register_internal_class_ex(&ce, NULL);

    /* Register $name property */
    zend_declare_property_null(Attributes_Validation_Fields_Alias_ce, "name", sizeof("name") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(
        Attributes_Validation_Fields_Alias_ce,
        ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER
    );
}