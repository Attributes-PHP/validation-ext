/*
 * Mockable wrappers for Zend internals and macros to ease unit testing.
 */

#ifndef AV_HELPERS_AV_WRAPPERS_H
#define AV_HELPERS_AV_WRAPPERS_H

#include <Zend/zend_types.h>

zend_string* av_string_init(const char *str, size_t len, bool persistent);
void av_string_release(zend_string *s);
void* av_emalloc(size_t size);
void av_efree(void *ptr);

#endif /* AV_HELPERS_AV_WRAPPERS_H */