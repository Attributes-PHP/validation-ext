#ifndef AV_VALIDATE_FUNCTION_H
#define AV_VALIDATE_FUNCTION_H

#include "php.h"
#include "av_model_configs.h"
#include "av_exception.h"

#define AV_ZVAL_DTOR_RETURN_THROWS(...) \
    AV_ZVAL_DTOR_RETURN_THROWS_N(__VA_ARGS__, 2, 1)(__VA_ARGS__)

#define AV_ZVAL_DTOR_RETURN_THROWS_N(_1, _2, N, ...) \
    AV_ZVAL_DTOR_RETURN_THROWS_##N

#define AV_ZVAL_DTOR_RETURN_THROWS_1(arg1) \
    zval_ptr_dtor(arg1); \
    RETURN_THROWS()

#define AV_ZVAL_DTOR_RETURN_THROWS_2(arg1, arg2) \
    zval_ptr_dtor(arg1); \
    zval_ptr_dtor(arg2); \
    RETURN_THROWS()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_AV_validate, 0, 2, Attributes\\Validation\\BaseModel, 0)
    ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
    ZEND_ARG_OBJ_INFO(0, model, Attributes\\Validation\\BaseModel, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(validate);

bool av_validate_model_internal(zval *raw_data, zval *model, zend_class_entry *model_ce, av_model_configs_properties *properties, zval *errors, zend_string *parent_path);

static zend_always_inline zend_string* transform_property_name(zend_string *property_name, char alias_generator);
static zend_always_inline zend_string* get_property_name(zend_class_entry *model_ce, zend_string *property_name, zend_property_info *prop_info, char alias_generator);
static zend_always_inline zval* get_property_value(zend_class_entry *model_ce, zval *raw_data, zend_string *field_name);
static inline bool validate_field_value(zend_string *field_name, zval *value, zend_property_info *prop_info, av_model_configs_properties *properties, zval *errors, zend_string *parent_path);

#endif /* AV_VALIDATE_FUNCTION_H */
