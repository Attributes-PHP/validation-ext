#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "attributes_validation.h"

/* Include the component headers */
#include "src/av_base_model.h"
#include "src/av_exception.h"
#include "src/av_model_configs.h"
#include "src/fields/av_field.h"
#include "src/fields/av_alias.h"

/* Module startup */
PHP_MINIT_FUNCTION(attributes_validation)
{
    // Register main classes
    av_register_BaseModel_class();
    av_register_ModelConfigs_class();
    av_register_all_exception_classes();

    // Register fields
    av_register_Field_interface();
    av_register_Alias_class();
    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(attributes_validation)
{
    return SUCCESS;
}

/* Request startup */
PHP_RINIT_FUNCTION(attributes_validation)
{
    return SUCCESS;
}

/* Request shutdown */
PHP_RSHUTDOWN_FUNCTION(attributes_validation)
{
    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(attributes_validation)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "attributes_validation", "enabled");
    php_info_print_table_end();
    
    DISPLAY_INI_ENTRIES();
}

/* Module entry */
zend_module_entry attributes_validation_module_entry = {
    STANDARD_MODULE_HEADER,
    EXTENSION_NAME,
    ext_functions,
    PHP_MINIT(attributes_validation),
    PHP_MSHUTDOWN(attributes_validation),
    PHP_RINIT(attributes_validation),
    PHP_RSHUTDOWN(attributes_validation),
    PHP_MINFO(attributes_validation),
    EXTENSION_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_ATTRIBUTES_VALIDATION
ZEND_GET_MODULE(attributes_validation)
#endif
