#include "src/validate_function.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "src/base_model.h"
#include "src/options.h"
#include "src/exception.h"
#include "Zend/zend_interfaces.h"

/* Function implementation for validate */
PHP_FUNCTION(validate)
{
    zval *rawData;
    zval *model;
    zend_bool strict = 0;
    zend_bool stopAtFirstError = 0;

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_ARRAY(rawData)
        Z_PARAM_OBJECT_OF_CLASS(model, validation_ext_BaseModel_ce)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(strict)
        Z_PARAM_BOOL(stopAtFirstError)
    ZEND_PARSE_PARAMETERS_END();

    // Create validation options with: 1) strict and 2) stopAtFirstError
    zval options_obj;
    create_options(&options_obj, strict, stopAtFirstError);
    if (EG(exception)) {
        return;
    }

    call_before_validation_hook(&rawData, model, &options_obj);
    if (EG(exception)) {
        return;
    }

    // TODO: 3. Get model-level configurations attributes
    // - Check for #[AliasGenerator] attribute on the model class
    // - Apply alias transformation to all property names when looking them up in rawData
    // - Example: snake_case to camelCase conversion

    // TODO: 4. Create an errors collection array to store validation errors
    // - Initialize empty array to aggregate all errors
    // - Each error should be keyed by the property path (e.g., "team_name", "users.0.email")

    // TODO: 5. Get all properties of the model via reflection
    // - Use zend_get_class_entry and zend_get_properties to inspect model properties
    // - For each property, get:
    //   a. Property name
    //   b. Type hint (if any)
    //   c. All attributes applied to the property
    //   d. Default value (if any)

    // TODO: 6. For each property, resolve the actual field name in rawData
    // - Check for #[Alias] attribute on the property
    // - If alias exists, use it as the key to look up in rawData
    // - If model has AliasGenerator, apply transformation to property name
    // - If field doesn't exist in rawData:
    //   * If property has default value, use it
    //   * If property is nullable, skip it
    //   * Otherwise, add "field is required" error

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

    call_after_validation_hook(&rawData, model, &options_obj);
    if (EG(exception)) {
        return;
    }

    // TODO: 11. Populate the model instance with validated data
    // - For each validated property, set the value on the model object
    // - Handle public properties directly
    // - Handle private/protected properties via reflection or property setting methods

    // TODO: 12. Return the populated model instance

    // For now, just return a clone of the model object
    // This is a stub implementation
    zval clone;
    ZVAL_OBJ(&clone, Z_OBJ_P(model));
    Z_ADDREF(clone);
    RETURN_ZVAL(&clone, 0, 1);
}

/**
* Instantiates an Options class with all required properties for validation
**/
static void create_options(zval *options, zend_bool strict, zend_bool stop_at_first_error)
{
    zval strict_zval;
    zval stop_zval;

    ZVAL_BOOL(&strict_zval, strict);
    ZVAL_BOOL(&stop_zval, stop_at_first_error);

    object_init_ex(options, validation_ext_Options_ce);
    zend_call_method_with_2_params(Z_OBJ_P(options), validation_ext_Options_ce, NULL, "__construct", NULL, &strict_zval, &stop_zval);
}

/**
 * Calls beforeValidation hook of BaseModel e.g. $rawData = $model->beforeValidation($rawData, $options)
 * This hook can modify the rawData before validation begins
 **/
static void call_before_validation_hook(zval **raw_data, zval *model, zval *options)
{
    zval *before_validation_result = emalloc(sizeof(zval));
    ZVAL_UNDEF(before_validation_result);
    zend_class_entry *model_ce = Z_OBJCE_P(model);

    zend_call_method_with_2_params(Z_OBJ_P(model), model_ce, NULL, "beforeValidation", before_validation_result, *raw_data, options);
    if (Z_TYPE_P(before_validation_result) != IS_ARRAY) {
        zval_ptr_dtor(before_validation_result);
        return;
    }

    zval_ptr_dtor(*raw_data);
    *raw_data = before_validation_result;
}

/** Calls afterValidation hook of BaseModel e.g. $model->afterValidation($rawData, $options) **/
static void call_after_validation_hook(zval **raw_data, zval *model, zval *options)
{
    zend_call_method_with_2_params(Z_OBJ_P(model), Z_OBJCE_P(model), NULL, "afterValidation", NULL, *raw_data, options);
}
