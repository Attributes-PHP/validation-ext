#include "validate_function.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "base_model.h"
#include "model_configs.h"
#include "exception.h"
#include "Zend/zend_interfaces.h"
#include "php.h"

/* Function implementation for validate */
ZEND_FUNCTION(validate)
{
    zval *raw_data;
    zval *model;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(raw_data)
        Z_PARAM_OBJECT_OF_CLASS(model, Attributes_Validation_BaseModel_ce)
    ZEND_PARSE_PARAMETERS_END();

    zval configs_obj;
    attributes_validation_model_configs_properties properties;
    attributes_validation_create_model_configs(&configs_obj, model, &properties);
    if (EG(exception)) {
        zval_ptr_dtor(&configs_obj);
        RETURN_THROWS();
    }

    attributes_validation_call_before_validation_hook(model, raw_data, &configs_obj);
    if (EG(exception)) {
        zval_ptr_dtor(&configs_obj);
        RETURN_THROWS();
    }

    zval errors;
    array_init(&errors);

    zend_class_entry *model_ce = Z_OBJCE_P(model);

    while (model_ce != NULL && model_ce != Attributes_Validation_BaseModel_ce) {
        zend_string *property_name;
        zend_property_info *prop_info;

        ZEND_HASH_FOREACH_STR_KEY_PTR(&model_ce->properties_info, property_name, prop_info) {
            // TODO: 6. For each property, resolve the actual field name in rawData
            // - Check for #[Alias] attribute on the property
            // - If alias exists, use it as the key to look up in rawData
            // - Otherwise, resort to the aliasGenerator property from ModelConfigs if exists
            // - If none of them exist, use the property name to look up in rawData
            // - If field doesn't exist in rawData:
            //   * If property has default value, use it
            //   * Otherwise, add "[FIELD_NAME]" => "field is required" to errors

            // For each property, we have:
            // a. Property name: property_name (zend_string *)
            // b. Type hint: prop_info->type (zend_type)
            // c. Attributes: prop_info->attributes (HashTable *)
            // d. Default value: from model_ce->default_properties_table[prop_info->offset]

            // TODO: Process this property (name, type, attributes, default value)
            // This will be used in subsequent steps (6-9)
        } ZEND_HASH_FOREACH_END();

        model_ce = model_ce->parent;
    }

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

    // TODO: 9. After all properties are validated, check for errors
    // - If errors collection is not empty:
    //   * Create ValidationException with all aggregated errors
    //   * Format: {"field_path": ["error message 1", "error message 2", ...]}
    //   * Provide getAllErrors() method to retrieve the full error array
    //   * Throw the exception

    attributes_validation_call_after_validation_hook(model, raw_data, &configs_obj);
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
