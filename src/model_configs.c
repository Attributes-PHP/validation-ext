#include "src/model_configs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_interfaces.h"

zend_class_entry *validation_ext_ModelConfigs_ce;

/* Method entries */
static zend_function_entry validation_ext_ModelConfigs_methods[] = {
    ZEND_ME(ModelConfigs, __construct, arginfo_ModelConfigs___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_FE_END
};

/* Method implementations */
PHP_METHOD(ModelConfigs, __construct)
{
    bool is_alias_generator_null = 1;

    model_configs_properties properties;
    set_default_properties(&properties);

    ZEND_PARSE_PARAMETERS_START(0, 7)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG_OR_NULL(properties.alias_generator, is_alias_generator_null)
        Z_PARAM_BOOL(properties.str_to_lower)
        Z_PARAM_BOOL(properties.str_to_upper)
        Z_PARAM_BOOL(properties.strip_whitespace)
        Z_PARAM_LONG(properties.extra)
        Z_PARAM_BOOL(properties.strict)
        Z_PARAM_BOOL(properties.stop_first_error)
    ZEND_PARSE_PARAMETERS_END();

    if (is_alias_generator_null) properties.alias_generator = validation_ext_ModelConfig_AliasGenerator_NONE;
    update_model_properties(Z_OBJ_P(getThis()), &properties);
}

/* Registration function */
void register_ModelConfigs_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\ModelConfigs", validation_ext_ModelConfigs_methods);

    validation_ext_ModelConfigs_ce = zend_register_internal_class(&ce);

    zend_long extra = validation_ext_ModelConfig_Extra_IGNORE;

    /* Declare properties */
    zend_declare_property_null(validation_ext_ModelConfigs_ce, "aliasGenerator", sizeof("aliasGenerator") - 1, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(validation_ext_ModelConfigs_ce, "strToLower", sizeof("strToLower") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(validation_ext_ModelConfigs_ce, "strToUpper", sizeof("strToUpper") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(validation_ext_ModelConfigs_ce, "stripWhitespace", sizeof("stripWhitespace") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_long(validation_ext_ModelConfigs_ce, "extra", sizeof("extra") - 1, extra, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(validation_ext_ModelConfigs_ce, "strict", sizeof("strict") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(validation_ext_ModelConfigs_ce, "stopAtFirstError", sizeof("stopAtFirstError") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);

    /* Register as an internal attribute that targets classes only */
    zend_internal_attribute_register(
        validation_ext_ModelConfigs_ce,
        ZEND_ATTRIBUTE_TARGET_CLASS
    );
}

/**
* Instantiates a ModelConfigs class with all required properties for validation
**/
void create_model_configs(zval *configs, bool strict, bool is_strict_null, bool stop_first_error, bool is_stop_first_error_null)
{
    object_init_ex(configs, validation_ext_ModelConfigs_ce);
    model_configs_properties properties;
    set_default_properties(&properties);

    if (!is_strict_null) properties.strict = strict;
    if (!is_stop_first_error_null) properties.stop_first_error = stop_first_error;

    update_model_properties(Z_OBJ_P(configs), &properties);
}

void update_model_properties(zend_object *this, model_configs_properties *properties)
{
    if (properties->alias_generator == validation_ext_ModelConfig_AliasGenerator_NONE) {
        zend_update_property_long(validation_ext_ModelConfigs_ce, this, "aliasGenerator", sizeof("aliasGenerator") - 1, properties->alias_generator);
    } else {
        zend_update_property_null(validation_ext_ModelConfigs_ce, this, "aliasGenerator", sizeof("aliasGenerator") - 1);
    }

    zend_update_property_bool(validation_ext_ModelConfigs_ce, this, "strToLower", sizeof("strToLower") - 1, properties->str_to_lower);
    zend_update_property_bool(validation_ext_ModelConfigs_ce, this, "strToUpper", sizeof("strToUpper") - 1, properties->str_to_upper);
    zend_update_property_bool(validation_ext_ModelConfigs_ce, this, "stripWhitespace", sizeof("stripWhitespace") - 1, properties->strip_whitespace);
    zend_update_property_long(validation_ext_ModelConfigs_ce, this, "extra", sizeof("extra") - 1, properties->extra);
    zend_update_property_bool(validation_ext_ModelConfigs_ce, this, "strict", sizeof("strict") - 1, properties->strict);
    zend_update_property_bool(validation_ext_ModelConfigs_ce, this, "stopAtFirstError", sizeof("stopAtFirstError") - 1, properties->stop_first_error);
}

void set_default_properties(model_configs_properties *properties)
{
    properties->alias_generator = validation_ext_ModelConfig_AliasGenerator_NONE;
    properties->str_to_lower = 0;
    properties->str_to_upper = 0;
    properties->strip_whitespace = 0;
    properties->extra = validation_ext_ModelConfig_Extra_IGNORE;
    properties->strict = 0;
    properties->stop_first_error = 0;
}
