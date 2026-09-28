#include "av_error_message.h"
#include "av_field.h"
#include <string.h>
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "../helpers/av_error_messages.h"

zend_class_entry *AV_Fields_ErrorMessage_ce;

ZEND_METHOD(AV_Fields_ErrorMessage, __construct)
{
    zend_string *required, *type;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_STR(required)
    Z_PARAM_STR(type)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(AV_Fields_ErrorMessage_ce, Z_OBJ_P(getThis()), "required", sizeof("required") - 1, ZSTR_VAL(required));
    zend_update_property_string(AV_Fields_ErrorMessage_ce, Z_OBJ_P(getThis()), "type", sizeof("type") - 1, ZSTR_VAL(type));
}

void av_register_ErrorMessage_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "ErrorMessage", class_AV_Fields_ErrorMessage_methods);
    AV_Fields_ErrorMessage_ce = zend_register_internal_class_ex(&ce, NULL);
    zend_class_implements(AV_Fields_ErrorMessage_ce, 1, AV_Fields_Field_ce);

    /* Register $required and $type properties */
    zend_declare_property_stringl(
        AV_Fields_ErrorMessage_ce, "required", sizeof("required") - 1, av_default_error_type_messages[AV_ERROR_REQUIRED], strlen(av_default_error_type_messages[AV_ERROR_REQUIRED]), ZEND_ACC_PUBLIC
    );
    zend_declare_property_stringl(AV_Fields_ErrorMessage_ce, "type", sizeof("type") - 1, av_default_error_type_messages[AV_ERROR_TYPE], strlen(av_default_error_type_messages[AV_ERROR_TYPE]), ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_ErrorMessage_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}