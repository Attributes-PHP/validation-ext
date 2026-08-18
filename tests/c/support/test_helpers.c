#include "unity.h"
#include "helpers/mock_av_wrappers.h"
#include <Zend/zend_types.h>
#include <stdlib.h>
#include <string.h>

// Stub implementations for av_wrappers to be used in tests
// These provide real implementations that the tests can use

zend_string* string_init_stub(const char *str, size_t len, bool persistent, int num_calls) {
    // zend_string has val[1] so we need sizeof(zend_string) - 1 + len + 1 = sizeof(zend_string) + len
    zend_string *s = malloc(sizeof(zend_string) + len);
    if (s) {
        s->gc.refcount = 1;
        s->gc.u.type_info = 0;
        s->h = 0; // No hash for testing
        s->len = len;
        if (len > 0) {
            memcpy(s->val, str, len);
        }
        s->val[len] = '\0';
    }
    return s;
}

void string_release_stub(zend_string *s, int num_calls) {
    if (s) {
        free(s);
    }
}

void* emalloc_stub(size_t size, int num_calls) {
    return malloc(size);
}

void efree_stub(void *ptr, int num_calls) {
    free(ptr);
}
