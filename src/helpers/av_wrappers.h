/*
 * Mockable wrappers for Zend internals and macros to ease unit testing.
 */

#ifndef AV_HELPERS_AV_WRAPPERS_H
#define AV_HELPERS_AV_WRAPPERS_H

#include <Zend/zend_types.h>

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
zend_result av_call_tostring(zend_object *object, zval *retval);
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
void av_zval_stringl(zval *z, const char *str, size_t len);
zend_class_entry *av_lookup_class_ex(zend_string *name, zend_string *lcname, uint32_t flags);
int av_snprintf(char *buffer, size_t size, const char *format, ...) ZEND_ATTRIBUTE_FORMAT(printf, 3, 4);
double av_fmax(double a, double b);

#endif /* AV_HELPERS_AV_WRAPPERS_H */
