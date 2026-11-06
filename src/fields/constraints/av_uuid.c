#include "av_uuid.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Uuid_ce;

static zend_always_inline bool is_hex_digit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/*
 * #[Uuid]: the value must be a canonical UUID string: 36 characters
 * laid out as 8-4-4-4-12 hexadecimal groups separated by dashes, both
 * letter cases accepted (for example
 * 123e4567-e89b-12d3-a456-426614174000). Non-string values fail the
 * constraint.
 */
bool av_validate_Uuid_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        av_add_field_error_with_prefix(AV_ERROR_UUID, field, property, errors);
        return false;
    }

    if (Z_STRLEN_P(value) != 36) {
        av_add_field_error_with_prefix(AV_ERROR_UUID, field, property, errors);
        return false;
    }

    const char *str = Z_STRVAL_P(value);
    for (size_t i = 0; i < 36; i++) {
        const char c = str[i];

        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') {
                av_add_field_error_with_prefix(AV_ERROR_UUID, field, property, errors);
                return false;
            }
            continue;
        }

        if (!is_hex_digit(c)) {
            av_add_field_error_with_prefix(AV_ERROR_UUID, field, property, errors);
            return false;
        }
    }

    return true;
}

void av_register_Uuid_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Uuid", class_AV_Fields_Constraints_Uuid_methods);
    AV_Fields_Constraints_Uuid_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Uuid_ce != NULL);
    AV_Fields_Constraints_Uuid_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Uuid_ce, 1, AV_Fields_Field_ce);

    zend_internal_attribute_register(AV_Fields_Constraints_Uuid_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
