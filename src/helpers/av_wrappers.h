/*
 * Mockable wrappers for Zend internals and macros to ease unit testing.
 */

#ifndef AV_HELPERS_AV_WRAPPERS_H
#define AV_HELPERS_AV_WRAPPERS_H

#include <Zend/zend_types.h>
#include <Zend/zend_string.h>

zend_string* av_string_init(const char *str, size_t len, bool persistent);
void av_string_release(zend_string *s);
void* av_emalloc(size_t size);
void av_efree(void *ptr);

/* Additional wrappers used by av_value_to_string to allow mocking in unit tests. */
zend_string* av_string_concat3(const char *str1, size_t str1_len,
                               const char *str2, size_t str2_len,
                               const char *str3, size_t str3_len);
zend_string* av_string_copy(zend_string *s);
zend_string* av_long_to_str(zend_long num);
zend_string* av_double_to_str(double num);
bool av_instanceof_function(const zend_class_entry *instance_ce, const zend_class_entry *ce);
bool av_is_stringable(const zend_class_entry *instance_ce);
const char* av_rsrc_list_get_rsrc_type(zend_resource *res);
zend_result av_call_tostring(zend_object *object, zval *retval);
void av_zval_ptr_dtor(zval *zval_ptr);

#endif /* AV_HELPERS_AV_WRAPPERS_H */
