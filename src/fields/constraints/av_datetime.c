#include "av_datetime.h"
#include "../av_field.h"
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
