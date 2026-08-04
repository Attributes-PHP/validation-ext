#include "validate_function.h"
#include "exception.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_attributes.h"
#include "base_model.h"
#include "model_configs.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_types.h"
#include "helpers/av_string.h"
#include <stddef.h>


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
    zend_string *field_name = NULL;

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

static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, char *error_message, size_t length)
{
    zval error_msg;
    ZVAL_STRING(&error_msg, error_message);
    zend_hash_str_add(Z_ARRVAL_P(errors), ZSTR_VAL(field_name), ZSTR_LEN(field_name), &error_msg);
    zval_ptr_dtor(&error_msg);
}

static inline zval *validate_field_value(zval *value, zend_property_info *prop_info, zval *errors)
{
    // TODO: 7. For each property, collect and sort validation rules
    // - Collect all attributes that are validation rules
    // - Type hint has highest priority (applied first)
    // - Other rules applied from bottom to top (reverse order of declaration)
    // - Supported rule types:
    //   * Type hints (int, string, DateTime, etc.)
    //   * #[ArrayOf(type1, type2, ...)] for array validation
    //   * #[Length(min, max)] for string length
    //   * Custom rules implementing Rules\Custom interface

    // TODO: 8. For each property value, perform validation:
    // - Get raw value from rawData using resolved field name
    // - Apply SensitiveParameter: mask value in errors if attribute is present
    // - Apply type hint validation first:
    //   * If strict mode, value must already be of the correct type
    //   * Otherwise, attempt to cast/coerce the value
    //   * Handle union types (e.g., float|int)
    //   * Handle nullable types (e.g., ?string)
    // - For ArrayOf:
    //   * Validate each element of the array against the specified types
    //   * Nested arrays create dot-notation paths (e.g., "users.0.email")
    // - Apply other rules in order
    // - For nested objects:
    //   * Recursively validate nested Base model instances
    //   * Build nested error paths
    // - If stopAtFirstError is true, throw ValidationException immediately on first error
    // - Otherwise, add error to errors collection and continue
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

            bool is_to_release_field_name = (field_name != property_name);

            if (UNEXPECTED(EG(exception))) {
                if (is_to_release_field_name) zend_string_release(field_name);
                zval_ptr_dtor(&configs_obj);
                zval_ptr_dtor(&errors);
                RETURN_THROWS();
            }

            zval *field_value = get_property_value(model_ce, raw_data, field_name, prop_info);
            if (field_value == NULL) {
                add_field_error(&errors, field_name, "required field", sizeof("required field") - 1);
                if (properties.stop_first_error) {
                    av_throw_validation_exception(&errors);

                    if (is_to_release_field_name) zend_string_release(field_name);
                    zval_ptr_dtor(&configs_obj);
                    zval_ptr_dtor(&errors);
                    RETURN_THROWS();
                }
                // field_name will be released at the end of the loop iteration
            }

            zval *valid_value = validate_field_value(field_value, prop_info, &errors);
            if (UNEXPECTED(EG(exception))) {
                if (is_to_release_field_name) zend_string_release(field_name);
                zval_ptr_dtor(&configs_obj);
                zval_ptr_dtor(&errors);
                RETURN_THROWS();
            }

            if (valid_value == NULL) {
                if (!properties.stop_first_error) {
                    if (is_to_release_field_name) zend_string_release(field_name);
                    continue;
                }

                av_throw_validation_exception(&errors);

                if (is_to_release_field_name) zend_string_release(field_name);
                zval_ptr_dtor(&configs_obj);
                RETURN_THROWS();
            }

            zend_update_property(model_ce, Z_OBJ_P(model), ZSTR_VAL(property_name), ZSTR_LEN(property_name), valid_value);

            if (is_to_release_field_name) zend_string_release(field_name);
        } ZEND_HASH_FOREACH_END();

        model_ce = model_ce->parent;
    }

    if (!properties.stop_first_error && zend_hash_num_elements(Z_ARRVAL_P(&errors))) {
        av_throw_validation_exception(&errors);

        zval_ptr_dtor(&configs_obj);
        RETURN_THROWS();
    }

    av_call_after_validation_hook(model, raw_data, &configs_obj);
    if (EG(exception)) {
        zval_ptr_dtor(&configs_obj);
        zval_ptr_dtor(&errors);
        RETURN_THROWS();
    }

    // TODO: 11. Populate the model instance with validated data
    // - For each validated property, set the value on the model object
    // - Handle public properties directly
    // - Handle private/protected properties via reflection or property setting methods
    zval_ptr_dtor(&configs_obj);
    zval_ptr_dtor(&errors);
    RETURN_COPY(model);
}
