#include "unity.h"
#include "helpers/mock_av_wrappers.h"
#include <Zend/zend_types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Stub implementations for av_wrappers to be used in tests
// These provide real implementations that the tests can use

zend_string *string_init_stub(const char *str, size_t len, bool persistent, int num_calls)
{
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

void string_release_stub(zend_string *s, int num_calls)
{
    if (s) {
        free(s);
    }
}

void *emalloc_stub(size_t size, int num_calls)
{
    return malloc(size);
}

void efree_stub(void *ptr, int num_calls)
{
    free(ptr);
}

zend_string *string_alloc_stub(size_t length, bool persistent, int num_calls)
{
    zend_string *s = malloc(sizeof(zend_string) + length);
    if (s) {
        s->gc.refcount = 1;
        s->gc.u.type_info = 0;
        s->h = 0;
        s->len = length;
        s->val[length] = '\0';
    }
    return s;
}

zend_string *string_truncate_stub(zend_string *s, size_t length, bool persistent, int num_calls)
{
    if (s) {
        s->len = length;
        s->val[length] = '\0';
    }
    return s;
}

const char *memnstr_stub(const char *haystack, const char *needle, size_t needle_len, const char *end, int num_calls)
{
    (void)num_calls;
    if (needle_len == 0)
        return haystack;

    size_t haystack_len = (size_t)(end - haystack);
    if (haystack_len < needle_len)
        return NULL;

    for (const char *p = haystack; p <= end - needle_len; p++) {
        if (memcmp(p, needle, needle_len) == 0)
            return p;
    }
    return NULL;
}

// PHP's php.h #defines snprintf -> ap_php_snprintf and vsnprintf ->
// ap_php_vsnprintf. The unit-test build has no Zend library, so provide these
// symbols delegating to the compiler builtins (which ignore the macros).
int ap_php_snprintf(char *buf, size_t len, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = __builtin_vsnprintf(buf, len, format, args);
    va_end(args);
    return result;
}

int ap_php_vsnprintf(char *buf, size_t len, const char *format, va_list ap)
{
    return __builtin_vsnprintf(buf, len, format, ap);
}
