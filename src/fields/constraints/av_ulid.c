#include "av_ulid.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Ulid_ce;

static zend_always_inline bool is_crockford_base32(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/*
 * #[Ulid]: the value must be a valid ULID string: exactly 26 characters
 * from the Crockford base32 alphabet (0-9 A-Z minus I, L, O, U; both
 * letter cases accepted). Non-string values fail the constraint.
 */
bool av_validate_Ulid_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    (void)constraint;

    zval *value = field->value;
    if (Z_TYPE_P(value) == IS_STRING && Z_STRLEN_P(value) == 26) {
        const char *str = Z_STRVAL_P(value);
        bool valid = true;

        for (size_t i = 0; i < 26; i++) {
            const char c = str[i];
            if (c == 'i' || c == 'l' || c == 'o' || c == 'u' || c == 'I' || c == 'L' || c == 'O' || c == 'U') {
                valid = false;
                break;
            }
            if (!is_crockford_base32(c)) {
                valid = false;
                break;
            }
        }

        if (valid) {
            return true;
        }
    }

    av_add_field_error_with_prefix(AV_ERROR_ULID, field, property, errors);
    return false;
}

void av_register_Ulid_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Ulid", class_AV_Fields_Constraints_Ulid_methods);
    AV_Fields_Constraints_Ulid_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Ulid_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Ulid_ce, 1, AV_Fields_Field_ce);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Ulid_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
