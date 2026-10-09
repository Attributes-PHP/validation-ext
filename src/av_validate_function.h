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

#endif /* AV_VALIDATE_FUNCTION_H */
