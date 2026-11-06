#include "av_regex.h"
#include "../av_field.h"
#include "../../helpers/av_error_messages.h"
#include "../../helpers/av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include <ext/pcre/php_pcre.h>

zend_class_entry *AV_Fields_Constraints_Regex_ce;

/*
 * #[Regex(pattern)]: the string value must match the regular expression
 * (a partial match suffices, mirroring preg_match()). Non-string
 * values fail the constraint; an invalid pattern fails the value and
 * raises the PCRE compilation warning.
 */
bool av_validate_Regex_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    zval *regex = zend_read_property(Z_OBJCE_P(constraint), Z_OBJ_P(constraint), "regex", sizeof("regex") - 1, false, NULL);

    zval *value = field->value;
    if (Z_TYPE_P(value) != IS_STRING) {
        add_invalid_regex_error_message(regex, field, property, errors);
        return false;
    }

    pcre_cache_entry *pce = pcre_get_compiled_regex_cache(Z_STR_P(regex));
    if (pce == NULL) {
        add_invalid_regex_error_message(regex, field, property, errors);
        return false;
    }

    zval match;
    ZVAL_UNDEF(&match);
#if PHP_VERSION_ID >= 80400
    /* PHP 8.4 removed the use_flags argument. */
    php_pcre_match_impl(pce, Z_STR_P(value), &match, NULL, 0, 0, 0);
#else
    php_pcre_match_impl(pce, Z_STR_P(value), &match, NULL, 0, 0, 0, 0);
#endif
    const bool is_valid = (Z_TYPE(match) == IS_LONG && Z_LVAL(match) > 0);
    if (!is_valid)
        add_invalid_regex_error_message(regex, field, property, errors);
    return is_valid;
}

zend_always_inline static void add_invalid_regex_error_message(zval *regex, av_field *field, av_property_info *property, zval *errors);

zend_always_inline static void add_invalid_regex_error_message(zval *regex, av_field *field, av_property_info *property, zval *errors)
{
    zend_string *expected = av_value_to_string(regex);
    av_add_field_error_with_expected(AV_ERROR_REGEX, field, property, errors, expected);
    av_string_release(expected);
}

ZEND_METHOD(AV_Fields_Constraints_Regex, __construct)
{
    zend_string *regex;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(regex)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(AV_Fields_Constraints_Regex_ce, Z_OBJ_P(getThis()), "regex", sizeof("regex") - 1, ZSTR_VAL(regex));
}

void av_register_Regex_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields\\Constraints", "Regex", class_AV_Fields_Constraints_Regex_methods);
    AV_Fields_Constraints_Regex_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Constraints_Regex_ce != NULL);
    AV_Fields_Constraints_Regex_ce->ce_flags |= ZEND_ACC_FINAL;

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraints_Regex_ce, 1, AV_Fields_Field_ce);

    zend_declare_property_null(AV_Fields_Constraints_Regex_ce, "regex", sizeof("regex") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Constraints_Regex_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
