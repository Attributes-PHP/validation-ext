#include "av_between.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Between_ce;

static zend_always_inline double zval_to_double(zval *value)
{
    return Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
}

/*
 * #[Between(min, max)]: the value must be a number between min and max
 * (inclusive). Non-numeric values fail the constraint.
 */
bool av_validate_Between_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *min = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "min", sizeof("min") - 1, false, NULL);
    zval *max = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "max", sizeof("max") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE) {
        double number = zval_to_double(value);
        if (number >= zval_to_double(min) && number <= zval_to_double(max)) {
            return true;
        }
    }

    zend_string *min_str = av_value_to_string(min);
    zend_string *max_str = av_value_to_string(max);
    zend_string *expected = av_string_concat3(ZSTR_VAL(min_str), ZSTR_LEN(min_str), " and ", sizeof(" and ") - 1, ZSTR_VAL(max_str), ZSTR_LEN(max_str));
    av_string_release(min_str);
    av_string_release(max_str);

    av_add_field_error_with_expected(AV_ERROR_BETWEEN, field, property, errors, expected);
    av_string_release(expected);
    return false;
}

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
    AV_Fields_Constraints_Between_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Between_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Between_ce, "min", sizeof("min") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Constraints_Between_ce, "max", sizeof("max") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Between_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
