#include "av_datetime.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_operators.h"
#include <ext/date/php_date.h>

zend_class_entry *AV_Fields_Constraints_DateTime_ce;

/*
 * $timezone: DateTimeZone|string|null
 *
 * Checked eagerly so a malformed timezone fails at construction instead of
 * at validation time.
 */
static bool is_valid_timezone(zval *value)
{
    switch (Z_TYPE_P(value)) {
        case IS_NULL:
        case IS_STRING:
            return true;

        case IS_OBJECT:
            return instanceof_function(Z_OBJCE_P(value), php_date_get_timezone_ce());

        default:
            return false;
    }
}

ZEND_METHOD(AV_Fields_Constraints_DateTime, __construct)
{
    zend_string *format = NULL;
    zval *timezone = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR(format)
    Z_PARAM_ZVAL(timezone)
    ZEND_PARSE_PARAMETERS_END();

    if (timezone != NULL && !is_valid_timezone(timezone)) {
        zend_argument_type_error(2, "must be of type DateTimeZone|string|null, %s given", zend_zval_type_name(timezone));
        RETURN_THROWS();
    }

    if (timezone != NULL && Z_TYPE_P(timezone) == IS_OBJECT && !Z_PHPTIMEZONE_P(timezone)->initialized) {
        zend_argument_value_error(2, "must be a constructed DateTimeZone object");
        RETURN_THROWS();
    }

    zend_update_property_string(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "format", sizeof("format") - 1, format == NULL ? "X-m-d\\TH:i:sP" : ZSTR_VAL(format));

    if (timezone == NULL) {
        zend_update_property_null(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "timezone", sizeof("timezone") - 1);
    } else {
        zend_update_property(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "timezone", sizeof("timezone") - 1, timezone);
    }
}

/*
 * Prepares the timezone argument for php_date_initialize(): NULL sets
 * *has_timezone to false (the parse uses the default timezone), a
 * DateTimeZone instance is reused and a string names the timezone of a
 * fresh DateTimeZone built through its internal constructor. The
 * result is an owned zval; returns false with a pending exception
 * when the timezone name is invalid.
 */
static bool prepare_timezone_for_parse(zval *result, zval *timezone, bool *has_timezone)
{
    if (Z_TYPE_P(timezone) == IS_NULL) {
        *has_timezone = false;
        return true;
    }

    *has_timezone = true;

    if (Z_TYPE_P(timezone) == IS_OBJECT) {
        ZVAL_COPY(result, timezone);
        return true;
    }

    zend_class_entry *tz_ce = php_date_get_timezone_ce();
    object_init_ex(result, tz_ce);

    zend_function *constructor = zend_hash_str_find_ptr(&tz_ce->function_table, "__construct", sizeof("__construct") - 1);
    zval retval;
    ZVAL_UNDEF(&retval);
    zend_call_known_instance_method_with_1_params(constructor, Z_OBJ_P(result), &retval, timezone);
    if (Z_TYPE(retval) != IS_UNDEF) {
        zval_ptr_dtor(&retval);
    }

    if (EG(exception) != NULL) {
        zval_ptr_dtor(result);
        return false;
    }

    return true;
}

/*
 * #[DateTime(format, timezone)]: the value must parse as a datetime in
 * the declared format, resolved in the declared timezone. String
 * values are parsed with the date extension; DateTimeInterface
 * instances pass as-is (they were already hydrated); other value
 * types fail the constraint.
 */
bool av_validate_DateTime_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *value = field->value;
    bool valid = false;
    if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_date_get_interface_ce())) {
        return true;
    }

    if (Z_TYPE_P(value) == IS_STRING) {
        zval *format = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "format", sizeof("format") - 1, false, NULL);
        zval *timezone = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "timezone", sizeof("timezone") - 1, false, NULL);

        zval tz_obj;
        bool has_timezone;
        if (!prepare_timezone_for_parse(&tz_obj, timezone, &has_timezone)) {
            return false;
        }

        zval date_obj;
        php_date_instantiate(php_date_get_date_ce(), &date_obj);
        valid = php_date_initialize(php_date_obj_from_obj(Z_OBJ_P(&date_obj)), Z_STRVAL_P(value), Z_STRLEN_P(value), ZSTR_VAL(Z_STR_P(format)), has_timezone ? &tz_obj : NULL, 0);
        zval_ptr_dtor(&date_obj);
        if (has_timezone) {
            zval_ptr_dtor(&tz_obj);
        }
    }

    if (valid) {
        return true;
    }

    zval *format = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "format", sizeof("format") - 1, false, NULL);
    zend_string *expected = av_value_to_string(format);
    av_add_field_error_with_expected(AV_ERROR_DATETIME, field, property, errors, expected);
    av_string_release(expected);
    return false;
}

void av_register_DateTime_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "DateTime", class_AV_Fields_Constraints_DateTime_methods);
    AV_Fields_Constraints_DateTime_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_DateTime_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_DateTime_ce, 1, AV_Fields_Field_ce);

    /* Register promoted $format and $timezone properties */
    zend_declare_property_null(AV_Fields_Constraints_DateTime_ce, "format", sizeof("format") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Constraints_DateTime_ce, "timezone", sizeof("timezone") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_DateTime_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
