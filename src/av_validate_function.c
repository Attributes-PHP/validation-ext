#include "av_validate_function.h"
#include "av_exception.h"
#include "validators/av_typehint_validator.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_attributes.h"
#include "av_base_model.h"
#include "av_model_configs.h"
#include "av_exception.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_types.h"
#include "helpers/av_string.h"
#include <stddef.h>
#include "Zend/zend_hash.h"
#include "helpers/av_error_messages.h"
#include "helpers/av_structs.h"
#include "php.h"


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
static zend_always_inline zend_string* get_property_name(av_property_info *property_info, zend_string *property_name, char alias_generator)
{
    zend_string *field_name = NULL;

    // Check for #[Alias] attribute on the property
    if (property_info->property->attributes != NULL) {
        zend_attribute *alias_attr = zend_get_attribute_str(
            property_info->property->attributes,
            "attributes\\validation\\fields\\alias",
            sizeof("attributes\\validation\\fields\\alias") - 1
        );
        if (alias_attr != NULL && alias_attr->argc > 0) {
            zval attr_value;
            if (zend_get_attribute_value(&attr_value, alias_attr, 0, property_info->model_ce) == SUCCESS) {
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

static zend_always_inline zval* get_property_value(zend_class_entry *model_ce, zval *raw_data, zend_string *field_name)
{
    zval *raw_value = zend_hash_find(Z_ARRVAL_P(raw_data), field_name);

    if (raw_value != NULL && Z_TYPE_P(raw_value) != IS_UNDEF) return raw_value;

    return NULL;
}



static zend_always_inline bool has_property_default_value(av_property_info *property_info)
{
    if (!property_info->model_ce->default_properties_table) return false;

    const uint32_t index = OBJ_PROP_TO_NUM(property_info->property->offset);
    if (index >= property_info->model_ce->default_properties_count) return false;

    const zval *default_value = &property_info->model_ce->default_properties_table[index];
    return Z_TYPE_P(default_value) != IS_UNDEF;
}

static inline bool validate_field_value(av_field *field, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors)
{
    if (!av_validate_type_hint(field, prop_info, properties, errors)) {
        return false;
    }
    return true;
}

// TODO: For each property, collect and sort validation rules
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
    //   * Otherwise, add error to errors collection and continue


bool av_validate_model_internal(zval *raw_data, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors, zend_string *parent_path)
{
    while (prop_info->model_ce != NULL && prop_info->model_ce != AV_BaseModel_ce) {
        zend_string *property_name = NULL;
        av_field field = {
            .parent = parent_path,
        };

        ZEND_HASH_FOREACH_STR_KEY_PTR(&prop_info->model_ce->properties_info, property_name, prop_info->property) {
            if (prop_info->property->flags & ZEND_ACC_STATIC) continue;
            if (prop_info->property->flags & (ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE)) continue;

            field.name = get_property_name(prop_info, property_name, properties->alias_generator);
            bool is_to_release_field_name = (field.name != property_name && field.name != NULL);

            if (UNEXPECTED(EG(exception))) {
                if (is_to_release_field_name) zend_string_release(field.name);
                return false;
            }

            field.value = get_property_value(prop_info->model_ce, raw_data, field.name);

            if (field.value == NULL) {
                const bool has_default_value = has_property_default_value(prop_info);
                if (has_default_value) {
                    if (is_to_release_field_name) zend_string_release(field.name);
                    continue;
                }

                av_add_field_error_with_prefix(AV_ERROR_REQUIRED, &field, prop_info, errors);
                if (is_to_release_field_name) zend_string_release(field.name);
                if (properties->stop_first_error) {
                    return false;
                }
                continue;
            }

            field.parent = parent_path ?
                zend_string_concat3(ZSTR_VAL(parent_path), ZSTR_LEN(parent_path), ".", 1, ZSTR_VAL(field.name), ZSTR_LEN(field.name)) :
                NULL;

            const bool is_valid = validate_field_value(&field, prop_info, properties, errors);

            if (field.parent) {
                zend_string_release(field.parent);
                field.parent = parent_path;
            }
            if (is_to_release_field_name) zend_string_release(field.name);

            if (!is_valid) {
                if (properties->stop_first_error) return false;
                continue;
            }

            zend_update_property(prop_info->model_ce, Z_OBJ_P(prop_info->model), ZSTR_VAL(property_name), ZSTR_LEN(property_name), field.value);
        } ZEND_HASH_FOREACH_END();

        prop_info->model_ce = prop_info->model_ce->parent;
    }

    return zend_hash_num_elements(Z_ARRVAL_P(errors)) == 0;
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
        AV_ZVAL_DTOR_RETURN_THROWS(&configs_obj);
    }

    av_call_before_validation_hook(model, raw_data, &configs_obj);
    if (EG(exception)) {
        AV_ZVAL_DTOR_RETURN_THROWS(&configs_obj);
    }

    zval errors;
    array_init(&errors);

    zend_class_entry *model_ce = Z_OBJCE_P(model);

    av_property_info property_info = {
        .model = model,
        .model_ce = model_ce,
    };
    if (!av_validate_model_internal(raw_data, &property_info, &properties, &errors, NULL)) {
        ZEND_ASSERT(zend_hash_num_elements(Z_ARRVAL_P(&errors)) > 0);

        av_throw_validation_exception(&errors);
        AV_ZVAL_DTOR_RETURN_THROWS(&configs_obj, &errors);
    }

    av_call_after_validation_hook(model, raw_data, &configs_obj);
    zval_ptr_dtor(&configs_obj);
    zval_ptr_dtor(&errors);

    if (EG(exception)) {
        RETURN_THROWS();
    }

    RETURN_COPY(model);
}
