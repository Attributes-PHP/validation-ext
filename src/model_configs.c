#include "model_configs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "zend_alloc.h"
#include <stddef.h>
#include <string.h>

zend_class_entry *Attributes_Validation_ModelConfigs_ce;

/* Method implementations */
ZEND_METHOD(Attributes_Validation_ModelConfigs, __construct)
{
    bool is_extra_null = false;
    validation_ext_model_configs_properties properties;
    set_default_properties(&properties);

    ZEND_PARSE_PARAMETERS_START(0, 7)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING_OR_NULL(properties.alias_generator.value, properties.alias_generator.len)
        Z_PARAM_BOOL(properties.str_to_lower)
        Z_PARAM_BOOL(properties.str_to_upper)
        Z_PARAM_BOOL(properties.strip_whitespace)
        Z_PARAM_STRING(properties.extra.value, properties.extra.len)
        Z_PARAM_BOOL(properties.strict)
        Z_PARAM_BOOL(properties.stop_first_error)
    ZEND_PARSE_PARAMETERS_END();

    if (properties.extra.len <= 0) {
        properties.extra.value = estrdup(VALIDATION_EXT_ALLOW);
        properties.extra.len = sizeof(VALIDATION_EXT_ALLOW) - 1;
        is_extra_null = true;
    }

    update_model_properties(Z_OBJ_P(getThis()), &properties);
    if (is_extra_null) efree(properties.extra.value);
}

/* Registration function */
void register_ModelConfigs_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation", "ModelConfigs", class_Attributes_Validation_ModelConfigs_methods);
    Attributes_Validation_ModelConfigs_ce = zend_register_internal_class_ex(&ce, NULL);
    Attributes_Validation_ModelConfigs_ce->ce_flags |= ZEND_ACC_FINAL;

    /* Declare properties */
    zend_declare_property_null(Attributes_Validation_ModelConfigs_ce, "aliasGenerator", sizeof("aliasGenerator") - 1, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(Attributes_Validation_ModelConfigs_ce, "strToLower", sizeof("strToLower") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(Attributes_Validation_ModelConfigs_ce, "strToUpper", sizeof("strToUpper") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(Attributes_Validation_ModelConfigs_ce, "stripWhitespace", sizeof("stripWhitespace") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_string(Attributes_Validation_ModelConfigs_ce, "extra", sizeof("extra") - 1, VALIDATION_EXT_ALLOW, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(Attributes_Validation_ModelConfigs_ce, "strict", sizeof("strict") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);
    zend_declare_property_bool(Attributes_Validation_ModelConfigs_ce, "stopAtFirstError", sizeof("stopAtFirstError") - 1, 0, ZEND_ACC_PUBLIC | ZEND_ACC_READONLY);

    /* Register as an internal attribute that targets classes only */
    zend_internal_attribute_register(
        Attributes_Validation_ModelConfigs_ce,
        ZEND_ATTRIBUTE_TARGET_CLASS
    );
}

/**
* Instantiates a ModelConfigs class with all required properties for validation
**/
void create_model_configs(zval *configs)
{
    object_init_ex(configs, Attributes_Validation_ModelConfigs_ce);
    validation_ext_model_configs_properties properties;

    set_default_properties(&properties);
    properties.extra.value = estrdup(VALIDATION_EXT_ALLOW);
    properties.extra.len = sizeof(VALIDATION_EXT_ALLOW) - 1;

    update_model_properties(Z_OBJ_P(configs), &properties);
    efree(properties.extra.value);
}

void update_model_properties(zend_object *this, validation_ext_model_configs_properties *properties)
{
    if (properties->alias_generator.len <= 0) {
        zend_update_property_null(Attributes_Validation_ModelConfigs_ce, this, "aliasGenerator", sizeof("aliasGenerator") - 1);
    } else {
        zend_update_property_string(Attributes_Validation_ModelConfigs_ce, this, "aliasGenerator", sizeof("aliasGenerator") - 1, properties->alias_generator.value);
    }

    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "strToLower", sizeof("strToLower") - 1, properties->str_to_lower);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "strToUpper", sizeof("strToUpper") - 1, properties->str_to_upper);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "stripWhitespace", sizeof("stripWhitespace") - 1, properties->strip_whitespace);
    zend_update_property_string(Attributes_Validation_ModelConfigs_ce, this, "extra", sizeof("extra") - 1, properties->extra.value);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "strict", sizeof("strict") - 1, properties->strict);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "stopAtFirstError", sizeof("stopAtFirstError") - 1, properties->stop_first_error);
}

void set_default_properties(validation_ext_model_configs_properties *properties)
{
    properties->alias_generator.value = NULL;
    properties->alias_generator.len = 0;
    properties->str_to_lower = false;
    properties->str_to_upper = false;
    properties->strip_whitespace = false;
    properties->extra.value = NULL;
    properties->extra.len = 0;
    properties->strict = false;
    properties->stop_first_error = false;
}
