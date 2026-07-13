#ifndef VALIDATION_EXT_MODEL_CONFIG_H
#define VALIDATION_EXT_MODEL_CONFIG_H

#include "php.h"

/* Class entry */
extern zend_class_entry *Attributes_Validation_ModelConfigs_ce;

/* Method declaration */
ZEND_METHOD(Attributes_Validation_ModelConfigs, __construct);

/* Argument info for ModelConfig constructor */
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs___construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, aliasGenerator, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strToLower, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strToUpper, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stripWhitespace, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extra, IS_STRING, 0, "\"ignore\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopAtFirstError, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

static const zend_function_entry class_Attributes_Validation_ModelConfigs_methods[] = {
    ZEND_ME(Attributes_Validation_ModelConfigs, __construct, arginfo_class_Attributes_Validation_ModelConfigs___construct, ZEND_ACC_PUBLIC)
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
void register_ModelConfigs_class(void);
void create_model_configs(zval *configs);
void update_model_properties(zend_object *this, validation_ext_model_configs_properties *properties, char *pretty_alias_generator, char *pretty_extra);
void set_default_properties(validation_ext_model_configs_properties *properties);
bool validate_alias_generator(char *pretty_alias_generator);
bool validate_extra(char *pretty_alias_generator);

#endif /* VALIDATION_EXT_MODEL_CONFIG_H */
