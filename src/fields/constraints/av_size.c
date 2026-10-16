#include "av_size.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Constraints_Size_ce;

/*
 * #[Size(min: ?, max: ?)]: the size of the value must respect the
 * declared bounds (inclusive). Strings are measured in bytes, arrays
 * in element count. With only one bound set, only that bound applies;
 * with neither set the constraint always passes. Other value types
 * fail the constraint.
 */
ZEND_METHOD(AV_Fields_Constraints_Size, __construct)
{
    zend_long min = 0;
    zend_long max = 0;
    bool min_null = true;
    bool max_null = true;

    ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG_OR_NULL(min, min_null)
    Z_PARAM_LONG_OR_NULL(max, max_null)
    ZEND_PARSE_PARAMETERS_END();

    if (min_null) {
        zend_update_property_null(AV_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "min", sizeof("min") - 1);
    } else {
        zend_update_property_long(AV_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "min", sizeof("min") - 1, min);
    }

    if (max_null) {
        zend_update_property_null(AV_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "max", sizeof("max") - 1);
    } else {
        zend_update_property_long(AV_Fields_Constraints_Size_ce, Z_OBJ_P(getThis()), "max", sizeof("max") - 1, max);
    }
}

bool av_validate_Size_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *min = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "min", sizeof("min") - 1, false, NULL);
    zval *max = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "max", sizeof("max") - 1, false, NULL);

    bool has_min = Z_TYPE_P(min) != IS_NULL;
    bool has_max = Z_TYPE_P(max) != IS_NULL;
    if (!has_min && !has_max) {
        return true;
    }

    zval *value = field->value;
    zend_long size;
    if (Z_TYPE_P(value) == IS_STRING) {
        size = (zend_long)Z_STRLEN_P(value);
    } else if (Z_TYPE_P(value) == IS_ARRAY) {
        size = (zend_long)zend_hash_num_elements(Z_ARRVAL_P(value));
    } else {
        goto invalid;
    }

    if (has_min && size < Z_LVAL_P(min)) {
        goto invalid;
    }
    if (has_max && size > Z_LVAL_P(max)) {
        goto invalid;
    }

    return true;

invalid:
    {
        zend_string *expected;

        if (has_min && has_max) {
            zend_string *min_str = av_value_to_string(min);
            zend_string *max_str = av_value_to_string(max);
            zend_string *part = av_string_concat3(ZSTR_VAL(min_str), ZSTR_LEN(min_str), " and ", sizeof(" and ") - 1, ZSTR_VAL(max_str), ZSTR_LEN(max_str));
            expected = av_string_concat3("between ", sizeof("between ") - 1, ZSTR_VAL(part), ZSTR_LEN(part), "", 0);
            av_string_release(part);
            av_string_release(min_str);
            av_string_release(max_str);
        } else if (has_min) {
            zend_string *min_str = av_value_to_string(min);
            expected = av_string_concat3("of at least ", sizeof("of at least ") - 1, ZSTR_VAL(min_str), ZSTR_LEN(min_str), "", 0);
            av_string_release(min_str);
        } else {
            zend_string *max_str = av_value_to_string(max);
            expected = av_string_concat3("of at most ", sizeof("of at most ") - 1, ZSTR_VAL(max_str), ZSTR_LEN(max_str), "", 0);
            av_string_release(max_str);
        }

        av_add_field_error_with_expected(AV_ERROR_SIZE, field, property, errors, expected);
        av_string_release(expected);
        return false;
    }
}

void av_register_Size_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Size", class_AV_Fields_Constraints_Size_methods);
    AV_Fields_Constraints_Size_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Size_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Size_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Size_ce, "min", sizeof("min") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Constraints_Size_ce, "max", sizeof("max") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Size_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
