#ifndef AV_VALIDATE_FUNCTION_H
#define AV_VALIDATE_FUNCTION_H

#include "php.h"
#include "av_model_configs.h"

#define AV_ERROR_REQUIRED "Required field", sizeof("Required field") - 1
#define AV_ERROR_TYPE_HINT "Invalid type", sizeof("Invalid type") - 1

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_AV_validate, 0, 2, Attributes\\Validation\\BaseModel, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
    ZEND_ARG_OBJ_INFO(0, model, Attributes\\Validation\\BaseModel, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(validate);

static zend_always_inline zend_string* transform_property_name(zend_string *property_name, char alias_generator);
static zend_always_inline zend_string* get_property_name(zend_class_entry *model_ce, zend_string *property_name, zend_property_info *prop_info, char alias_generator);
static zend_always_inline zval* get_property_value(zend_class_entry *model_ce, zval *raw_data, zend_string *field_name);
static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, char *error_message, size_t length);
static inline bool validate_type_hint(zend_string *field_name, zval *value, zend_property_info *prop_info, av_model_configs_properties properties, zval *errors);
static inline zval *validate_field_value(zend_string *field_name, zval *value, zend_property_info *prop_info, av_model_configs_properties properties, zval *errors);

#endif /* AV_VALIDATE_FUNCTION_H */
