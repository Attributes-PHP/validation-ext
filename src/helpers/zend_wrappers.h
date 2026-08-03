/*
 * Mockable wrappers for Zend internals and macros to ease unit testing.
 */

#ifndef ATTRIBUTES_VALIDATION_HELPERS_ZEND_WRAPPERS_H
#define ATTRIBUTES_VALIDATION_HELPERS_ZEND_WRAPPERS_H

#include <Zend/zend_types.h>

zend_string* attributes_validation_string_init(const char *str, size_t len, bool persistent);
void attributes_validation_string_release(zend_string *s);
void* attributes_validation_emalloc(size_t size);
void attributes_validation_efree(void *ptr);

#endif /* ATTRIBUTES_VALIDATION_HELPERS_ZEND_WRAPPERS_H */