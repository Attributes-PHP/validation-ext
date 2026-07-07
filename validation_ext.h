/*
 * Header for validation-ext PHP extension
 */

#ifndef VALIDATION_EXT_H
#define VALIDATION_EXT_H

#define VALIDATION_EXT_NAME "validation_ext"
#define VALIDATION_EXT_VERSION "1.0.0"

#include "src/call_function.h"
#include "src/validate_function.h"

static const zend_function_entry ext_functions[] = {
    ZEND_NS_FE("Attributes\\Validation", validate, arginfo_Attributes_Validation_validate)
    ZEND_NS_FE("Attributes\\Validation", call, arginfo_Attributes_Validation_call)
    ZEND_FE_END
};

#endif /* VALIDATION_EXT_H */
