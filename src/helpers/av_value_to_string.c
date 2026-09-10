#include "av_value_to_string.h"
#include "av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_operators.h"
#include <string.h>

/*
 * Converts any PHP zval into a zend_string suitable for inclusion in an
 * error message template (the {value} placeholder).
 *
 * The conversion is fully type-aware so error messages stay readable:
 *   null     -> "null"
 *   bool     -> "true" / "false"
 *   int      -> numeric string
 *   double   -> numeric string
 *   string   -> single-quoted value
 *   array    -> "array"
 *   object   -> __toString() result when Stringable, else the class name
 *   resource -> resource type name, or "resource"
 *
 * All Zend internals are reached through the mockable av_wrappers so the
 * function can be unit tested in isolation.
 */
zend_string *av_value_to_string(zval *value)
{
    if (value == NULL) {
        return av_string_init("null", sizeof("null") - 1, 0);
    }

    ZVAL_DEREF(value);

    switch (Z_TYPE_P(value)) {
        case IS_NULL:
            return av_string_init("null", sizeof("null") - 1, 0);
        case IS_TRUE:
            return av_string_init("true", sizeof("true") - 1, 0);
        case IS_FALSE:
            return av_string_init("false", sizeof("false") - 1, 0);
        case IS_LONG:
            return av_long_to_str(Z_LVAL_P(value));
        case IS_DOUBLE:
            return av_double_to_str(Z_DVAL_P(value));
        case IS_STRING:
            return av_string_concat3("'", 1, Z_STRVAL_P(value), Z_STRLEN_P(value), "'", 1);
        case IS_ARRAY:
            return av_string_init("array", sizeof("array") - 1, 0);
        case IS_OBJECT:
            if (av_is_stringable(Z_OBJCE_P(value))) {
                zval result;
                if (av_call_tostring(Z_OBJ_P(value), &result) == SUCCESS) {
                    zend_string *str = av_string_concat3("'", 1, Z_STRVAL(result), Z_STRLEN(result), "'", 1);
                    av_zval_ptr_dtor(&result);
                    return str;
                }
            }
            return av_string_copy(Z_OBJCE_P(value)->name);
        case IS_RESOURCE: {
            const char *type_name = av_rsrc_list_get_rsrc_type(Z_RES_P(value));
            const char *fallback = "resource";
            return av_string_init(type_name ? type_name : fallback, strlen(type_name ? type_name : fallback), 0);
        }
        default:
            return av_string_init("(unknown)", sizeof("(unknown)") - 1, 0);
    }
}
