#ifndef ATTRIBUTES_VALIDATION_MODEL_CONFIG_H
#define ATTRIBUTES_VALIDATION_MODEL_CONFIG_H

#include "php.h"
#include "Zend/zend_attributes.h"

/* Class entry */
extern zend_class_entry *Attributes_Validation_ModelConfigs_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_getAliasGenerator, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_isStrToLower, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_isStrToUpper, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_isStripWhitespace, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_getExtra, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_isStrict, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs_isStopAtFirstError, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Attributes_Validation_ModelConfigs, getAliasGenerator);
ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrToLower);
ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrToUpper);
ZEND_METHOD(Attributes_Validation_ModelConfigs, isStripWhitespace);
ZEND_METHOD(Attributes_Validation_ModelConfigs, getExtra);
ZEND_METHOD(Attributes_Validation_ModelConfigs, isStrict);
ZEND_METHOD(Attributes_Validation_ModelConfigs, isStopAtFirstError);

static const zend_function_entry class_Attributes_Validation_ModelConfigs_methods[] = {
    ZEND_ME(Attributes_Validation_ModelConfigs, getAliasGenerator, arginfo_class_Attributes_Validation_ModelConfigs_getAliasGenerator, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_ModelConfigs, isStrToLower, arginfo_class_Attributes_Validation_ModelConfigs_isStrToLower, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_ModelConfigs, isStrToUpper, arginfo_class_Attributes_Validation_ModelConfigs_isStrToUpper, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_ModelConfigs, isStripWhitespace, arginfo_class_Attributes_Validation_ModelConfigs_isStripWhitespace, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_ModelConfigs, getExtra, arginfo_class_Attributes_Validation_ModelConfigs_getExtra, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_ModelConfigs, isStrict, arginfo_class_Attributes_Validation_ModelConfigs_isStrict, ZEND_ACC_PUBLIC)
    ZEND_ME(Attributes_Validation_ModelConfigs, isStopAtFirstError, arginfo_class_Attributes_Validation_ModelConfigs_isStopAtFirstError, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

// Alias
#define ATTRIBUTES_VALIDATION_PASCAL_CASE 'p'
#define ATTRIBUTES_VALIDATION_CAMEL_CASE 'c'
#define ATTRIBUTES_VALIDATION_SNAKE_CASE 's'
#define ATTRIBUTES_VALIDATION_KEBAB_CASE 'k'

// Extra
#define ATTRIBUTES_VALIDATION_IGNORE 'i'
#define ATTRIBUTES_VALIDATION_FORBID 'f'
#define ATTRIBUTES_VALIDATION_ALLOW 'a'

typedef struct {
    char alias_generator;
    bool str_to_lower;
    bool str_to_upper;
    bool strip_whitespace;
    char extra;
    bool strict;
    bool stop_first_error;
} attributes_validation_model_configs_properties;

/* Registration function */
void attributes_validation_register_ModelConfigs_class(void);
void attributes_validation_create_model_configs(zval *configs, zval *model, attributes_validation_model_configs_properties *properties);
static void update_model_properties(zend_object *this, attributes_validation_model_configs_properties *properties, char *pretty_alias_generator, char *pretty_extra);
static zend_always_inline void set_default_properties(attributes_validation_model_configs_properties *properties);
static bool validate_alias_generator(char *pretty_alias_generator);
static bool validate_extra(char *pretty_alias_generator);
static zend_always_inline void declare_typed_property_bool(const char *name, bool default_value);
static zend_always_inline void declare_typed_property_string(const char *name, char *default_value, bool allow_null);
static zend_always_inline void declare_typed_property(const char *name, zval *default_value, zend_uchar type);
static zend_attribute* get_model_configs_attribute(zend_class_entry *base_model_class_entry);
static int get_argument_index_by_name(zend_string *name);

#endif /* ATTRIBUTES_VALIDATION_MODEL_CONFIG_H */
