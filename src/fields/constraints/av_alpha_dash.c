#include "av_alpha_dash.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_AlphaDash_ce;

/*
 * #[AlphaDash]: the value must only contain letters, numbers, dashes
 * and underscores. Non-string values fail the constraint.
 */
bool av_validate_AlphaDash_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    (void)constraint;

    zval *value = field->value;
    if (Z_TYPE_P(value) == IS_STRING) {
        const char *str = Z_STRVAL_P(value);
        size_t len = Z_STRLEN_P(value);
        bool valid = true;

        for (size_t i = 0; i < len; i++) {
            const unsigned char c = (unsigned char)str[i];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
                continue;
            }
            valid = false;
            break;
        }

        if (valid) {
            return true;
        }
    }

    av_add_field_error_with_prefix(AV_ERROR_ALPHA_DASH, field, property, errors);
    return false;
}

void av_register_AlphaDash_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "AlphaDash", class_AV_Fields_Constraints_AlphaDash_methods);
    AV_Fields_Constraints_AlphaDash_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_AlphaDash_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_AlphaDash_ce, 1, AV_Fields_Field_ce);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_AlphaDash_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
