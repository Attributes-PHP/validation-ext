#ifndef AV_COMPILED_MODEL_H
#define AV_COMPILED_MODEL_H

#include "php.h"
#include "av_model_configs.h"
#include "helpers/av_structs.h"

/*
 * Compiled form of a BaseModel class, built once per class per alias
 * generator on first validation and reused for every subsequent
 * validate() call of the request.
 *
 * The plan flattens the per-call interpretation the validator used to
 * perform: the properties_info walk over the class hierarchy, the flag
 * checks, the field-name resolution and the zend_type interpretation.
 * At steady state, validating a model means one cache lookup by class
 * entry plus a flat iteration over the field slots.
 */

/* Forward declaration: the array attribute spec, owned by the spec cache */
typedef struct av_spec av_spec;

typedef enum {
    AV_ARM_BASIC = 1, // arm->mask holds the pure type mask
    AV_ARM_CLASS,     // arm->ce / arm->origin hold the class arm
    AV_ARM_INTERSECTION, // arm->origin holds the intersection type
} av_compiled_arm_kind;

typedef struct {
    uint8_t kind;
    uint32_t mask;                 // AV_ARM_BASIC
    zend_class_entry *ce;           // AV_ARM_CLASS: resolved CE, NULL while unresolved
    const zend_type *origin;       // AV_ARM_CLASS / AV_ARM_INTERSECTION: the original type arm
} av_compiled_arm;

typedef enum {
    AV_ARRAY_UNPROBED, // array attribute checks not performed yet
    AV_ARRAY_ABSENT,   // property declares no array type-hint attributes
    AV_ARRAY_SPEC,     // array_spec borrowed from the spec cache
} av_array_state;

typedef struct {
    zend_property_info *prop;
    zend_class_entry *declaring_ce;

    zend_string *name;      // resolved field name, owned; NULL until compiled
    uint32_t contains_code; // bit per type code accepted by the native hint
    av_compiled_arm *arms;  // flattened zend_type arms, NULL when untyped
    uint32_t arms_count;
    bool untyped;
    bool is_intersection_group; // pure intersection hint: every class arm must match
    bool has_default;

    av_array_state array_state;
    av_spec *array_spec; // borrowed, owned by the array spec cache
} av_compiled_field;

typedef struct {
    av_compiled_field *fields;
    uint32_t count;
    char generator;  // alias generator the field names are compiled under
    bool is_root;    // compiled for a top-level validate() call
    av_model_configs_properties configs; // valid when is_root
    bool hooks_overridden;
} av_compiled_model;

/*
 * Plans per class entry, one per alias generator the class has been
 * validated under. Index 0 is the "no generator" slot; a root compile
 * stores its configs and reuses the slot of its own generator.
 */
typedef struct {
    av_compiled_model *by_generator[5];
} av_class_plans;

/*
 * Returns the root plan for the model's class, compiling the skeleton
 * and resolving ModelConfigs on first use. Returns NULL with a pending
 * exception when the configs cannot be parsed.
 */
av_compiled_model *av_get_root_compiled_model(zval *model);

/*
 * Validates raw_data against the model's compiled plan. properties are
 * the configs in effect (the root model's, propagated into nested
 * validation), matching the previous semantics.
 */
bool av_compiled_validate_model(zval *raw_data, zval *model, av_model_configs_properties *properties, zval *errors, zend_string *parent_path);

void av_clear_compiled_model_cache(void);
void av_clear_field_name_cache(void);

#endif /* AV_COMPILED_MODEL_H */
