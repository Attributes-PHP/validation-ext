#include "av_wrappers.h"
#include "php.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_hash.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_list.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_variables.h"

/*
 * Wrapper implementations for Zend internals.
 *
 * Simple passthrough functions are only compiled in TESTING mode.
 * In production, they are macros defined in av_wrappers.h.
 *
 * Functions with actual logic remain as real functions in both modes.
 */

#ifdef TESTING

/* Simple passthrough wrappers - only needed for testing/mocking */

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

bool av_is_stringable(const zend_class_entry *instance_ce)
{
    return instanceof_function(instance_ce, zend_ce_stringable);
}

const char *av_rsrc_list_get_rsrc_type(zend_resource *res)
{
    return zend_rsrc_list_get_rsrc_type(res);
}

void av_zval_ptr_dtor(zval *zval_ptr)
{
    zval_ptr_dtor(zval_ptr);
}

const char *av_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end)
{
    return php_memnstr(haystack, needle, needle_len, end);
}

zval *av_hash_find(const HashTable *ht, zend_string *key)
{
    return zend_hash_find(ht, key);
}

zval *av_hash_next_index_insert(HashTable *ht, zval *pData)
{
    return zend_hash_next_index_insert(ht, pData);
}

zval *av_hash_add(HashTable *ht, zend_string *key, zval *pData)
{
    return zend_hash_add(ht, key, pData);
}

HashTable *av_new_array(uint32_t size)
{
    return zend_new_array(size);
}

zend_class_entry *av_lookup_class_ex(zend_string *name, zend_string *lcname, uint32_t flags)
{
    return zend_lookup_class_ex(name, lcname, flags);
}

zend_result av_update_class_constants(zend_class_entry *ce)
{
    return zend_update_class_constants(ce);
}

zend_result av_zval_update_constant_ex(zval *zv, zend_class_entry *scope)
{
    return zval_update_constant_ex(zv, scope);
}

void *av_get_attribute_str(HashTable *attributes, const char *str, size_t len)
{
    return zend_get_attribute_str(attributes, str, len);
}

zend_result av_get_attribute_value(zval *ret, void *attribute, uint32_t arg_num, zend_class_entry *scope)
{
    return zend_get_attribute_value(ret, (const zend_attribute *)attribute, arg_num, scope);
}

void av_throw_value_error(const char *message)
{
    zend_throw_exception_ex(zend_ce_value_error, 0, "%s", message);
}

int av_binary_strcasecmp(const char *s1, size_t len1, const char *s2, size_t len2)
{
    return zend_binary_strcasecmp(s1, len1, s2, len2);
}

#endif /* TESTING */

/*
 * =============================================================================
 * FUNCTIONS WITH LOGIC
 * =============================================================================
 * These always remain as real functions because they contain actual logic,
 * not just passthrough to Zend functions. They are needed in both modes.
 */

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

void av_zval_stringl(zval *z, const char *str, size_t len)
{
    ZVAL_NEW_STR(z, av_string_init(str, len, 0));
}
