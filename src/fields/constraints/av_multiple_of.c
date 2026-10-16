#include "av_multiple_of.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include <math.h>

/* Tolerance for float multiple checks: fmod(0.3, 0.1) keeps enough
 * representation noise that a zero threshold would reject it. */
#define AV_MULTIPLE_OF_EPSILON 1e-9

zend_class_entry *AV_Fields_Constraints_MultipleOf_ce;

/*
 * #[MultipleOf(value)]: the value must be a multiple of value. Integer
 * pairs use integer modulo; mixed or float pairs use fmod with a small
 * epsilon so floating point representation noise stays within bounds.
 * Non-numeric values fail the constraint.
 */
bool av_validate_MultipleOf_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *bound = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "value", sizeof("value") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) == IS_LONG && Z_TYPE_P(bound) == IS_LONG) {
        if (Z_LVAL_P(bound) == 0 ? Z_LVAL_P(value) == 0 : Z_LVAL_P(value) % Z_LVAL_P(bound) == 0) {
            return true;
        }
    } else if (Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE) {
        double number = Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
        double divisor = Z_TYPE_P(bound) == IS_LONG ? (double)Z_LVAL_P(bound) : Z_DVAL_P(bound);

        bool multiple;
        if (divisor == 0) {
            multiple = number == 0;
        } else {
            double remainder = fmod(number, divisor);
            double tolerance = fabs(divisor) * AV_MULTIPLE_OF_EPSILON;
            multiple = fabs(remainder) <= tolerance || fabs(fabs(remainder) - fabs(divisor)) <= tolerance;
        }

        if (multiple) {
            return true;
        }
    }

    zend_string *expected = av_value_to_string(bound);
    av_add_field_error_with_expected(AV_ERROR_MULTIPLE_OF, field, property, errors, expected);
    av_string_release(expected);
    return false;
}

ZEND_METHOD(AV_Fields_Constraints_MultipleOf, __construct)
{
    zval *value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_NUMBER(value)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(AV_Fields_Constraints_MultipleOf_ce, Z_OBJ_P(getThis()), "value", sizeof("value") - 1, value);
}

void av_register_MultipleOf_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "MultipleOf", class_AV_Fields_Constraints_MultipleOf_methods);
    AV_Fields_Constraints_MultipleOf_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_MultipleOf_ce != NULL);
    AV_Fields_Constraints_MultipleOf_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_MultipleOf_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_MultipleOf_ce, "value", sizeof("value") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_MultipleOf_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
