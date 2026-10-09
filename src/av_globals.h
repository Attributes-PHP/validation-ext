/*
 * Per-request module globals: the request-scoped caches live here
 * instead of file-scope statics, so each thread gets its own copy
 * under ZTS. The engine calls the GINIT ctor (zeros the pointers)
 * when the module starts, or per thread via TSRM under ZTS.
 */

#ifndef AV_GLOBALS_H
#define AV_GLOBALS_H

#include "Zend/zend_API.h"

ZEND_BEGIN_MODULE_GLOBALS(attributes_validation)
HashTable *av_model_configs_cache;
HashTable *av_field_name_cache;
HashTable *av_plans_cache;
HashTable *av_spec_cache;
ZEND_END_MODULE_GLOBALS(attributes_validation)

#define AV_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(attributes_validation, v)

ZEND_EXTERN_MODULE_GLOBALS(attributes_validation)

#endif /* AV_GLOBALS_H */
