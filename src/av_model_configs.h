#ifndef AV_MODEL_CONFIG_H
#define AV_MODEL_CONFIG_H

#include "php.h"
#include "Zend/zend_attributes.h"

/* Class entry */
extern zend_class_entry *AV_ModelConfigs_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_getAliasGenerator, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_isStrToLower, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_isStrToUpper, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_isStripWhitespace, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_getExtra, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_isStrict, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_isStopAtFirstError, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_ModelConfigs_setDefaultErrorMessages, 0, 1, IS_VOID, 0)
ZEND_ARG_TYPE_INFO(0, setErrorMessages, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_ModelConfigs, getAliasGenerator);
ZEND_METHOD(AV_ModelConfigs, isStrToLower);
ZEND_METHOD(AV_ModelConfigs, isStrToUpper);
ZEND_METHOD(AV_ModelConfigs, isStripWhitespace);
ZEND_METHOD(AV_ModelConfigs, getExtra);
ZEND_METHOD(AV_ModelConfigs, isStrict);
ZEND_METHOD(AV_ModelConfigs, isStopAtFirstError);
ZEND_METHOD(AV_ModelConfigs, setDefaultErrorMessages);

static const zend_function_entry class_AV_ModelConfigs_methods[] = {
    ZEND_ME(AV_ModelConfigs, getAliasGenerator, arginfo_class_AV_ModelConfigs_getAliasGenerator, ZEND_ACC_PUBLIC) ZEND_ME(AV_ModelConfigs, isStrToLower, arginfo_class_AV_ModelConfigs_isStrToLower, ZEND_ACC_PUBLIC)
        ZEND_ME(AV_ModelConfigs, isStrToUpper, arginfo_class_AV_ModelConfigs_isStrToUpper, ZEND_ACC_PUBLIC) ZEND_ME(AV_ModelConfigs, isStripWhitespace, arginfo_class_AV_ModelConfigs_isStripWhitespace, ZEND_ACC_PUBLIC)
            ZEND_ME(AV_ModelConfigs, getExtra, arginfo_class_AV_ModelConfigs_getExtra, ZEND_ACC_PUBLIC) ZEND_ME(AV_ModelConfigs, isStrict, arginfo_class_AV_ModelConfigs_isStrict, ZEND_ACC_PUBLIC)
                ZEND_ME(AV_ModelConfigs, isStopAtFirstError, arginfo_class_AV_ModelConfigs_isStopAtFirstError, ZEND_ACC_PUBLIC)
                    ZEND_ME(AV_ModelConfigs, setDefaultErrorMessages, arginfo_class_AV_ModelConfigs_setDefaultErrorMessages, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC) ZEND_FE_END
};

// Alias
#define AV_PASCAL_CASE 'p'
#define AV_CAMEL_CASE  'c'
#define AV_SNAKE_CASE  's'
#define AV_KEBAB_CASE  'k'

// Extra
#define AV_IGNORE 'i'
#define AV_FORBID 'f'
#define AV_ALLOW  'a'

typedef struct {
    char alias_generator;
    bool str_to_lower;
    bool str_to_upper;
    bool strip_whitespace;
    char extra;
    bool strict;
    bool stop_first_error;
} av_model_configs_properties;

/* Registration function */
void av_register_ModelConfigs_class(void);
void av_get_model_configs(zval *configs, zval *model, av_model_configs_properties *properties, bool build_object);
void av_clear_model_configs_cache(void);

#endif /* AV_MODEL_CONFIG_H */
