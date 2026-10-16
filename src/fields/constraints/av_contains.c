#include "av_contains.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"

zend_class_entry *AV_Fields_Constraints_Contains_ce;

/*
 * $values: string|int|float
 *
 * Enforced per element because PHP 8.2 arginfo cannot express a
 * union-typed variadic parameter.
 */
ZEND_METHOD(AV_Fields_Constraints_Contains, __construct)
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
                continue;
            default:
                zend_argument_type_error(i + 1, "must be of type string|int|float, %s given", zend_zval_type_name(&values[i]));
                RETURN_THROWS();
        }
    }

    zval member;
    array_init_size(&member, values_count);
    for (uint32_t i = 0; i < values_count; i++) {
        add_next_index_zval(&member, &values[i]);
        Z_TRY_ADDREF(values[i]);
    }

    zend_update_property(AV_Fields_Constraints_Contains_ce, Z_OBJ_P(getThis()), "values", sizeof("values") - 1, &member);
    zval_ptr_dtor(&member);
}

/*
 * #[Contains(...values)]: every declared value must be present. Array
 * values must contain each declared value (strict comparison); string
 * values must contain each declared value as a substring (numeric
 * values are matched by their string representation). Other value
 * types fail the constraint.
 */
bool av_validate_Contains_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *required = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "values", sizeof("values") - 1, false, NULL);

    zval *value = field->value;
    bool valid = false;

    if (Z_TYPE_P(value) == IS_ARRAY) {
        valid = true;
        zval *needle;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(required), needle)
        {
            bool found = false;
            zval *element;
            ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(value), element)
            {
                if (zend_is_identical(needle, element)) {
                    found = true;
                    break;
                }
            }
            ZEND_HASH_FOREACH_END();

            if (!found) {
                valid = false;
                break;
            }
        }
        ZEND_HASH_FOREACH_END();
    } else if (Z_TYPE_P(value) == IS_STRING) {
        valid = true;
        zval *needle;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(required), needle)
        {
            zend_string *needle_str = zval_get_string(needle);
            bool found = av_memnstr(Z_STRVAL_P(value), ZSTR_VAL(needle_str), ZSTR_LEN(needle_str), Z_STRVAL_P(value) + Z_STRLEN_P(value)) != NULL;
            zend_string_release(needle_str);

            if (!found) {
                valid = false;
                break;
            }
        }
        ZEND_HASH_FOREACH_END();
    }

    if (valid) {
        return true;
    }

    zend_string *expected = av_values_to_list_string(required);
    av_add_field_error_with_expected(AV_ERROR_CONTAINS, field, property, errors, expected);
    av_string_release(expected);
    return false;
}

void av_register_Contains_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Contains", class_AV_Fields_Constraints_Contains_methods);
    AV_Fields_Constraints_Contains_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Contains_ce != NULL);
    AV_Fields_Constraints_Contains_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Contains_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Contains_ce, "values", sizeof("values") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Contains_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
