#include "av_regex.h"
#include "../av_field.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Regex_ce;

ZEND_METHOD(AV_Fields_Constraints_Regex, __construct)
{
    zend_string *regex;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(regex)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(AV_Fields_Constraints_Regex_ce, Z_OBJ_P(getThis()), "regex", sizeof("regex") - 1, ZSTR_VAL(regex));
}

void av_register_Regex_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Regex", class_AV_Fields_Constraints_Regex_methods);
    AV_Fields_Constraints_Regex_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Regex_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Regex_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Regex_ce, "regex", sizeof("regex") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Regex_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
