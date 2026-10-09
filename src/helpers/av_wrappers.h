/*
 * Mockable wrappers for Zend internals and macros to ease unit testing.
 *
 * In production builds (TESTING not defined), simple passthrough functions are
 * implemented as macros that directly call their Zend equivalents for zero overhead.
 * In test builds (TESTING defined), they are real functions that can be mocked
 * using CMock.
 *
 * Functions with actual logic (not just passthrough) remain as real functions
 * in both modes.
 */

#ifndef AV_HELPERS_AV_WRAPPERS_H
#define AV_HELPERS_AV_WRAPPERS_H

#include <Zend/zend_types.h>

/*
 * =============================================================================
 * SIMPLE PASS-THROUGH WRAPPERS
 * =============================================================================
 * These are implemented as macros in production for zero overhead, and as
 * functions during testing for mockability.
 */

#ifndef TESTING
/* Production mode: macros that directly call Zend functions */

#define av_string_init(str, len, persistent)                              zend_string_init(str, len, persistent)
#define av_string_release(s)                                              zend_string_release(s)
#define av_emalloc(size)                                                  emalloc(size)
#define av_efree(ptr)                                                     efree(ptr)
#define av_string_alloc(length, persistent)                               zend_string_alloc(length, persistent)
#define av_string_truncate(s, length, persistent)                         zend_string_truncate(s, length, persistent)
#define av_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len) zend_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len)
#define av_string_copy(s)                                                 zend_string_copy(s)
#define av_long_to_str(num)                                               zend_long_to_str(num)
#define av_double_to_str(num)                                             zend_double_to_str(num)
#define av_is_stringable(instance_ce)                                     instanceof_function(instance_ce, zend_ce_stringable)
#define av_rsrc_list_get_rsrc_type(res)                                   zend_rsrc_list_get_rsrc_type(res)
#define av_zval_ptr_dtor(zval_ptr)                                        zval_ptr_dtor(zval_ptr)
#define av_memnstr(haystack, needle, needle_len, end)                     php_memnstr(haystack, needle, needle_len, end)
#define av_hash_find(ht, key)                                             zend_hash_find(ht, key)
#define av_hash_next_index_insert(ht, pData)                              zend_hash_next_index_insert(ht, pData)
#define av_hash_add(ht, key, pData)                                       zend_hash_add(ht, key, pData)
#define av_new_array(size)                                                zend_new_array(size)
#define av_lookup_class_ex(name, lcname, flags)                           zend_lookup_class_ex(name, lcname, flags)
#define av_update_class_constants(ce)                                     zend_update_class_constants(ce)
#define av_zval_update_constant_ex(zv, scope)                             zval_update_constant_ex(zv, scope)
#define av_get_attribute_str(attributes, str, len)                        zend_get_attribute_str(attributes, str, len)
#define av_get_attribute_value(ret, attribute, arg_num, scope)            zend_get_attribute_value(ret, attribute, arg_num, scope)
#define av_throw_value_error(message)                                     zend_throw_exception_ex(zend_ce_value_error, 0, "%s", message)
#define av_binary_strcasecmp(s1, len1, s2, len2)                          zend_binary_strcasecmp(s1, len1, s2, len2)
#define av_hash_init(ht, size, dtor, persistent)                          zend_hash_init(ht, size, NULL, dtor, persistent)
#define av_hash_destroy(ht)                                               zend_hash_destroy(ht)
#define av_hash_index_find_ptr(ht, h)                                     zend_hash_index_find_ptr(ht, h)
#define av_hash_index_add_ptr(ht, h, data)                                zend_hash_index_add_ptr(ht, h, data)
#define av_has_exception()                                                (EG(exception) != NULL)

#else
/* Testing mode: function declarations for CMock */

zend_string *av_string_init(const char *str, size_t len, bool persistent);
void av_string_release(zend_string *s);
void *av_emalloc(size_t size);
void av_efree(void *ptr);
zend_string *av_string_alloc(size_t length, bool persistent);
zend_string *av_string_truncate(zend_string *s, size_t length, bool persistent);

/* Additional wrappers used by av_value_to_string to allow mocking in unit tests. */
zend_string *av_string_concat3(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len);
zend_string *av_string_copy(zend_string *s);
zend_string *av_long_to_str(zend_long num);
zend_string *av_double_to_str(double num);
bool av_is_stringable(const zend_class_entry *instance_ce);
const char *av_rsrc_list_get_rsrc_type(zend_resource *res);
void av_zval_ptr_dtor(zval *zval_ptr);

/* Additional wrappers used by av_replace_placeholders to allow mocking in unit tests. */
const char *av_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end);

/* Additional wrappers used by av_error_messages to keep the unit-test build
 * free of unresolved Zend/library symbols. Every Zend Hash Table, class lookup
 * and constant evaluation call in av_error_messages.c goes through these so
 * the whole translation unit can be linked into Ceedling tests. */
zval *av_hash_find(const HashTable *ht, zend_string *key);
zval *av_hash_next_index_insert(HashTable *ht, zval *pData);
zval *av_hash_add(HashTable *ht, zend_string *key, zval *pData);
HashTable *av_new_array(uint32_t size);
zend_class_entry *av_lookup_class_ex(zend_string *name, zend_string *lcname, uint32_t flags);
zend_result av_update_class_constants(zend_class_entry *ce);
zend_result av_zval_update_constant_ex(zval *zv, zend_class_entry *scope);

/* Additional wrappers used by the error message template logic to read
 * #[ErrorMessage] attributes and to throw exceptions without pulling real
 * Zend symbols into the unit-test build. The attribute pointer is passed as
 * void * so the mockable declarations stay free of Zend attribute types. */
void *av_get_attribute_str(HashTable *attributes, const char *str, size_t len);
zend_result av_get_attribute_value(zval *ret, void *attribute, uint32_t arg_num, zend_class_entry *scope);
void av_throw_value_error(const char *message);
int av_binary_strcasecmp(const char *s1, size_t len1, const char *s2, size_t len2);

/* Additional wrappers used by the per-property error templates cache in
 * av_error_messages.c, so its hash table operations stay mockable and the
 * unit-test build remains free of unresolved Zend/library symbols. */
void av_hash_init(HashTable *ht, uint32_t size, dtor_func_t dtor, bool persistent);
void av_hash_destroy(HashTable *ht);
void *av_hash_index_find_ptr(HashTable *ht, zend_ulong h);
void *av_hash_index_add_ptr(HashTable *ht, zend_ulong h, void *data);
bool av_has_exception(void);

#endif /* TESTING */

/*
 * =============================================================================
 * FUNCTIONS WITH LOGIC
 * =============================================================================
 * These always remain as real functions because they contain actual logic,
 * not just passthrough to Zend functions.
 */

zend_result av_call_tostring(zend_object *object, zval *retval);
void av_zval_stringl(zval *z, const char *str, size_t len);

#endif /* AV_HELPERS_AV_WRAPPERS_H */
