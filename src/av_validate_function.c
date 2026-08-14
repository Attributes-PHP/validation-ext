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

static zend_always_inline zval* get_property_value(zend_class_entry *model_ce, zval *raw_data, zend_string *field_name)
{
    zval *raw_value = zend_hash_find(Z_ARRVAL_P(raw_data), field_name);

    if (raw_value != NULL && Z_TYPE_P(raw_value) != IS_UNDEF) return raw_value;

    return NULL;
}



static zend_always_inline bool has_property_default_value(zend_class_entry *model_ce, zend_property_info *prop_info)
{
    if (!model_ce->default_properties_table) return false;

    const uint32_t index = OBJ_PROP_TO_NUM(prop_info->offset);
    if (index >= model_ce->default_properties_count) return false;

    const zval *default_value = &model_ce->default_properties_table[index];
    return Z_TYPE_P(default_value) != IS_UNDEF;
}

static inline bool validate_field_value(zend_string *field_name, zval *value, zend_property_info *prop_info, av_model_configs_properties *properties, zval *errors, zend_string *parent_path)
{
    if (!av_validate_type_hint(field_name, value, prop_info, properties, errors, parent_path)) {
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


bool av_validate_model_internal(zval *raw_data, zval *model, zend_class_entry *model_ce, av_model_configs_properties *properties, zval *errors, zend_string *parent_path)
{
    while (model_ce != NULL && model_ce != AV_BaseModel_ce) {
        zend_string *property_name = NULL;
        zend_property_info *prop_info;

        ZEND_HASH_FOREACH_STR_KEY_PTR(&model_ce->properties_info, property_name, prop_info) {
            if (prop_info->flags & ZEND_ACC_STATIC) continue;
            if (prop_info->flags & (ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE)) continue;

            zend_string *field_name = get_property_name(model_ce, property_name, prop_info, properties->alias_generator);
            bool is_to_release_field_name = (field_name != property_name && field_name != NULL);

            if (UNEXPECTED(EG(exception))) {
                if (is_to_release_field_name) zend_string_release(field_name);
                return false;
            }

            zval *field_value = get_property_value(model_ce, raw_data, field_name);

            if (field_value == NULL) {
                const bool has_default_value = has_property_default_value(model_ce, prop_info);
                if (has_default_value) {
                    if (is_to_release_field_name) zend_string_release(field_name);
                    continue;
                }

                zend_string *msg = zend_string_concat3(
                    "The ", sizeof("The ") - 1,
                    ZSTR_VAL(field_name), ZSTR_LEN(field_name),
                    " field is required.", sizeof(" field is required.") - 1
                );
                av_add_field_error_with_prefix(errors, parent_path, field_name, ZSTR_VAL(msg), ZSTR_LEN(msg));
                zend_string_release(msg);
                if (is_to_release_field_name) zend_string_release(field_name);
                if (properties->stop_first_error) {
                    return false;
                }
                continue;
            }

            zval valid_value;
            ZVAL_COPY(&valid_value, field_value);

            zend_string *nested_path = parent_path ?
                zend_string_concat3(ZSTR_VAL(parent_path), ZSTR_LEN(parent_path), ".", 1, ZSTR_VAL(field_name), ZSTR_LEN(field_name)) :
                NULL;

            const bool is_valid = validate_field_value(field_name, &valid_value, prop_info, properties, errors, nested_path);

            if (nested_path) zend_string_release(nested_path);
            if (is_to_release_field_name) zend_string_release(field_name);

            if (!is_valid) {
                zval_ptr_dtor(&valid_value);
                if (properties->stop_first_error) {
                    return false;
                }
                continue;
            }

            zend_update_property(model_ce, Z_OBJ_P(model), ZSTR_VAL(property_name), ZSTR_LEN(property_name), &valid_value);
            zval_ptr_dtor(&valid_value);
        } ZEND_HASH_FOREACH_END();

        model_ce = model_ce->parent;
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

    if (!av_validate_model_internal(raw_data, model, model_ce, &properties, &errors, NULL)) {
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
