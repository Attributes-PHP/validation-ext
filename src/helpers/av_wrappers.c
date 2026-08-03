#include "av_wrappers.h"
#include "Zend/zend_API.h"

/*
 * Wrapper implementations for Zend internals.
 * 
 * These provide a mockable interface to Zend functions for testing.
 */

zend_string* av_string_init(const char *str, size_t len, bool persistent) {
    return zend_string_init(str, len, persistent);
}

void av_string_release(zend_string *s) {
    zend_string_release(s);
}

void* av_emalloc(size_t size) {
    return emalloc(size);
}

void av_efree(void *ptr) {
    efree(ptr);
}
