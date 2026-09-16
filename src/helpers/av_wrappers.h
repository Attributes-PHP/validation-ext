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

#define av_string_init(str, len, persistent) zend_string_init(str, len, persistent)
#define av_string_release(s) zend_string_release(s)
#define av_emalloc(size) emalloc(size)
#define av_efree(ptr) efree(ptr)
#define av_string_alloc(length, persistent) zend_string_alloc(length, persistent)
#define av_string_truncate(s, length, persistent) zend_string_truncate(s, length, persistent)
#define av_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len) \
    zend_string_concat3(str1, str1_len, str2, str2_len, str3, str3_len)
#define av_string_copy(s) zend_string_copy(s)
#define av_long_to_str(num) zend_long_to_str(num)
#define av_double_to_str(num) zend_double_to_str(num)
#define av_instanceof_function(instance_ce, ce) instanceof_function(instance_ce, ce)
#define av_is_stringable(instance_ce) instanceof_function(instance_ce, zend_ce_stringable)
#define av_rsrc_list_get_rsrc_type(res) zend_rsrc_list_get_rsrc_type(res)
#define av_zval_ptr_dtor(zval_ptr) zval_ptr_dtor(zval_ptr)
#define av_memnstr(haystack, needle, needle_len, end) php_memnstr(haystack, needle, needle_len, end)
#define av_hash_find(ht, key) zend_hash_find(ht, key)
#define av_hash_next_index_insert(ht, pData) zend_hash_next_index_insert(ht, pData)
#define av_hash_add(ht, key, pData) zend_hash_add(ht, key, pData)
#define av_new_array(size) zend_new_array(size)
#define av_lookup_class_ex(name, lcname, flags) zend_lookup_class_ex(name, lcname, flags)
#define av_fmax(a, b) fmax(a, b)

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
bool av_instanceof_function(const zend_class_entry *instance_ce, const zend_class_entry *ce);
bool av_is_stringable(const zend_class_entry *instance_ce);
const char *av_rsrc_list_get_rsrc_type(zend_resource *res);
void av_zval_ptr_dtor(zval *zval_ptr);

/* Additional wrappers used by av_replace_placeholders to allow mocking in unit tests. */
const char *av_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end);

/* Additional wrappers used by av_error_messages to keep the unit-test build
 * free of unresolved Zend/library symbols. Every Zend Hash Table, class lookup,
 * string formatting and math call in av_error_messages.c goes through these so
 * the whole translation unit can be linked into Ceedling tests. */
zval *av_hash_find(const HashTable *ht, zend_string *key);
zval *av_hash_next_index_insert(HashTable *ht, zval *pData);
zval *av_hash_add(HashTable *ht, zend_string *key, zval *pData);
HashTable *av_new_array(uint32_t size);
zend_class_entry *av_lookup_class_ex(zend_string *name, zend_string *lcname, uint32_t flags);
int av_snprintf(char *buffer, size_t size, const char *format, ...) ZEND_ATTRIBUTE_FORMAT(printf, 3, 4);
double av_fmax(double a, double b);

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
