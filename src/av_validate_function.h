#ifndef AV_VALIDATE_FUNCTION_H
#define AV_VALIDATE_FUNCTION_H

#include "php.h"
#include "av_model_configs.h"
#include "av_exception.h"
#include "helpers/av_structs.h"

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_AV_validate, 0, 2, Attributes\\Validation\\BaseModel, 0)
ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
ZEND_ARG_OBJ_INFO(0, model, Attributes\\Validation\\BaseModel, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(validate);

bool av_validate_model_internal(zval *raw_data, av_property_info *property_info, av_model_configs_properties *properties, zval *errors, zend_string *parent_path);

/**
 * Runs the full validation flow for one model instance: ModelConfigs,
 * before/after validation hooks and property hydration from raw data.
 *
 * @param raw_data  The raw data to hydrate the model from
 * @param model     The model instance to fill in
 * @return          SUCCESS with the model filled in, or FAILURE after
 *                  throwing (ValidationException or a hook exception)
 */
zend_result av_hydrate_model(zval *raw_data, zval *model);

static zend_always_inline zend_string *transform_property_name(zend_string *property_name, char alias_generator);
static zend_always_inline zend_string *get_property_name(av_property_info *property_info, zend_string *property_name, char alias_generator);
static zend_always_inline zval *get_property_value(zend_class_entry *model_ce, zval *raw_data, zend_string *field_name);
static inline bool validate_field_value(av_field *field, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors);

#endif /* AV_VALIDATE_FUNCTION_H */
