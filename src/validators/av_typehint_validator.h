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
#include "../av_compiled_model.h"
#include "../av_exception.h"
#include "av_array_typehint_validator.h"

#define AV_EPSILON 1e-15

/*
 * Resolves a named type arm to its class entry: the ce cache when
 * populated, otherwise the declaring-class scoped self/parent lookup.
 * Unresolvable names return NULL.
 */
zend_class_entry *av_get_ce_from_type(zend_property_info *info, const zend_type *type);

bool av_handle_class_by_ce(av_field *field, av_property_info *prop_info, zend_class_entry *ce, av_model_configs_properties *properties, zval *errors);
bool av_coerce_bool(av_field *field);

void av_init_typehint_validator(void);
void av_shutdown_typehint_validator(void);

/**
 * Validates that a value matches the property's compiled type arms.
 *
 * For union types, tries each arm in the union.
 *
 * @param field         The field related structure
 * @param cf            The compiled field slot
 * @param prop_info     Property type information (error rendering)
 * @param properties    Model configuration properties (for recursive validation)
 * @param errors        Error collection array
 * @return              true if validation succeeds, false otherwise
 */
bool av_validate_compiled_type_hint(av_field *field, av_compiled_field *cf, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors);

#endif /* AV_TYPEHINT_VALIDATION_H */
