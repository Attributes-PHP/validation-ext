#ifndef VALIDATION_EXT_MODEL_CONFIG_H
#define VALIDATION_EXT_MODEL_CONFIG_H

#include "php.h"

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
#define VALIDATION_EXT_PASCAL_CASE 'p'
#define VALIDATION_EXT_CAMEL_CASE 'c'
#define VALIDATION_EXT_SNAKE_CASE 's'
#define VALIDATION_EXT_KEBAB_CASE 'k'

// Extra
#define VALIDATION_EXT_IGNORE 'i'
#define VALIDATION_EXT_FORBID 'f'
#define VALIDATION_EXT_ALLOW 'a'

typedef struct {
    char alias_generator;
    bool str_to_lower;
    bool str_to_upper;
    bool strip_whitespace;
    char extra;
    bool strict;
    bool stop_first_error;
} validation_ext_model_configs_properties;

/* Registration function */
void attributes_validation_register_ModelConfigs_class(void);
void attributes_validation_create_model_configs(zval *configs, zval *model, validation_ext_model_configs_properties *properties);
static void update_model_properties(zend_object *this, validation_ext_model_configs_properties *properties, char *pretty_alias_generator, char *pretty_extra);
static void set_default_properties(validation_ext_model_configs_properties *properties);
static bool validate_alias_generator(char *pretty_alias_generator);
static bool validate_extra(char *pretty_alias_generator);
static inline void declare_typed_property_bool(const char *name, bool default_value);
static inline void declare_typed_property_string(const char *name, char *default_value, bool allow_null);
static inline void declare_typed_property(const char *name, zval *default_value, zend_uchar type);

#endif /* VALIDATION_EXT_MODEL_CONFIG_H */
