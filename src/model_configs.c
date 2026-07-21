#include "model_configs.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "helpers/options.h"
#include "zend_type_info.h"
#include "zend_types.h"

zend_class_entry *Attributes_Validation_ModelConfigs_ce;

/**
 * Reads a property from the current object and returns it.
 * Used by all getter methods in this class.
 */
#define ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN(prop_name) \
    ZEND_PARSE_PARAMETERS_NONE(); \
    zval rv, *value; \
    value = zend_read_property(Attributes_Validation_ModelConfigs_ce, Z_OBJ_P(getThis()), prop_name, sizeof(prop_name) - 1, 1, &rv); \
    RETURN_COPY_DEREF(value)

ZEND_METHOD(Attributes_Validation_ModelConfigs, getAliasGenerator)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("aliasGenerator");
}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrToLower)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("strToLower");
}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrToUpper)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("strToUpper");
}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStripWhitespace)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("stripWhitespace");
}

ZEND_METHOD(Attributes_Validation_ModelConfigs, getExtra)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("extra");
}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrict)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("strict");
}

ZEND_METHOD(Attributes_Validation_ModelConfigs, isStopAtFirstError)
{
    ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN("stopAtFirstError");
}

/* Registration function */
void attributes_validation_register_ModelConfigs_class(void)
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
 * Instantiates a ModelConfigs class with all required properties for validation.
 * If the model class (or any parent class) has a ModelConfigs attribute, its values
 * are used. Otherwise, default properties are applied.
 * Inheritance is supported: if Child extends Parent and Parent has ModelConfigs,
 * those configs are used for Child unless Child overrides them with its own ModelConfigs.
 */
void attributes_validation_create_model_configs(zval *configs, zval *model, attributes_validation_model_configs_properties *properties)
{
    object_init_ex(configs, Attributes_Validation_ModelConfigs_ce);

    zend_class_entry *base_model_class_entry = Z_OBJCE_P(model);
    zend_attribute *model_configs_attr = get_model_configs_attribute(base_model_class_entry);

    set_default_properties(properties);

    if (model_configs_attr == NULL) {
        update_model_properties(Z_OBJ_P(configs), properties, NULL, "ignore");
        return;
    }

    char *pretty_alias_generator = NULL;
    char *pretty_extra = "ignore";

    /* Parse attribute arguments */
    for (uint32_t i = 0; i < model_configs_attr->argc; i++) {
        zval arg_val;
        if (zend_get_attribute_value(&arg_val, model_configs_attr, i, base_model_class_entry) != SUCCESS) continue;

        zend_attribute_arg argument = model_configs_attr->args[i];
        int index = argument.name == NULL ? i : get_argument_index_by_name(argument.name);

        switch (index) {
            case 0: /* aliasGenerator */
                if (Z_TYPE(arg_val) == IS_STRING) {
                    pretty_alias_generator = Z_STRVAL(arg_val);
                    if (!validate_alias_generator(pretty_alias_generator)) return;

                    properties->alias_generator = pretty_alias_generator[0];
                }
                break;
            case 1: /* strToLower */
                properties->str_to_lower = Z_TYPE(arg_val) == IS_TRUE;
                break;
            case 2: /* strToUpper */
                properties->str_to_upper = Z_TYPE(arg_val) == IS_TRUE;
                break;
            case 3: /* stripWhitespace */
                properties->strip_whitespace = Z_TYPE(arg_val) == IS_TRUE;
                break;
            case 4: /* extra */
                if (Z_TYPE(arg_val) == IS_STRING) {
                    pretty_extra = Z_STRVAL(arg_val);
                    if (!validate_extra(pretty_extra)) return;

                    properties->extra = pretty_extra[0];
                }
                break;
            case 5: /* strict */
                properties->strict = Z_TYPE(arg_val) == IS_TRUE;
                break;
            case 6: /* stopAtFirstError */
                properties->stop_first_error = Z_TYPE(arg_val) == IS_TRUE;
                break;
            default:
                zval_ptr_dtor(&arg_val);
                return;
        }
        zval_ptr_dtor(&arg_val);
    }

    update_model_properties(Z_OBJ_P(configs), properties, pretty_alias_generator, pretty_extra);
}

static void update_model_properties(zend_object *this, attributes_validation_model_configs_properties *properties, char *pretty_alias_generator, char *pretty_extra)
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

static void set_default_properties(attributes_validation_model_configs_properties *properties)
{
    properties->alias_generator = false;
    properties->str_to_lower = false;
    properties->str_to_upper = false;
    properties->strip_whitespace = false;
    properties->extra = ATTRIBUTES_VALIDATION_IGNORE;
    properties->strict = false;
    properties->stop_first_error = false;
}

static bool validate_alias_generator(char *pretty_alias_generator)
{
    char *all_pretty_alias[] = {"pascal", "camel", "snake", "kebab"};
    attributes_validation_invalid_method_parameter invalid_parameter_error = {
        .class_name = "Attributes\\Validation\\ModelConfigs",
        .method_name = "__construct()",
        .parameter_number = 1,
        .name = "aliasGenerator"
    };
    return attributes_validation_validate_method_parameter(pretty_alias_generator, all_pretty_alias, 4, &invalid_parameter_error);
}

static bool validate_extra(char *pretty_extra)
{
    char *all_pretty_extra[] = {"ignore", "forbid", "allow"};
    attributes_validation_invalid_method_parameter invalid_parameter_error = {
        .class_name = "Attributes\\Validation\\ModelConfigs",
        .method_name = "__construct()",
        .parameter_number = 5,
        .name = "extra"
    };
    return attributes_validation_validate_method_parameter(pretty_extra, all_pretty_extra, 3, &invalid_parameter_error);
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

/**
 * Walks up the inheritance chain to find the first ModelConfigs attribute
 */
static zend_attribute* get_model_configs_attribute(zend_class_entry *base_model_class_entry)
{
    zend_class_entry *current_ce = base_model_class_entry;
    zend_attribute *model_configs_attr = NULL;
    while (current_ce != NULL) {
        if (current_ce->attributes != NULL) {
            model_configs_attr = zend_get_attribute_str(
                current_ce->attributes,
                "attributes\\validation\\modelconfigs",
                sizeof("attributes\\validation\\modelconfigs") - 1
            );
            if (model_configs_attr != NULL) {
                return model_configs_attr;
            }
        }
        current_ce = current_ce->parent;
    }

    return NULL;
}

static int get_argument_index_by_name(zend_string *name)
{
    size_t len = ZSTR_LEN(name);
    const char *val = ZSTR_VAL(name);

    switch (len) {
        case 14: // "aliasGenerator"
            if (memcmp(val, "aliasGenerator", 14) == 0) return 0;
            break;

        case 10: // "strToLower", "strToUpper"
            // Both are length 10. Check the 4th character ('L' vs 'U') to differentiate instantly.
            if (val[5] == 'L' && memcmp(val, "strToLower", 10) == 0) return 1;
            if (val[5] == 'U' && memcmp(val, "strToUpper", 10) == 0) return 2;
            break;

        case 15: // "stripWhitespace"
            if (memcmp(val, "stripWhitespace", 15) == 0) return 3;
            break;

        case 5: // "extra"
            if (memcmp(val, "extra", 5) == 0) return 4;
            break;

        case 6: // "strict"
            if (memcmp(val, "strict", 6) == 0) return 5;
            break;

        case 16: // "stopAtFirstError"
            if (memcmp(val, "stopAtFirstError", 16) == 0) return 6;
            break;
    }
    
    zend_throw_exception_ex(zend_ce_error, 0, "Unknown named parameter $%s", ZSTR_VAL(name));
    return -1;
}

#undef ATTRIBUTES_VALIDATION_GET_PROPERTY_AND_RETURN
