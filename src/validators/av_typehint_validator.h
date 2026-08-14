/**
 * Type hint validation with custom coercion support
 *
 * This header declares the type hint validation function that handles
 * property type validation with support for:
 * - Simple types with custom coercion (e.g., bool)
 * - Union types (PHP 8.2+) like bool|int
 * - Strict and loose validation modes
 */

#ifndef AV_TYPEHINT_VALIDATION_H
#define AV_TYPEHINT_VALIDATION_H

#include "php.h"
#include "../av_model_configs.h"
#include "../av_validate_function.h"
#include "../av_exception.h"

#define AV_EPSILON 1e-15

static zend_class_entry *resolve_single_class_type(zend_string *name, zend_class_entry *self_ce);
static zend_always_inline zend_class_entry *get_ce_from_type(zend_property_info *info, zend_type *type);
static bool handle_intersection(zend_string *field_name, zval *value, zend_property_info *prop_info, zend_type *value_type);
static bool handle_class(zend_string *field_name, zval *value, zend_property_info *prop_info, zend_type *value_type, av_model_configs_properties *properties, zval *errors, zend_string *parent_path);
static bool coerce_bool(zend_string *field_name, zval *value);
static zend_string* generate_type_name(zend_type *type);

/* Enum detection */
static bool av_type_is_enum(zend_type *type);

/* Error message generation */
static zend_string* generate_error_message(zend_type property_type, zend_string *field_name, zend_string *parent_path);
static bool av_vowel_sound(char c);

/**
 * Validates that a value matches the property's type hint.
 *
 * For union types, tries each type in the union.
 *
 * @param field_name    The name of the field being validated
 * @param value         The value to validate (may be modified in-place)
 * @param prop_info     Property type information
 * @param properties    Model configuration properties (for recursive validation)
 * @param errors        Error collection array
 * @param parent_path   Parent path for nested error messages
 * @return              true if validation succeeds, false otherwise
 */
bool av_validate_type_hint(zend_string *field_name, zval *value, zend_property_info *prop_info, av_model_configs_properties *properties, zval *errors, zend_string *parent_path);

#endif /* AV_TYPEHINT_VALIDATION_H */
