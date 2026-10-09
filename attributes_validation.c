#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "attributes_validation.h"

/* Include the component headers */
#include "src/av_base_model.h"
#include "src/av_compiled_model.h"
#include "src/av_exception.h"
#include "src/av_model_configs.h"
#include "src/av_type.h"
#include "src/fields/av_field.h"
#include "src/fields/av_alias.h"
#include "src/fields/av_dict.h"
#include "src/fields/av_error_message.h"
#include "src/fields/av_intersection.h"
#include "src/fields/av_sequence.h"
#include "src/fields/av_union.h"
#include "src/validators/av_typehint_validator.h"

/* Module startup */
PHP_MINIT_FUNCTION(attributes_validation)
{
    // Register main classes
    av_register_BaseModel_class();
    av_register_ModelConfigs_class();
    av_register_all_exception_classes();

    // Register the Type enum and its namespace constants
    av_register_Type_enum();
    av_register_type_constants(module_number);

    // Register fields
    av_register_Field_interface();
    av_register_Alias_class();
    av_register_ErrorMessage_class();
    av_register_Sequence_class();
    av_register_Union_class();
    av_register_Intersection_class();
    av_register_Dict_class();
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
    // Initialize DateTime classes (must run after all MINITs)
    av_init_typehint_validator();
    return SUCCESS;
}

/* Request shutdown */
PHP_RSHUTDOWN_FUNCTION(attributes_validation)
{
    // Plans borrow specs from the array spec cache and names from the
    // field name cache: clearing the plans first is the only ordering
    // that matters, as dtors never dereference borrowed pointers
    av_clear_compiled_model_cache();
    av_clear_model_configs_cache();
    av_clear_array_spec_cache();
    av_clear_field_name_cache();

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
