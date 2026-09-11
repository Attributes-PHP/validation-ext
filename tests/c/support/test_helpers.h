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

#endif /* TEST_HELPERS_H */
