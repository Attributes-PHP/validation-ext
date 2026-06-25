#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "Zend/zend_API.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_inheritance.h"
#include "ext/standard/info.h"
#include "validation_ext.h"

/* Include the component headers */
#include "src/validate_function.h"
#include "src/call_function.h"
#include "src/base_model.h"
#include "src/arrayof.h"
#include "src/options.h"
#include "src/exception.h"

/* Function entries for the module */
const zend_function_entry validation_ext_functions[] = {
    ZEND_NS_FE("Attributes\\Validation", validate, arginfo_validate)
    ZEND_NS_FE("Attributes\\Validation", call, arginfo_call)
    PHP_FE_END
};

/* Module startup */
PHP_MINIT_FUNCTION(validation_ext)
{
    // Register classes
    register_BaseModel_class();
    register_ArrayOf_class();
    register_Options_class();
    register_all_exception_classes();
    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(validation_ext)
{
    return SUCCESS;
}

/* Request startup */
PHP_RINIT_FUNCTION(validation_ext)
{
    return SUCCESS;
}

/* Request shutdown */
PHP_RSHUTDOWN_FUNCTION(validation_ext)
{
    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(validation_ext)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "validation_ext", "enabled");
    php_info_print_table_end();
    
    DISPLAY_INI_ENTRIES();
}

/* Module entry */
zend_module_entry validation_ext_module_entry = {
    STANDARD_MODULE_HEADER,
    VALIDATION_EXT_NAME,
    validation_ext_functions,
    PHP_MINIT(validation_ext),
    PHP_MSHUTDOWN(validation_ext),
    PHP_RINIT(validation_ext),
    PHP_RSHUTDOWN(validation_ext),
    PHP_MINFO(validation_ext),
    VALIDATION_EXT_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_VALIDATION_EXT
ZEND_GET_MODULE(validation_ext)
#endif
