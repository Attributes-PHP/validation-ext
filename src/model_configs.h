#ifndef VALIDATION_EXT_MODEL_CONFIG_H
#define VALIDATION_EXT_MODEL_CONFIG_H

#include "php.h"

/* Class entry */
extern zend_class_entry *validation_ext_ModelConfigs_ce;

/* Method declaration */
PHP_METHOD(ModelConfigs, __construct);

/* Argument info for ModelConfig constructor */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ModelConfigs___construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, aliasGenerator, IS_LONG, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strToLower, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strToUpper, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stripWhitespace, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extra, IS_LONG, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopAtFirstError, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

typedef enum {
    validation_ext_ModelConfig_AliasGenerator_NONE = -1,
    validation_ext_ModelConfig_AliasGenerator_PASCAL,
    validation_ext_ModelConfig_AliasGenerator_CAMEL,
    validation_ext_ModelConfig_AliasGenerator_SNAKE,
    validation_ext_ModelConfig_AliasGenerator_KEBAB,
} validation_ext_ModelConfigs_enum_alias_generator;

typedef enum {
    validation_ext_ModelConfig_Extra_IGNORE = 0,
    validation_ext_ModelConfig_Extra_FORBID,
    validation_ext_ModelConfig_Extra_ALLOW,
} validation_ext_ModelConfigs_enum_extra;

typedef struct {
    zend_long alias_generator;
    bool str_to_lower;
    bool str_to_upper;
    bool strip_whitespace;
    zend_long extra;
    bool strict;
    bool stop_first_error;
} model_configs_properties;

/* Registration function */
void register_ModelConfigs_class();
void create_model_configs(zval *configs, bool strict, bool is_strict_null, bool stop_first_error, bool is_stop_first_error_null);
void update_model_properties(zend_object *this, model_configs_properties *properties);
void set_default_properties(model_configs_properties *properties);

#endif /* VALIDATION_EXT_MODEL_CONFIG_H */
