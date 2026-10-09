/**
 * Array type-hint validation via attributes
 *
 * Validates the contents of array properties against the type attributes
 * declared on them:
 *
 *   #[Sequence(T)]                          every element matches T
 *   #[Union(a, b, ...)]               every element matches one arm, in order
 *   #[Intersection(A, B, ...)]       every element is an instance of all classes
 *   #[Dict(key: K, of: V)]            keys match K and values match V
 *
 * Arms accept the Type constants (bool, int, float, string, sequence, dict,
 * object) or Type enum cases, class name strings (User::class, resolved
 * against the declaring class namespace) and nested attribute instances
 * (new Sequence(...), new Union(...), ...) which describe nested arrays. A class
 * name string equal to one of the attribute class names (Dict::class)
 * references the same-named attribute declared on the property.
 *
 * Type constants may also be combined with the bitwise OR operator
 * (#[Union(bool | int | string)]): the validator expands the bits into a
 * type mask. The bitmask form loses the declaration order of the plain
 * argument list, so coercions apply in a fixed bool, int, float, string
 * order instead.
 *
 * Sequence and Dict value positions (of:) flatten Union and Intersection
 * arguments into the element type list; attribute objects there stay
 * nested array shapes.
 */

#ifndef AV_ARRAY_TYPEHINT_VALIDATOR_H
#define AV_ARRAY_TYPEHINT_VALIDATOR_H

#include "php.h"
#include "../av_model_configs.h"
#include "../av_validate_function.h"
#include "../av_compiled_model.h"
#include "../helpers/av_structs.h"

/**
 * Validates that an array value matches the array type-hint attributes
 * (Sequence, Union, Intersection, Dict) declared on the property.
 *
 * Attribute probes and spec cache lookups run once per compiled field
 * slot; the spec is then borrowed for every later validation of the
 * property. Returns true when the property declares none of these
 * attributes (a plain `array` keeps behaving as before) or when every
 * key and element validated.
 */
bool av_validate_array_typehint_cached(av_field *field, av_compiled_field *cf, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors);
void av_clear_array_spec_cache(void);

#endif /* AV_ARRAY_TYPEHINT_VALIDATOR_H */
