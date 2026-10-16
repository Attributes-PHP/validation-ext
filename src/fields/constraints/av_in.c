#include "av_in.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"

zend_class_entry *AV_Fields_Constraints_In_ce;

/*
 * $values: string|int|float|bool
 *
 * Enforced per element because PHP 8.2 arginfo cannot express a
 * union-typed variadic parameter.
 */
ZEND_METHOD(AV_Fields_Constraints_In, __construct)
{
    zval *values;
    uint32_t values_count;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('z', values, values_count)
    ZEND_PARSE_PARAMETERS_END();

    for (uint32_t i = 0; i < values_count; i++) {
        switch (Z_TYPE(values[i])) {
            case IS_STRING:
            case IS_LONG:
            case IS_DOUBLE:
            case IS_TRUE:
            case IS_FALSE:
                continue;
            default:
                zend_argument_type_error(i + 1, "must be of type string|int|float|bool, %s given", zend_zval_type_name(&values[i]));
                RETURN_THROWS();
        }
    }

    zval member;
    array_init_size(&member, values_count);
    for (uint32_t i = 0; i < values_count; i++) {
        add_next_index_zval(&member, &values[i]);
        Z_TRY_ADDREF(values[i]);
    }

    zend_update_property(AV_Fields_Constraints_In_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

/*
 * #[In(...values)]: the value must be strictly identical to one of the
 * declared values.
 */
bool av_validate_In_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *allowed = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    zval *value = field->value;
    zval *entry;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(allowed), entry)
    {
        if (zend_is_identical(entry, value)) {
            return true;
        }
    }
    ZEND_HASH_FOREACH_END();

    zend_string *expected = av_values_to_list_string(allowed);
    av_add_field_error_with_expected(AV_ERROR_IN, field, property, errors, expected);
    av_string_release(expected);
    return false;
}

void av_register_In_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "In", class_AV_Fields_Constraints_In_methods);
    AV_Fields_Constraints_In_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_In_ce != NULL);
    AV_Fields_Constraints_In_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_In_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_In_ce, "values", sizeof("values") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_In_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
