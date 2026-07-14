#include "model_configs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "helpers/options.h"
#include "zend_type_info.h"

zend_class_entry *Attributes_Validation_ModelConfigs_ce;

ZEND_METHOD(Attributes_Validation_ModelConfigs, getAliasGenerator)
{

}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrToLower)
{

}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrToUpper)
{

}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStripWhitespace)
{

}

ZEND_METHOD(Attributes_Validation_ModelConfigs, getExtra)
{

}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrict)
{

}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStopAtFirstError)
{

}

/* Registration function */
void register_ModelConfigs_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation", "ModelConfigs", class_Attributes_Validation_ModelConfigs_methods);
    Attributes_Validation_ModelConfigs_ce = zend_register_internal_class_ex(&ce, NULL);
    Attributes_Validation_ModelConfigs_ce->ce_flags |= ZEND_ACC_FINAL;

    /* Declare properties */
    declare_typed_property_string("aliasGenerator", NULL, true);
    declare_typed_property_bool("strToLower", false);
    declare_typed_property_bool("strToUpper", false);
    declare_typed_property_bool("stripWhitespace", false);
    declare_typed_property_string("extra", "ignore", false);
    declare_typed_property_bool("strict", false);
    declare_typed_property_bool("stopAtFirstError", false);

    /* Register as an internal attribute that targets classes only */
    zend_internal_attribute_register(
        Attributes_Validation_ModelConfigs_ce,
        ZEND_ATTRIBUTE_TARGET_CLASS
    );
}

/**
* Instantiates a ModelConfigs class with all required properties for validation
**/
void create_model_configs(zval *configs, zval *model, validation_ext_model_configs_properties *properties)
{
    object_init_ex(configs, Attributes_Validation_ModelConfigs_ce);

    set_default_properties(properties);
    update_model_properties(Z_OBJ_P(configs), properties, NULL, "ignore");
}

void update_model_properties(zend_object *this, validation_ext_model_configs_properties *properties, char *pretty_alias_generator, char *pretty_extra)
{
    if (pretty_alias_generator == NULL) {
        zend_update_property_null(Attributes_Validation_ModelConfigs_ce, this, "aliasGenerator", sizeof("aliasGenerator") - 1);
    } else {
        zend_update_property_string(Attributes_Validation_ModelConfigs_ce, this, "aliasGenerator", sizeof("aliasGenerator") - 1, pretty_alias_generator);
    }

    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "strToLower", sizeof("strToLower") - 1, properties->str_to_lower);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "strToUpper", sizeof("strToUpper") - 1, properties->str_to_upper);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "stripWhitespace", sizeof("stripWhitespace") - 1, properties->strip_whitespace);
    zend_update_property_string(Attributes_Validation_ModelConfigs_ce, this, "extra", sizeof("extra") - 1, pretty_extra);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "strict", sizeof("strict") - 1, properties->strict);
    zend_update_property_bool(Attributes_Validation_ModelConfigs_ce, this, "stopAtFirstError", sizeof("stopAtFirstError") - 1, properties->stop_first_error);
}

void set_default_properties(validation_ext_model_configs_properties *properties)
{
    properties->alias_generator = false;
    properties->str_to_lower = false;
    properties->str_to_upper = false;
    properties->strip_whitespace = false;
    properties->extra = VALIDATION_EXT_IGNORE;
    properties->strict = false;
    properties->stop_first_error = false;
}

bool validate_alias_generator(char *pretty_alias_generator)
{
    char *all_pretty_alias[] = {"pascal", "camel", "snake", "kebab"};
    validation_ext_invalid_method_parameter invalid_parameter_error = {
        .class_name = "Attributes\\Validation\\ModelConfigs",
        .method_name = "__construct()",
        .parameter_number = 1,
        .name = "aliasGenerator"
    };
    return validate_method_parameter(pretty_alias_generator, all_pretty_alias, 4, &invalid_parameter_error);
}

bool validate_extra(char *pretty_extra)
{
    char *all_pretty_extra[] = {"ignore", "forbid", "allow"};
    validation_ext_invalid_method_parameter invalid_parameter_error = {
        .class_name = "Attributes\\Validation\\ModelConfigs",
        .method_name = "__construct()",
        .parameter_number = 5,
        .name = "extra"
    };
    return validate_method_parameter(pretty_extra, all_pretty_extra, 3, &invalid_parameter_error);
}

static inline void declare_typed_property_bool(const char *name, bool default_value)
{
    zval z_default_value;
    ZVAL_BOOL(&z_default_value, default_value);
    declare_typed_property(name, &z_default_value, MAY_BE_BOOL);
}

static inline void declare_typed_property_string(const char *name, char *default_value, bool allow_null)
{
    zval z_default_value;
    if (default_value == NULL) ZVAL_NULL(&z_default_value);
    else {
        zend_string *default_string = zend_string_init(default_value, strlen(default_value), 1);
        ZVAL_STR(&z_default_value, default_string);
    }

    zend_uchar type = allow_null ? MAY_BE_STRING|MAY_BE_NULL : MAY_BE_STRING;
    declare_typed_property(name, &z_default_value, type);
}

static inline void declare_typed_property(const char *name, zval *default_value, zend_uchar type)
{
    zend_type property_type = (zend_type) ZEND_TYPE_INIT_MASK(type);
    zend_string *property_name = zend_string_init(name, strlen(name), 1);
    zend_declare_typed_property(Attributes_Validation_ModelConfigs_ce, property_name, default_value, ZEND_ACC_PRIVATE, NULL, property_type);
    zend_string_release(property_name);
}
