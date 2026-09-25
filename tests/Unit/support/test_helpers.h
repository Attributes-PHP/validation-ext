#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "helpers/mock_av_wrappers.h"

// Stub declarations for av_wrappers mocks
extern zend_string *string_init_stub(const char *str, size_t len, bool persistent, int num_calls);
extern void string_release_stub(zend_string *s, int num_calls);
extern void *emalloc_stub(size_t size, int num_calls);
extern void efree_stub(void *ptr, int num_calls);
extern zend_string *string_alloc_stub(size_t length, bool persistent, int num_calls);
extern zend_string *string_truncate_stub(zend_string *s, size_t length, bool persistent, int num_calls);
extern const char *memnstr_stub(const char *haystack, const char *needle, size_t needle_len, const char *end, int num_calls);
extern zend_string *concat3_stub(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len, int num_calls);
extern zend_string *string_copy_stub(zend_string *s, int num_calls);
extern zend_string *long_to_str_stub(zend_long num, int num_calls);
extern zend_string *double_to_str_stub(double num, int num_calls);

// Stubs for the PHP snprintf API (php.h #defines snprintf/vsnprintf to
// ap_php_snprintf/ap_php_vsnprintf). The Ceedling unit-test build has no
// Zend library to link against, so these delegate to the compiler builtins so
// the test files can keep using snprintf() naturally after including php.h.
extern int ap_php_snprintf(char *buf, size_t len, const char *format, ...);
extern int ap_php_vsnprintf(char *buf, size_t len, const char *format, va_list ap);

#endif /* TEST_HELPERS_H */
