#include "av_datetime.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_operators.h"
#include <ext/date/php_date.h>

zend_class_entry *AV_Fields_Constraints_DateTime_ce;

static bool is_valid_timezone(zval *value);
static bool build_timezone_from_name(zval *result, zval *name);
zend_always_inline static void add_invalid_datetime_error_message(zval *format, av_field *field, av_property_info *property, zval *errors);

/*
 * $timezone: DateTimeZone|string|null
 *
 * Only the type is checked here; a timezone name is resolved by
 * build_timezone_from_name() so a malformed value fails at
 * construction instead of at validation time.
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

/*
 * Builds a DateTimeZone for a timezone name through its internal
 * constructor. The result is an owned zval; returns false with a
 * pending exception when the timezone name is invalid.
 */
static bool build_timezone_from_name(zval *result, zval *name)
{
    zend_class_entry *tz_ce = php_date_get_timezone_ce();
    object_init_ex(result, tz_ce);

    zend_function *constructor = zend_hash_str_find_ptr(&tz_ce->function_table, "__construct", sizeof("__construct") - 1);
    ZEND_ASSERT(constructor != NULL);
    zend_call_known_instance_method_with_1_params(constructor, Z_OBJ_P(result), NULL, name);

    if (EG(exception) != NULL) {
        zval_ptr_dtor(result);
        return false;
    }

    return true;
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

    if (timezone == NULL) {
        zend_update_property_string(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "format", sizeof("format") - 1, format == NULL ? "X-m-d\\TH:i:sP" : ZSTR_VAL(format));
        zend_update_property_null(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "timezone", sizeof("timezone") - 1);
        return;
    }

    if (!is_valid_timezone(timezone)) {
        zend_argument_type_error(2, "must be of type DateTimeZone|string|null, %s given", zend_zval_type_name(timezone));
        RETURN_THROWS();
    }

    /*
     * $timezone keeps the raw value while the resolved DateTimeZone
     * is cached in the private $timezone_obj, so validation reuses it
     * instead of resolving the name a second time.
     */
    if (Z_TYPE_P(timezone) == IS_OBJECT) {
        if (!Z_PHPTIMEZONE_P(timezone)->initialized) {
            zend_argument_value_error(2, "must be a constructed DateTimeZone object");
            RETURN_THROWS();
        }
        zend_update_property(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "timezone_obj", sizeof("timezone_obj") - 1, timezone);
    } else if (Z_TYPE_P(timezone) == IS_STRING) {
        zval tz_obj;
        if (!build_timezone_from_name(&tz_obj, timezone)) {
            zend_clear_exception();
            zend_argument_value_error(2, "must be a valid timezone identifier, '%s' given", Z_STRVAL_P(timezone));
            RETURN_THROWS();
        }
        zend_update_property(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "timezone_obj", sizeof("timezone_obj") - 1, &tz_obj);
        zval_ptr_dtor(&tz_obj);
    }

    zend_update_property_string(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "format", sizeof("format") - 1, format == NULL ? "X-m-d\\TH:i:sP" : ZSTR_VAL(format));
    zend_update_property(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(getThis()), "timezone", sizeof("timezone") - 1, timezone);
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
    if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), php_date_get_interface_ce())) {
        return true;
    }

    if (Z_TYPE_P(value) != IS_STRING) {
        zval *format = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "format", sizeof("format") - 1, false, NULL);
        add_invalid_datetime_error_message(format, field, property, errors);
        return false;
    }

    zval *format = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "format", sizeof("format") - 1, false, NULL);
    zval *timezone_obj = zend_read_property(AV_Fields_Constraints_DateTime_ce, Z_OBJ_P(constraint), "timezone_obj", sizeof("timezone_obj") - 1, false, NULL);

    zval date_obj;
    php_date_instantiate(php_date_get_date_ce(), &date_obj);
    const bool valid = php_date_initialize(php_date_obj_from_obj(Z_OBJ_P(&date_obj)), Z_STRVAL_P(value), Z_STRLEN_P(value), ZSTR_VAL(Z_STR_P(format)), Z_TYPE_P(timezone_obj) == IS_OBJECT ? timezone_obj : NULL, 0);
    if (!valid)
        add_invalid_datetime_error_message(format, field, property, errors);

    zval_ptr_dtor(&date_obj);
    return valid;
}

zend_always_inline static void add_invalid_datetime_error_message(zval *format, av_field *field, av_property_info *property, zval *errors)
{
    zend_string *expected = av_value_to_string(format);
    av_add_field_error_with_expected(AV_ERROR_DATETIME, field, property, errors, expected);
    av_string_release(expected);
}

void av_register_DateTime_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "DateTime", class_AV_Fields_Constraints_DateTime_methods);
    AV_Fields_Constraints_DateTime_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_DateTime_ce != NULL);
    AV_Fields_Constraints_DateTime_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_DateTime_ce, 1, AV_Fields_Field_ce);

    /* Register promoted $format and $timezone properties */
    zend_declare_property_null(AV_Fields_Constraints_DateTime_ce, "format", sizeof("format") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Constraints_DateTime_ce, "timezone", sizeof("timezone") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Constraints_DateTime_ce, "timezone_obj", sizeof("timezone_obj") - 1, ZEND_ACC_PRIVATE);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_DateTime_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
