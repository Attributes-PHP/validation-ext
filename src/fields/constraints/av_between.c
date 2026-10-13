#include "av_between.h"
#include "../av_field.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Between_ce;

ZEND_METHOD(AV_Fields_Constraints_Between, __construct)
{
    zval *min;
    zval *max;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_NUMBER(min)
    Z_PARAM_NUMBER(max)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(AV_Fields_Constraints_Between_ce, Z_OBJ_P(getThis()), "min", sizeof("min") - 1, min);
    zend_update_property(AV_Fields_Constraints_Between_ce, Z_OBJ_P(getThis()), "max", sizeof("max") - 1, max);
}

void av_register_Between_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Between", class_AV_Fields_Constraints_Between_methods);
    AV_Fields_Constraints_Between_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Between_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Between_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Between_ce, "min", sizeof("min") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Constraints_Between_ce, "max", sizeof("max") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Between_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
