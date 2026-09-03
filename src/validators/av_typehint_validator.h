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
static bool handle_intersection(av_field *field, av_property_info *prop_info, zend_type *value_type);
static bool handle_class(av_field *field, av_property_info *prop_info, zend_type *value_type, av_model_configs_properties *properties, zval *errors);
static bool coerce_bool(av_field *field);
static bool coerce_datetime(zval *value, zend_class_entry *target_ce, av_model_configs_properties *properties);
static bool is_datetime_class(zend_class_entry *ce);

void av_init_typehint_validator(void);
/**
 * Validates that a value matches the property's type hint.
 *
 * For union types, tries each type in the union.
 *
 * @param field         The field related structure
 * @param prop_info     Property type information
 * @param properties    Model configuration properties (for recursive validation)
 * @param errors        Error collection array
 * @return              true if validation succeeds, false otherwise
 */
bool av_validate_type_hint(av_field *field, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors);

#endif /* AV_TYPEHINT_VALIDATION_H */
