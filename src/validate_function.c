#include "validate_function.h"
#include "exception.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_attributes.h"
#include "base_model.h"
#include "model_configs.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_types.h"
#include "Zend/zend_type_info.h"
#include "helpers/av_string.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>


/**
 * Transforms a property name based on the alias generator type
 */
static zend_always_inline zend_string* transform_property_name(zend_string *property_name, char alias_generator)
{
    switch (alias_generator) {
        case AV_PASCAL_CASE:
            return av_to_pascal_case(property_name);
        case AV_CAMEL_CASE:
            return av_to_camel_case(property_name);
        case AV_SNAKE_CASE:
            return av_to_snake_case(property_name);
        case AV_KEBAB_CASE:
            return av_to_kebab_case(property_name);
        default:
            return zend_string_copy(property_name);
    }
}

/**
 * Retrieves the property name from the following priority:
 *  1) If the Alias attribute is set uses that value
 *  2) If aliasGenerator is configured, transforms the property name
 *  3) Otherwise uses the property name as-is
 */
static zend_always_inline zend_string* get_property_name(zend_class_entry *model_ce, zend_string *property_name, zend_property_info *prop_info, char alias_generator)
{
    zend_string *field_name;

    // Check for #[Alias] attribute on the property
    if (prop_info->attributes != NULL) {
        zend_attribute *alias_attr = zend_get_attribute_str(
            prop_info->attributes,
            "attributes\\validation\\fields\\alias",
            sizeof("attributes\\validation\\fields\\alias") - 1
        );
        if (alias_attr != NULL && alias_attr->argc > 0) {
            zval attr_value;
            if (zend_get_attribute_value(&attr_value, alias_attr, 0, model_ce) == SUCCESS) {
                if (UNEXPECTED(Z_TYPE(attr_value) != IS_STRING)) {
                    zval_ptr_dtor(&attr_value);
                    zend_argument_type_error(1, "must be of type string, %s given", zend_zval_type_name(&attr_value));
                    return NULL;
                }
                field_name = zend_string_copy(Z_STR(attr_value));
                zval_ptr_dtor(&attr_value);
                return field_name;
            }
        }
    }

    // If no Alias attribute, check for aliasGenerator in ModelConfigs
    if (alias_generator != false) {
        return transform_property_name(property_name, alias_generator);
    }

    return property_name;
}

static zend_always_inline zval* get_property_value(zend_class_entry *model_ce, zval *raw_data, zend_string *field_name, zend_property_info *prop_info)
{
    // Check if field exists in rawData
    zval *raw_value = zend_hash_find(Z_ARRVAL_P(raw_data), field_name);

    if (raw_value != NULL) return raw_value;

    // Field doesn't exist in rawData, check for default value
    zval *default_prop = &model_ce->default_properties_table[prop_info->offset];
    if (default_prop != NULL && Z_TYPE_P(default_prop) != IS_UNDEF) {
        return default_prop;
    }

    return NULL;
}

static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, const char *error_message, size_t length)
{
    zval error_msg;
    ZVAL_STRINGL(&error_msg, error_message, length);
    zend_hash_str_add(Z_ARRVAL_P(errors), ZSTR_VAL(field_name), ZSTR_LEN(field_name), &error_msg);
}

static zend_always_inline void add_field_errorf(zval *errors, zend_string *field_name, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char *buffer;
    int len = vspprintf(&buffer, 0, format, args);
    va_end(args);
    
    if (len > 0) {
        zval error_msg;
        ZVAL_STRINGL(&error_msg, buffer, len);
        zend_hash_str_add(Z_ARRVAL_P(errors), ZSTR_VAL(field_name), ZSTR_LEN(field_name), &error_msg);
    }
    
    efree(buffer);
}

static zend_always_inline zval* validate_field_value(zval *value, zend_property_info *prop_info, zval *errors, av_model_configs_properties *properties);

static zend_always_inline bool validate_type(zval *value, zend_type type, zval *errors, zend_string *field_name, bool strict)
{
    zend_uchar expected_type = type & ZEND_TYPE_MASK;
    zend_uchar actual_type = Z_TYPE_P(value);
    
    // If strict mode, types must match exactly
    if (strict) {
        if (actual_type != expected_type) {
            add_field_errorf(errors, field_name, "Expected %s, got %s", 
                            zend_type_name(type), zend_zval_type_name(value));
            return false;
        }
        return true;
    }
    
    // Non-strict mode: attempt coercion
    switch (expected_type) {
        case IS_LONG:
            if (actual_type == IS_STRING) {
                zval coerced;
                ZVAL_LONG(&coerced, zval_get_long(value));
                ZVAL_COPY(value, &coerced);
                return true;
            }
            break;
            
        case IS_DOUBLE:
            if (actual_type == IS_STRING || actual_type == IS_LONG) {
                zval coerced;
                ZVAL_DOUBLE(&coerced, zval_get_double(value));
                ZVAL_COPY(value, &coerced);
                return true;
            }
            break;
            
        case IS_STRING:
            if (actual_type == IS_LONG || actual_type == IS_DOUBLE) {
                zval coerced;
                if (actual_type == IS_LONG) {
                    ZVAL_STRING(&coerced, zend_long_to_str(Z_LVAL_P(value)));
                } else {
                    ZVAL_STRING(&coerced, zend_double_to_str(Z_DVAL_P(value)));
                }
                ZVAL_COPY(value, &coerced);
                return true;
            }
            break;
            
        case IS_BOOL:
            if (actual_type == IS_STRING || actual_type == IS_LONG || actual_type == IS_DOUBLE) {
                zval coerced;
                ZVAL_BOOL(&coerced, zval_is_true(value));
                ZVAL_COPY(value, &coerced);
                return true;
            }
            break;
            
        case IS_ARRAY:
            if (actual_type == IS_OBJECT) {
                // Try to convert object to array
                zval coerced;
                array_init(&coerced);
                zend_object_to_array(value, &coerced, 0);
                ZVAL_COPY(value, &coerced);
                return true;
            }
            break;
    }
    
    // If no coercion was possible, check if types match
    if (actual_type != expected_type) {
        add_field_errorf(errors, field_name, "Expected %s, got %s",
                        zend_type_name(type), zend_zval_type_name(value));
        return false;
    }
    
    return true;
}

static zend_always_inline zval* validate_field_value(zval *value, zend_property_info *prop_info, zval *errors, av_model_configs_properties *properties)
{
    if (value == NULL) {
        return NULL;
    }

    zend_type property_type = prop_info->type;
    zend_string *field_name = zend_string_init(ZSTR_VAL(prop_info->name), ZSTR_LEN(prop_info->name), 0);
    
    // Handle nullable types
    bool is_nullable = (property_type & ZEND_TYPE_NULLABLE) != 0;
    property_type &= ~ZEND_TYPE_NULLABLE;
    
    // If value is NULL and type is nullable, return NULL (valid)
    if (Z_TYPE_P(value) == IS_NULL && is_nullable) {
        zend_string_release(field_name);
        return value;
    }
    
    // If value is NULL but type is not nullable, add error
    if (Z_TYPE_P(value) == IS_NULL) {
        add_field_errorf(errors, field_name, "Expected %s, got null", zend_type_name(property_type));
        zend_string_release(field_name);
        return NULL;
    }
    
    // Validate and coerce the value based on property type
    if (!validate_type(value, property_type, errors, field_name, properties->strict)) {
        zend_string_release(field_name);
        return NULL;
    }
    
    zend_string_release(field_name);
    return value;
}

/* Function implementation for validate */
ZEND_FUNCTION(validate)
{
    zval *raw_data;
    zval *model;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(raw_data)
        Z_PARAM_OBJECT_OF_CLASS(model, AV_BaseModel_ce)
    ZEND_PARSE_PARAMETERS_END();

    zval configs_obj;
    av_model_configs_properties properties;
    av_create_model_configs(&configs_obj, model, &properties);
    if (UNEXPECTED(EG(exception))) {
        zval_ptr_dtor(&configs_obj);
        RETURN_THROWS();
    }

    av_call_before_validation_hook(model, raw_data, &configs_obj);
    if (EG(exception)) {
        zval_ptr_dtor(&configs_obj);
        RETURN_THROWS();
    }

    zval errors;
    array_init(&errors);

    zend_class_entry *model_ce = Z_OBJCE_P(model);

    while (model_ce != NULL && model_ce != AV_BaseModel_ce) {
        zend_string *property_name;
        zend_property_info *prop_info;

        ZEND_HASH_FOREACH_STR_KEY_PTR(&model_ce->properties_info, property_name, prop_info) {
            // Fetches name of the field name for the $rawData
            zend_string *field_name = get_property_name(model_ce, property_name, prop_info, properties.alias_generator);
            bool field_name_owned = (field_name != property_name);

            if (UNEXPECTED(EG(exception))) {
                if (field_name_owned) {
                    zend_string_release(field_name);
                }
                zval_ptr_dtor(&configs_obj);
                zval_ptr_dtor(&errors);
                RETURN_THROWS();
            }

            zval *field_value = get_property_value(model_ce, raw_data, field_name, prop_info);

            if (field_value == NULL) {
                add_field_error(&errors, field_name, "required field", sizeof("required field") - 1);
                if (properties.stop_first_error) {
                    av_throw_validation_exception(&errors);
                    if (field_name_owned) {
                        zend_string_release(field_name);
                    }
                    zval_ptr_dtor(&configs_obj);
                    zval_ptr_dtor(&errors);
                    RETURN_THROWS();
                }
                // Release field_name after adding error if not stopping at first error
                if (field_name_owned) {
                    zend_string_release(field_name);
                    field_name_owned = false;
                }
                continue;
            }

            if (field_name_owned) {
                zend_string_release(field_name);
                field_name_owned = false;
            }

            zval *valid_value = validate_field_value(field_value, prop_info, &errors, &properties);
            if (UNEXPECTED(EG(exception))) {
                zval_ptr_dtor(&configs_obj);
                zval_ptr_dtor(&errors);
                RETURN_THROWS();
            }

            if (valid_value == NULL) {
                if (!properties.stop_first_error) {
                    continue;
                }

                av_throw_validation_exception(&errors);
                zval_ptr_dtor(&configs_obj);
                zval_ptr_dtor(&errors);
                RETURN_THROWS();
            }

            zend_update_property(model_ce, Z_OBJ_P(model), ZSTR_VAL(property_name), ZSTR_LEN(property_name), valid_value);
        } ZEND_HASH_FOREACH_END();

        model_ce = model_ce->parent;
    }

    if (!properties.stop_first_error && zend_hash_num_elements(Z_ARRVAL_P(&errors))) {
        av_throw_validation_exception(&errors);
        zval_ptr_dtor(&configs_obj);
        zval_ptr_dtor(&errors);
        RETURN_THROWS();
    }

    av_call_after_validation_hook(model, raw_data, &configs_obj);
    if (EG(exception)) {
        zval_ptr_dtor(&configs_obj);
        zval_ptr_dtor(&errors);
        RETURN_THROWS();
    }

    zval_ptr_dtor(&configs_obj);
    zval_ptr_dtor(&errors);
    RETURN_COPY(model);
}
