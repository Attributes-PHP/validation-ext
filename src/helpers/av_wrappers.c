#include "av_wrappers.h"
#include "php.h"
#include "Zend/zend_API.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_list.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_variables.h"

/*
 * Wrapper implementations for Zend internals.
 * 
 * These provide a mockable interface to Zend functions for testing.
 */

zend_string *av_string_init(const char *str, size_t len, bool persistent)
{
    return zend_string_init(str, len, persistent);
}

void av_string_release(zend_string *s)
{
    zend_string_release(s);
}

void *av_emalloc(size_t size)
{
    return emalloc(size);
}

void av_efree(void *ptr)
{
    efree(ptr);
}

zend_string *av_string_alloc(size_t length, bool persistent)
{
    return zend_string_alloc(length, persistent);
}

zend_string *av_string_truncate(zend_string *s, size_t length, bool persistent)
{
    return zend_string_truncate(s, length, persistent);
}

zend_string *av_string_concat3(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len)
{
    return zend_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len);
}

zend_string *av_string_copy(zend_string *s)
{
    return zend_string_copy(s);
}

zend_string *av_long_to_str(zend_long num)
{
    return zend_long_to_str(num);
}

zend_string *av_double_to_str(double num)
{
    return zend_double_to_str(num);
}

bool av_instanceof_function(const zend_class_entry *instance_ce, const zend_class_entry *ce)
{
    return instanceof_function(instance_ce, ce);
}

bool av_is_stringable(const zend_class_entry *instance_ce)
{
    return instanceof_function(instance_ce, zend_ce_stringable);
}

const char *av_rsrc_list_get_rsrc_type(zend_resource *res)
{
    return zend_rsrc_list_get_rsrc_type(res);
}

zend_result av_call_tostring(zend_object *object, zval *retval)
{
    ZVAL_UNDEF(retval);
    zend_call_method_with_0_params(object, NULL, NULL, "__tostring", retval);
    if (Z_TYPE_P(retval) != IS_STRING) {
        if (Z_TYPE_P(retval) != IS_UNDEF) {
            zval_ptr_dtor(retval);
            ZVAL_UNDEF(retval);
        }
        return FAILURE;
    }
    return SUCCESS;
}

void av_zval_ptr_dtor(zval *zval_ptr)
{
    zval_ptr_dtor(zval_ptr);
}

const char *av_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end)
{
    return php_memnstr(haystack, needle, needle_len, end);
}
