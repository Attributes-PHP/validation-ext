#include "av_min.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Min_ce;

static zend_always_inline double zval_to_double(zval *value)
{
    return Z_TYPE_P(value) == IS_LONG ? (double)Z_LVAL_P(value) : Z_DVAL_P(value);
}

/*
 * #[Min(value)]: the value must be a number at least value. Non-numeric
 * values fail the constraint.
 */
bool av_validate_Min_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *bound = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "value", sizeof("value") - 1, false, NULL);

    zval *value = field->value;
    if ((Z_TYPE_P(value) == IS_LONG || Z_TYPE_P(value) == IS_DOUBLE) && zval_to_double(value) >= zval_to_double(bound)) {
        return true;
    }

    zend_string *expected = av_value_to_string(bound);
    av_add_field_error_with_expected(AV_ERROR_MIN, field, property, errors, expected);
    av_string_release(expected);
    return false;
}

ZEND_METHOD(AV_Fields_Constraints_Min, __construct)
{
    zval *value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_NUMBER(value)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(AV_Fields_Constraints_Min_ce, Z_OBJ_P(getThis()), "value", sizeof("value") - 1, value);
}

void av_register_Min_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Min", class_AV_Fields_Constraints_Min_methods);
    AV_Fields_Constraints_Min_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Min_ce != NULL);
    AV_Fields_Constraints_Min_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Min_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Min_ce, "value", sizeof("value") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Min_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
