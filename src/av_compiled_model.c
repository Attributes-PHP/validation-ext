#include "av_compiled_model.h"
#include "av_globals.h"
#include "av_base_model.h"
#include "av_exception.h"
#include "av_validate_function.h"
#include "helpers/av_error_messages.h"
#include "helpers/av_string.h"
#include "validators/av_array_typehint_validator.h"
#include "validators/av_typehint_validator.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_hash.h"
#include <stddef.h>
#include <string.h>

/* =========================================================================
 * Field name resolution (moved from av_validate_function.c, compile-time only)
 * ========================================================================= */

/**
 * Transforms a property name based on the alias generator type
 */
static zend_always_inline zend_string *transform_property_name(zend_string *property_name, char alias_generator)
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
 * Resolves the field name of a property:
 *  1) If the Alias attribute is set uses that value
 *  2) If aliasGenerator is configured, transforms the property name
 *  3) Otherwise uses the property name as-is
 */
static zend_string *resolve_field_name(av_property_info *property_info, zend_string *property_name, char alias_generator)
{
    zend_string *field_name = NULL;

    // Check for #[Alias] attribute on the property
    if (property_info->property->attributes != NULL) {
        zend_attribute *alias_attr = zend_get_attribute_str(property_info->property->attributes, "attributes\\validation\\fields\\alias", sizeof("attributes\\validation\\fields\\alias") - 1);
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

/*
 * Field name cache, keyed by the property info and stored in the module
 * globals (see av_globals.h): the Alias attribute and the alias
 * generator never change within a request, so resolutions run once per
 * property. The cache owns one reference; hits hand out an additional
 * one. Erroring resolutions stay uncached so their errors keep firing.
 */
static void av_field_name_cache_dtor(zval *entry)
{
    zend_string_release(Z_PTR_P(entry));
}

void av_clear_field_name_cache(void)
{
    if (AV_G(av_field_name_cache) != NULL) {
        zend_hash_destroy(AV_G(av_field_name_cache));
        efree(AV_G(av_field_name_cache));
        AV_G(av_field_name_cache) = NULL;
    }
}

static zend_string *get_property_name(av_property_info *property_info, zend_string *property_name, char alias_generator)
{
    // Properties without attributes and without a generator resolve to
    // the property name directly: nothing to cache
    if (property_info->property->attributes == NULL && alias_generator == false) {
        return property_name;
    }

    if (AV_G(av_field_name_cache) != NULL) {
        zend_string *cached = zend_hash_index_find_ptr(AV_G(av_field_name_cache), (zend_ulong)(uintptr_t)property_info->property);
        if (cached != NULL) {
            return zend_string_copy(cached);
        }
    }

    zend_string *field_name = resolve_field_name(property_info, property_name, alias_generator);

    // The error path (pending exception) must keep firing
    if (field_name == NULL || UNEXPECTED(EG(exception) != NULL)) {
        return field_name;
    }

    // A property with attributes but no Alias and no generator resolves
    // to the property name: cache the miss too, so its attribute lookup
    // is not repeated on every validation
    if (field_name == property_name) {
        field_name = zend_string_copy(property_name);
    }

    if (AV_G(av_field_name_cache) == NULL) {
        AV_G(av_field_name_cache) = emalloc(sizeof(HashTable));
        zend_hash_init(AV_G(av_field_name_cache), 8, NULL, av_field_name_cache_dtor, 0);
    }

    zend_hash_index_add_ptr(AV_G(av_field_name_cache), (zend_ulong)(uintptr_t)property_info->property, field_name);

    return zend_string_copy(field_name);
}

static zend_always_inline bool has_property_default_value(av_property_info *property_info)
{
    // OBJ_PROP_TO_NUM() is only meaningful for declared (non-static) properties
    ZEND_ASSERT(!(property_info->property->flags & ZEND_ACC_STATIC));

    if (!property_info->model_ce->default_properties_table)
        return false;

    const uint32_t index = OBJ_PROP_TO_NUM(property_info->property->offset);
    if (index >= property_info->model_ce->default_properties_count)
        return false;

    const zval *default_value = &property_info->model_ce->default_properties_table[index];
    return Z_TYPE_P(default_value) != IS_UNDEF;
}

/* =========================================================================
 * Plan cache and compilation
 * ========================================================================= */

/* Class entry -> av_class_plans*, stored in the module globals (see av_globals.h) */

static uint32_t generator_index(char alias_generator)
{
    switch (alias_generator) {
        case AV_PASCAL_CASE:
            return 1;
        case AV_CAMEL_CASE:
            return 2;
        case AV_SNAKE_CASE:
            return 3;
        case AV_KEBAB_CASE:
            return 4;
        default:
            return 0;
    }
}

static void free_compiled_field(av_compiled_field *cf)
{
    if (cf->name != NULL) {
        zend_string_release(cf->name);
    }
    if (cf->arms != NULL) {
        efree(cf->arms);
    }
}

static void free_compiled_model(av_compiled_model *plan)
{
    for (uint32_t i = 0; i < plan->count; i++) {
        free_compiled_field(&plan->fields[i]);
    }
    if (plan->fields != NULL) {
        efree(plan->fields);
    }
    efree(plan);
}

static void class_plans_dtor(zval *entry)
{
    av_class_plans *plans = Z_PTR_P(entry);

    for (uint32_t i = 0; i < 5; i++) {
        if (plans->by_generator[i] != NULL) {
            free_compiled_model(plans->by_generator[i]);
        }
    }
    efree(plans);
}

void av_clear_compiled_model_cache(void)
{
    if (AV_G(av_plans_cache) != NULL) {
        zend_hash_destroy(AV_G(av_plans_cache));
        efree(AV_G(av_plans_cache));
        AV_G(av_plans_cache) = NULL;
    }
}

static av_class_plans *get_class_plans(zend_class_entry *ce)
{
    av_class_plans *plans = NULL;

    if (AV_G(av_plans_cache) != NULL) {
        plans = zend_hash_index_find_ptr(AV_G(av_plans_cache), (zend_ulong)(uintptr_t)ce);
        if (plans != NULL) {
            return plans;
        }
    }

    plans = emalloc(sizeof(av_class_plans));
    memset(plans, 0, sizeof(av_class_plans));

    if (AV_G(av_plans_cache) == NULL) {
        AV_G(av_plans_cache) = emalloc(sizeof(HashTable));
        zend_hash_init(AV_G(av_plans_cache), 8, NULL, class_plans_dtor, 0);
    }

    return zend_hash_index_add_ptr(AV_G(av_plans_cache), (zend_ulong)(uintptr_t)ce, plans);
}

/*
 * Builds the plan skeleton: one slot per public non-static property of
 * the whole hierarchy, in the same order the interpreter walked it
 * (child class first). Slot contents are compiled lazily on first use.
 */
static av_compiled_model *compile_plan_skeleton(zend_class_entry *model_ce, char generator)
{
    uint32_t count = 0;

    for (zend_class_entry *ce = model_ce; ce != NULL && ce != AV_BaseModel_ce; ce = ce->parent) {
        zend_property_info *property;
        ZEND_HASH_FOREACH_PTR(&ce->properties_info, property)
        {
            (void) property;
            count++;
        }
        ZEND_HASH_FOREACH_END();
    }

    av_compiled_model *plan = emalloc(sizeof(av_compiled_model));
    memset(plan, 0, sizeof(av_compiled_model));
    plan->generator = generator;
    plan->count = 0;

    if (count == 0) {
        return plan;
    }

    plan->fields = safe_emalloc(count, sizeof(av_compiled_field), 0);
    memset(plan->fields, 0, count * sizeof(av_compiled_field));

    for (zend_class_entry *ce = model_ce; ce != NULL && ce != AV_BaseModel_ce; ce = ce->parent) {
        zend_string *property_name;
        zend_property_info *property;

        ZEND_HASH_FOREACH_STR_KEY_PTR(&ce->properties_info, property_name, property)
        {
            (void) property_name;
            if (property->flags & ZEND_ACC_STATIC)
                continue;
            if (property->flags & (ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE))
                continue;

            ZEND_ASSERT(plan->count < count);
            av_compiled_field *cf = &plan->fields[plan->count++];
            cf->prop = property;
            cf->declaring_ce = ce;
        }
        ZEND_HASH_FOREACH_END();
    }

    return plan;
}

/*
 * Compiles one field slot on first use: resolves the field name,
 * flattens the native type hint and precomputes the default-value
 * presence. A failure (pending exception from a bad Alias attribute)
 * leaves the slot uncompiled so the error keeps firing.
 */
static bool compile_field(av_compiled_field *cf, av_compiled_model *plan)
{
    av_property_info property_info = {
        .model = NULL,
        .model_ce = cf->declaring_ce,
        .property = cf->prop,
    };

    zend_string *resolved = get_property_name(&property_info, cf->prop->name, plan->generator);
    if (UNEXPECTED(EG(exception) != NULL)) {
        if (resolved != NULL && resolved != cf->prop->name) {
            zend_string_release(resolved);
        }
        return false;
    }
    if (resolved == NULL) {
        return false;
    }

    // The plan owns one reference to a stable string
    cf->name = (resolved == cf->prop->name) ? zend_string_copy(cf->prop->name) : resolved;

    zend_type type = cf->prop->type;
    if (!ZEND_TYPE_IS_SET(type)) {
        cf->untyped = true;
    } else {
        // The native-hint containment bitmap: evaluating the engine's
        // own macro once per property replaces re-interpreting the
        // zend_type on every validation
        for (uint32_t code = 0; code < 16; code++) {
            if (ZEND_TYPE_CONTAINS_CODE(type, code)) {
                cf->contains_code |= (1u << code);
            }
        }

        // The arms reference the property's own type storage (the
        // zend_type for single types, its heap-allocated list for
        // unions/intersections): both live as long as the property_info
        cf->is_intersection_group = ZEND_TYPE_IS_INTERSECTION(type);
        const zend_type *arms_src;
        uint32_t arms_count;
        if (ZEND_TYPE_HAS_LIST(type)) {
            arms_src = ZEND_TYPE_LIST(type)->types;
            arms_count = ZEND_TYPE_LIST(type)->num_types;
        } else {
            arms_src = &cf->prop->type;
            arms_count = 1;
        }

        if (arms_count > 0) {
            cf->arms = safe_emalloc(arms_count, sizeof(av_compiled_arm), 0);
            memset(cf->arms, 0, arms_count * sizeof(av_compiled_arm));
            cf->arms_count = arms_count;

            for (uint32_t i = 0; i < arms_count; i++) {
                const zend_type *arm_type = &arms_src[i];
                av_compiled_arm *arm = &cf->arms[i];

                if (ZEND_TYPE_IS_INTERSECTION(*arm_type)) {
                    arm->kind = AV_ARM_INTERSECTION;
                    arm->origin = arm_type;
                } else if (ZEND_TYPE_HAS_NAME(*arm_type)) {
                    arm->kind = AV_ARM_CLASS;
                    arm->origin = arm_type;
                    arm->ce = av_get_ce_from_type(cf->prop, arm_type);
                } else {
                    arm->kind = AV_ARM_BASIC;
                    arm->mask = ZEND_TYPE_PURE_MASK(*arm_type);
                }
            }
        }
    }

    cf->has_default = has_property_default_value(&property_info);
    return true;
}

av_compiled_model *av_get_root_compiled_model(zval *model)
{
    zend_class_entry *ce = Z_OBJCE_P(model);
    av_class_plans *plans = get_class_plans(ce);

    // An existing root compile is reused as-is
    for (uint32_t i = 0; i < 5; i++) {
        av_compiled_model *plan = plans->by_generator[i];
        if (plan != NULL && plan->is_root) {
            return plan;
        }
    }

    zval configs_obj;
    av_model_configs_properties properties;
    av_get_model_configs(&configs_obj, model, &properties, false);
    if (UNEXPECTED(EG(exception) != NULL)) {
        return NULL;
    }

    const uint32_t index = generator_index(properties.alias_generator);

    av_compiled_model *plan = plans->by_generator[index];
    if (plan == NULL) {
        plan = compile_plan_skeleton(ce, properties.alias_generator);
        plans->by_generator[index] = plan;
    }

    plan->is_root = true;
    plan->configs = properties;
    plan->hooks_overridden = av_model_overrides_hooks(ce);

    return plan;
}

/* =========================================================================
 * Runtime
 * ========================================================================= */

bool av_compiled_validate_model(zval *raw_data, zval *model, av_model_configs_properties *properties, zval *errors, zend_string *parent_path)
{
    ZEND_ASSERT(Z_TYPE_P(raw_data) == IS_ARRAY);
    ZEND_ASSERT(Z_TYPE_P(errors) == IS_ARRAY);

    zend_class_entry *ce = Z_OBJCE_P(model);
    av_class_plans *plans = get_class_plans(ce);

    // The nested lookup reuses the root's slot when the generator
    // matches: the baked field names are identical
    const uint32_t index = generator_index(properties->alias_generator);
    av_compiled_model *plan = plans->by_generator[index];
    if (plan == NULL) {
        plan = compile_plan_skeleton(ce, properties->alias_generator);
        plans->by_generator[index] = plan;
    }

    for (uint32_t i = 0; i < plan->count; i++) {
        av_compiled_field *cf = &plan->fields[i];

        if (cf->name == NULL) {
            if (!compile_field(cf, plan)) {
                return false;
            }
        }

        // field->parent keeps the parent-excludes-own-name invariant:
        // error paths and the nested recursion compose the full dot
        // path only where it is needed
        av_field field = {
            .parent = parent_path,
            .name = cf->name,
        };

        field.value = zend_hash_find(Z_ARRVAL_P(raw_data), cf->name);
        if (field.value == NULL || Z_TYPE_P(field.value) == IS_UNDEF) {
            if (cf->has_default) {
                continue;
            }

            av_property_info property_info = {
                .model = model,
                .model_ce = cf->declaring_ce,
                .property = cf->prop,
            };
            av_add_field_error_with_prefix(AV_ERROR_REQUIRED, &field, &property_info, errors);
            if (properties->stop_first_error) {
                return false;
            }
            continue;
        }

        av_property_info property_info = {
            .model = model,
            .model_ce = cf->declaring_ce,
            .property = cf->prop,
        };
        const bool is_valid = av_validate_compiled_type_hint(&field, cf, &property_info, properties, errors);

        if (!is_valid) {
            if (properties->stop_first_error) {
                return false;
            }
            continue;
        }

        // Direct offset write: the value is already of the declared
        // type, so the property name lookup and the type
        // re-verification of zend_update_property() are redundant
        zval *property_slot = OBJ_PROP(Z_OBJ_P(model), cf->prop->offset);
        zval_ptr_dtor(property_slot);
        ZVAL_COPY(property_slot, field.value);
    }

    return zend_hash_num_elements(Z_ARRVAL_P(errors)) == 0;
}
