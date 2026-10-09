#include "av_array_typehint_validator.h"
#include "../av_globals.h"
#include "../av_base_model.h"
#include "../av_type.h"
#include "../fields/av_dict.h"
#include "../fields/av_intersection.h"
#include "../fields/av_sequence.h"
#include "../fields/av_union.h"
#include "../helpers/av_error_messages.h"
#include "../helpers/av_string.h"
#include "../helpers/av_wrappers.h"
#include "av_typehint_validator.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_enum.h"
#include "Zend/zend_exceptions.h"
#include "zend_type_info.h"
#include <stdint.h>
#include <string.h>

/*
 * Resolved form of an attribute arm. Specs are built once per validation
 * call: class arms are resolved eagerly (an unresolvable class is a spec
 * error and must throw regardless of the validated data) and freed with
 * free_spec() afterwards.
 */
typedef struct av_arm av_arm;
typedef struct av_spec av_spec;

struct av_arm {
    uint32_t basic_mask;     // basic type bit, 0 when not a basic arm
    zend_class_entry *ce;    // resolved class arm, NULL otherwise
    zend_string *class_name; // class name as written, for error messages
    av_spec *nested;         // nested array spec, NULL otherwise
};

struct av_spec {
    av_arm *arms;
    uint32_t arms_count;
    av_arm *key_arms; // Dict key arms, NULL for the list form
    uint32_t key_arms_count;
    bool is_intersection;
    // Whether validating an element can read its name (a key or element
    // error, a class-arm hydration or a nested array validation): pure
    // basic-type specs never build per-element path strings
    bool needs_element_names;
};

/*
 * Build context carried through the recursive spec construction: the
 * property being validated and the attribute the spec is built from
 * (recursion guard for class-name reference arms).
 */
typedef struct {
    av_property_info *prop_info;
    const zend_attribute *root_attribute;
} av_spec_context;

/*
 * Result of validating an element against a spec.
 *  AV_ELEMENT_VALID              element accepted (possibly coerced in place)
 *  AV_ELEMENT_INVALID            element rejected, error not yet reported
 *  AV_ELEMENT_INVALID_REPORTED   element rejected, a nested model or nested
 *                                array validation already reported the error
 */
typedef enum {
    AV_ELEMENT_VALID,
    AV_ELEMENT_INVALID,
    AV_ELEMENT_INVALID_REPORTED
} av_element_result;

/*
 * Type case values ARE the engine MAY_BE_* type masks (plus the private
 * dict bit), so an arm's basic mask needs no translation table: the case
 * value is the mask, with the dict bit folded into MAY_BE_ARRAY. The
 * basic_type_names table below names them for error messages.
 */

static const struct {
    uint32_t mask;
    const char *name;
    size_t length;
} basic_type_names[] = {
    {MAY_BE_BOOL, "boolean", sizeof("boolean") - 1}, {MAY_BE_LONG, "integer", sizeof("integer") - 1}, {MAY_BE_DOUBLE, "float", sizeof("float") - 1},
    {MAY_BE_STRING, "string", sizeof("string") - 1}, {MAY_BE_ARRAY, "array", sizeof("array") - 1},    {MAY_BE_OBJECT, "object", sizeof("object") - 1},
};

#define BASIC_TYPE_NAMES_SIZE (sizeof(basic_type_names) / sizeof(basic_type_names[0]))

static const struct {
    const char *lcname;
    size_t length;
} array_attribute_names[] = {
    {"attributes\\validation\\fields\\sequence", sizeof("attributes\\validation\\fields\\sequence") - 1},
    {"attributes\\validation\\fields\\union", sizeof("attributes\\validation\\fields\\union") - 1},
    {"attributes\\validation\\fields\\intersection", sizeof("attributes\\validation\\fields\\intersection") - 1},
    {"attributes\\validation\\fields\\dict", sizeof("attributes\\validation\\fields\\dict") - 1},
};

#define ARRAY_ATTRIBUTE_NAMES_SIZE (sizeof(array_attribute_names) / sizeof(array_attribute_names[0]))

static bool validate_array_elements(av_field *field, av_property_info *prop_info, const av_spec *spec, av_model_configs_properties *properties, zval *errors);
static av_spec *build_spec_from_object(zval *object, av_spec_context *ctx);
static av_spec *build_spec_from_attribute(const zend_attribute *attribute, av_spec_context *ctx);

/* =========================================================================
 * Attribute lookup
 * ========================================================================= */

/* Matches an attribute lowercase name against one of the array attribute names */
static bool is_array_attribute_name(const zend_string *lcname)
{
    for (size_t i = 0; i < ARRAY_ATTRIBUTE_NAMES_SIZE; i++) {
        if (ZSTR_LEN(lcname) == array_attribute_names[i].length && memcmp(ZSTR_VAL(lcname), array_attribute_names[i].lcname, ZSTR_LEN(lcname)) == 0) {
            return true;
        }
    }

    return false;
}

/* Exact, length-aware comparison of a lowercase attribute name */
static bool lcname_equals(const zend_string *lcname, const char *name, size_t length)
{
    return ZSTR_LEN(lcname) == length && memcmp(ZSTR_VAL(lcname), name, length) == 0;
}

/*
 * Returns the first Sequence, Union, Intersection or Dict attribute declared on
 * the property, in declaration order, or NULL when the property declares
 * none of them.
 *
 * Property attributes live in a packed HashTable keyed by insertion order
 * (declaration order); the attribute names are carried by each entry.
 */
static const zend_attribute *find_array_attribute(HashTable *attributes)
{
    zend_attribute *attribute;

    ZEND_HASH_PACKED_FOREACH_PTR(attributes, attribute)
    {
        if (attribute->offset == 0 && is_array_attribute_name(attribute->lcname)) {
            return attribute;
        }
    }
    ZEND_HASH_FOREACH_END();

    return NULL;
}

static bool is_array_attribute_ce(const zend_class_entry *ce)
{
    return ce == AV_Fields_Sequence_ce || ce == AV_Fields_Union_ce || ce == AV_Fields_Intersection_ce || ce == AV_Fields_Dict_ce;
}

static zval *read_object_property(zval *object, const char *name, size_t length)
{
    zval rv;
    zval *value = zend_read_property(Z_OBJCE_P(object), Z_OBJ_P(object), name, length, 1, &rv);
    ZVAL_DEREF(value);
    return value;
}

/* =========================================================================
 * Class name resolution (attribute arms)
 * ========================================================================= */

/*
 * Builds the declaring-class-namespace-qualified form of an unqualified
 * class name ("Address" in a class "Ns\User" -> "Ns\Address").
 *
 * Returns a newly allocated zend_string the caller must release, or NULL
 * for names that cannot be qualified: "self"/"parent", leading-backslash
 * names and names declared in a global class.
 */
static zend_string *qualify_name_with_namespace(zend_string *name, zend_class_entry *self_ce)
{
    const char *val = ZSTR_VAL(name);
    size_t len = ZSTR_LEN(name);

    if (len == 0 || self_ce == NULL || val[0] == '\\' || zend_string_equals_literal_ci(name, "self") || zend_string_equals_literal_ci(name, "parent")) {
        return NULL;
    }

    const char *class_name = ZSTR_VAL(self_ce->name);
    size_t class_name_len = ZSTR_LEN(self_ce->name);
    size_t namespace_len = 0;
    for (size_t i = class_name_len; i > 0; i--) {
        if (class_name[i - 1] == '\\') {
            namespace_len = i - 1;
            break;
        }
    }

    if (namespace_len == 0) {
        return NULL;
    }

    size_t qualified_len = namespace_len + 1 + len;
    zend_string *qualified = zend_string_alloc(qualified_len, 0);

    memcpy(ZSTR_VAL(qualified), class_name, namespace_len);
    ZSTR_VAL(qualified)[namespace_len] = '\\';
    memcpy(ZSTR_VAL(qualified) + namespace_len + 1, val, len);
    ZSTR_VAL(qualified)[qualified_len] = '\0';

    return qualified;
}

/*
 * Resolves a class name arm the way PHP resolves unqualified type hint
 * names: relative to the declaring class namespace first, then in the
 * global namespace. "self" and "parent" resolve against the declaring
 * class. Unlike compiled type hints, arm names are resolved at validation
 * time, so the lookups allow autoloading.
 *
 * Returns the class entry or NULL when the name cannot be resolved.
 */
static zend_class_entry *resolve_arm_class_name(zend_string *name, zend_class_entry *self_ce)
{
    const char *val = ZSTR_VAL(name);
    size_t len = ZSTR_LEN(name);

    if (len == 0 || self_ce == NULL) {
        return NULL;
    }

    if (zend_string_equals_literal_ci(name, "self")) {
        return self_ce;
    }

    if (zend_string_equals_literal_ci(name, "parent")) {
        return self_ce->parent;
    }

    if (val[0] == '\\') {
        // Fully qualified: strip the leading backslash
        zend_string *qualified = zend_string_init(val + 1, len - 1, 0);
        zend_class_entry *ce = zend_lookup_class_ex(qualified, NULL, ZEND_FETCH_CLASS_DEFAULT);
        zend_string_release(qualified);
        return ce;
    }

    // Unqualified: try the declaring class namespace first
    zend_string *qualified = qualify_name_with_namespace(name, self_ce);
    if (qualified != NULL) {
        zend_class_entry *ce = zend_lookup_class_ex(qualified, NULL, ZEND_FETCH_CLASS_DEFAULT);
        zend_string_release(qualified);
        if (ce != NULL) {
            return ce;
        }
    }

    return zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_DEFAULT);
}

/*
 * Throws the hard error for a class arm that cannot be resolved. The
 * message carries the property, the declaring class and every fully
 * qualified name the resolution attempted.
 */
static void throw_arm_class_not_found(zend_string *name, av_property_info *prop_info)
{
    zend_class_entry *declaring_ce = prop_info->property->ce;
    zend_string *qualified = qualify_name_with_namespace(name, declaring_ce);

    if (qualified != NULL) {
        zend_throw_exception_ex(
            zend_ce_value_error, 0, "Type \"%s\" for property \"%s\" of class \"%s\" could not be found: looked for \"%s\" and \"%s\".", ZSTR_VAL(name), ZSTR_VAL(prop_info->property->name), ZSTR_VAL(declaring_ce->name),
            ZSTR_VAL(qualified), ZSTR_VAL(name)
        );
        zend_string_release(qualified);
        return;
    }

    zend_throw_exception_ex(
        zend_ce_value_error, 0, "Type \"%s\" for property \"%s\" of class \"%s\" could not be found: looked for \"%s\".", ZSTR_VAL(name), ZSTR_VAL(prop_info->property->name), ZSTR_VAL(declaring_ce->name), ZSTR_VAL(name)
    );
}

/* =========================================================================
 * Spec construction
 * ========================================================================= */

static void free_spec(av_spec *spec)
{
    if (spec == NULL) {
        return;
    }

    for (uint32_t i = 0; i < spec->arms_count; i++) {
        if (spec->arms[i].class_name != NULL) {
            zend_string_release(spec->arms[i].class_name);
        }
        free_spec(spec->arms[i].nested);
    }
    for (uint32_t i = 0; i < spec->key_arms_count; i++) {
        if (spec->key_arms[i].class_name != NULL) {
            zend_string_release(spec->key_arms[i].class_name);
        }
    }

    if (spec->arms != NULL) {
        efree(spec->arms);
    }
    if (spec->key_arms != NULL) {
        efree(spec->key_arms);
    }
    efree(spec);
}
static void free_arms(av_arm *arms, uint32_t arms_count)
{
    for (uint32_t i = 0; i < arms_count; i++) {
        if (arms[i].class_name != NULL) {
            zend_string_release(arms[i].class_name);
        }
        free_spec(arms[i].nested);
    }
    efree(arms);
}

/*
 * Builds a single basic arm from a Type case value, which may combine
 * several engine masks with the bitwise OR operator. The value is the
 * mask; the private dict bit folds into MAY_BE_ARRAY. Key-position masks
 * are restricted to the integer and string types.
 */
static bool build_basic_arm(zend_long type_value, bool is_key, av_arm *arm)
{
    if (type_value <= 0 || ((uint32_t)type_value & ~AV_TYPE_ALL_BITS) != 0) {
        zend_throw_exception_ex(zend_ce_value_error, 0, "Unsupported type value " ZEND_LONG_FMT " in an Attributes\\Validation array type attribute.", type_value);
        return false;
    }

    uint32_t mask = (uint32_t)type_value;
    if ((mask & (uint32_t)AV_TYPE_DICT) != 0) {
        mask = (mask & ~(uint32_t)AV_TYPE_DICT) | MAY_BE_ARRAY;
    }

    if (is_key && (mask & ~(MAY_BE_LONG | MAY_BE_STRING)) != 0) {
        zend_throw_exception_ex(zend_ce_value_error, 0, "Dict key types must be integer or string, " ZEND_LONG_FMT " given.", type_value);
        return false;
    }

    arm->basic_mask = mask;
    return true;
}

/*
 * Evaluates one attribute argument into a freshly owned zval. The caller
 * releases the zval with zval_ptr_dtor().
 */
static bool evaluate_attribute_argument(zval *value, const zend_attribute *attribute, uint32_t index, zend_class_entry *scope)
{
    if (zend_get_attribute_value(value, (zend_attribute *)attribute, index, scope) != SUCCESS) {
        return false;
    }

    if (UNEXPECTED(EG(exception) != NULL)) {
        zval_ptr_dtor(value);
        ZVAL_UNDEF(value);
        return false;
    }

    return true;
}

/*
 * Builds a nested spec from an attribute object arm (arm position): the
 * element itself must be an array matching the nested spec. Attribute
 * objects reach arm position as evaluated attribute arguments (nested new
 * expressions), which the caller keeps alive until the spec is built.
 */
static bool build_nested_arm(zval *object, av_spec_context *ctx, av_arm *arm)
{
    if (!is_array_attribute_ce(Z_OBJCE_P(object))) {
        zend_throw_exception_ex(zend_ce_value_error, 0, "Unsupported type \"%s\" in an Attributes\\Validation array type attribute.", ZSTR_VAL(Z_OBJCE_P(object)->name));
        return false;
    }

    arm->nested = build_spec_from_object(object, ctx);
    return arm->nested != NULL;
}

/*
 * Builds a nested spec from a class-name reference arm (Dict::class): the
 * name must match one of the array type attributes and the property must
 * declare it. Self-references are rejected.
 *
 * Returns false without an exception when the name is not one of the
 * array type attribute names (falling back to class resolution).
 */
static bool build_reference_arm(zend_string *name, bool is_key, av_spec_context *ctx, av_arm *arm)
{
    if (is_key) {
        return false;
    }

    for (size_t i = 0; i < ARRAY_ATTRIBUTE_NAMES_SIZE; i++) {
        if (ZSTR_LEN(name) != array_attribute_names[i].length || zend_binary_strcasecmp(ZSTR_VAL(name), ZSTR_LEN(name), array_attribute_names[i].lcname, array_attribute_names[i].length) != 0) {
            continue;
        }

        const zend_attribute *reference = zend_get_attribute_str(ctx->prop_info->property->attributes, array_attribute_names[i].lcname, array_attribute_names[i].length);
        if (reference == NULL) {
            zend_throw_exception_ex(
                zend_ce_value_error, 0, "Type \"%s\" for property \"%s\" of class \"%s\" references an attribute not declared on the property.", ZSTR_VAL(name), ZSTR_VAL(ctx->prop_info->property->name),
                ZSTR_VAL(ctx->prop_info->property->ce->name)
            );
            return false;
        }

        if (reference == ctx->root_attribute) {
            zend_throw_exception_ex(
                zend_ce_value_error, 0, "Type \"%s\" for property \"%s\" of class \"%s\" references itself.", ZSTR_VAL(name), ZSTR_VAL(ctx->prop_info->property->name), ZSTR_VAL(ctx->prop_info->property->ce->name)
            );
            return false;
        }

        arm->nested = build_spec_from_attribute(reference, ctx);
        return arm->nested != NULL;
    }

    return false;
}

/*
 * Builds one arm. In arm position (is_key false) attribute objects and
 * class-name references become nested array specs; in key position only
 * the integer and string basic types are accepted.
 */
static bool build_arm(zval *arm_value, bool is_key, av_spec_context *ctx, av_arm *arm)
{
    memset(arm, 0, sizeof(av_arm));

    switch (Z_TYPE_P(arm_value)) {
        case IS_LONG:
            return build_basic_arm(Z_LVAL_P(arm_value), is_key, arm);

        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(arm_value);

            if (ce == AV_Type_ce) {
                zval *case_value = zend_enum_fetch_case_value(Z_OBJ_P(arm_value));
                if (Z_TYPE_P(case_value) != IS_LONG) {
                    ZEND_ASSERT(false);
                    return false;
                }

                return build_basic_arm(Z_LVAL_P(case_value), is_key, arm);
            }

            if (is_key) {
                // Only the top-level key position flattens a Union
                // (build_key_arms); nested attribute objects cannot
                // describe a key type
                zend_throw_exception_ex(zend_ce_value_error, 0, "Dict key types must be integer or string.");
                return false;
            }

            return build_nested_arm(arm_value, ctx, arm);
        }

        case IS_STRING: {
            if (build_reference_arm(Z_STR_P(arm_value), is_key, ctx, arm)) {
                return true;
            }

            if (UNEXPECTED(EG(exception) != NULL)) {
                return false;
            }

            if (is_key) {
                zend_throw_exception_ex(zend_ce_value_error, 0, "Dict key types must be integer or string, \"%s\" given.", ZSTR_VAL(Z_STR_P(arm_value)));
                return false;
            }

            arm->class_name = zend_string_copy(Z_STR_P(arm_value));
            arm->ce = resolve_arm_class_name(arm->class_name, ctx->prop_info->property->ce);
            if (arm->ce == NULL) {
                throw_arm_class_not_found(arm->class_name, ctx->prop_info);
                zend_string_release(arm->class_name);
                arm->class_name = NULL;
                return false;
            }

            return true;
        }

        default:
            zend_throw_exception_ex(zend_ce_value_error, 0, "Unsupported type in an Attributes\\Validation array type attribute.");
            return false;
    }
}

/*
 * Builds an arm list from a zval holding a single arm or an array of arms.
 * Arms in key position are restricted to the integer and string basic
 * types by build_arm().
 */
static bool build_arms_from_zval(zval *arms_value, bool is_key, av_spec_context *ctx, av_arm **arms_out, uint32_t *arms_count_out)
{
    bool is_list = Z_TYPE_P(arms_value) == IS_ARRAY;
    uint32_t count = is_list ? zend_hash_num_elements(Z_ARRVAL_P(arms_value)) : 1;

    av_arm *arms = emalloc((count > 0 ? count : 1) * sizeof(av_arm));
    uint32_t built = 0;
    bool result = true;

    if (is_list) {
        zval *arm_value;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arms_value), arm_value)
        {
            result = build_arm(arm_value, is_key, ctx, &arms[built]);
            if (!result) {
                break;
            }
            built++;
        }
        ZEND_HASH_FOREACH_END();
    } else {
        result = build_arm(arms_value, is_key, ctx, &arms[0]);
        built = result ? 1 : 0;
    }

    if (!result) {
        free_arms(arms, built);
        return false;
    }

    *arms_out = arms;
    *arms_count_out = built;
    return true;
}

/*
 * Builds the Dict key spec. Key arms come from the key zval (a single basic
 * type) or from a Union attribute object, whose arms are flattened into the
 * key list.
 */
static bool build_key_arms(zval *key_value, av_spec_context *ctx, av_spec *spec)
{
    if (Z_TYPE_P(key_value) == IS_OBJECT && Z_OBJCE_P(key_value) == AV_Fields_Union_ce) {
        key_value = read_object_property(key_value, "of", sizeof("of") - 1);
    }

    return build_arms_from_zval(key_value, true, ctx, &spec->key_arms, &spec->key_arms_count);
}

/*
 * Validates that an intersection spec only declares class arms.
 */
static bool validate_intersection_arms(const av_spec *spec)
{
    for (uint32_t i = 0; i < spec->arms_count; i++) {
        if (spec->arms[i].ce == NULL) {
            zend_throw_exception_ex(zend_ce_value_error, 0, "Intersection types must be class names.");
            return false;
        }
    }

    return true;
}

/*
 * Builds an element type spec for the value positions of Dict and Sequence:
 * the zval may be a single arm, a Union object (its arms are flattened
 * into the list) or an Intersection object (the value must match every
 * class). Attribute objects stay nested array shapes.
 */
static bool build_value_arms(zval *of_value, av_spec_context *ctx, av_spec *spec)
{
    if (Z_TYPE_P(of_value) == IS_OBJECT) {
        if (Z_OBJCE_P(of_value) == AV_Fields_Union_ce) {
            of_value = read_object_property(of_value, "of", sizeof("of") - 1);
        } else if (Z_OBJCE_P(of_value) == AV_Fields_Intersection_ce) {
            spec->is_intersection = true;
            of_value = read_object_property(of_value, "of", sizeof("of") - 1);
        }
    }

    if (!build_arms_from_zval(of_value, false, ctx, &spec->arms, &spec->arms_count)) {
        return false;
    }

    return !spec->is_intersection || validate_intersection_arms(spec);
}

/*
 * Builds a spec from a materialized attribute object. Attribute objects
 * reach this function as evaluated attribute arguments (new expressions
 * nested inside an attribute).
 *
 *  - Sequence     one arm, from the `of` property
 *  - Union        arm list, from the `of` array
 *  - Intersection class-only arm list, elements must match every class
 *  - Dict         key arms from `key` and value arms from `of`
 */
static av_spec *build_spec_from_object(zval *object, av_spec_context *ctx)
{
    zend_class_entry *ce = Z_OBJCE_P(object);
    av_spec *spec = emalloc(sizeof(av_spec));
    memset(spec, 0, sizeof(av_spec));

    if (ce == AV_Fields_Dict_ce) {
        if (!build_key_arms(read_object_property(object, "key", sizeof("key") - 1), ctx, spec)) {
            efree(spec);
            return NULL;
        }

        if (!build_value_arms(read_object_property(object, "of", sizeof("of") - 1), ctx, spec)) {
            free_spec(spec);
            return NULL;
        }

        return spec;
    }

    if (ce == AV_Fields_Sequence_ce) {
        if (!build_value_arms(read_object_property(object, "of", sizeof("of") - 1), ctx, spec)) {
            efree(spec);
            return NULL;
        }

        return spec;
    }

    spec->is_intersection = (ce == AV_Fields_Intersection_ce);

    if (!build_arms_from_zval(read_object_property(object, "of", sizeof("of") - 1), false, ctx, &spec->arms, &spec->arms_count)) {
        efree(spec);
        return NULL;
    }

    if (spec->is_intersection && !validate_intersection_arms(spec)) {
        free_spec(spec);
        return NULL;
    }

    return spec;
}

/*
 * Reads the two Dict arguments by name, falling back to positional order
 * (an unnamed first argument is `key`, an unnamed second is `of`).
 */
static bool find_dict_arguments(const zend_attribute *attribute, av_property_info *prop_info, uint32_t *key_index, uint32_t *of_index)
{
    *key_index = attribute->argc;
    *of_index = attribute->argc;

    for (uint32_t i = 0; i < attribute->argc; i++) {
        if (attribute->args[i].name == NULL) {
            if (*key_index == attribute->argc) {
                *key_index = i;
            } else if (*of_index == attribute->argc) {
                *of_index = i;
            }
            continue;
        }

        if (zend_string_equals_literal_ci(attribute->args[i].name, "key") && *key_index == attribute->argc) {
            *key_index = i;
        } else if (zend_string_equals_literal_ci(attribute->args[i].name, "of") && *of_index == attribute->argc) {
            *of_index = i;
        }
    }

    if (*key_index == attribute->argc || *of_index == attribute->argc) {
        zend_throw_exception_ex(
            zend_ce_value_error, 0, "Dict attribute for property \"%s\" of class \"%s\" must declare both key and of types.", ZSTR_VAL(prop_info->property->name), ZSTR_VAL(prop_info->property->ce->name)
        );
        return false;
    }

    return true;
}

/*
 * Builds a spec straight from an attribute declaration on the property.
 * Attribute arguments are evaluated on demand, so class name resolution
 * and unresolvable-name errors surface even for empty arrays.
 *
 *  - Sequence     one arm (the argument list is a one-arm list)
 *  - Union        one arm list, one arm per argument
 *  - Intersection one class-only arm list, one arm per argument
 *  - Dict         key and of arguments, by name or position
 */
static av_spec *build_spec_from_attribute(const zend_attribute *attribute, av_spec_context *ctx)
{
    av_property_info *prop_info = ctx->prop_info;
    zend_class_entry *scope = prop_info->property->ce;
    av_spec *spec = emalloc(sizeof(av_spec));
    memset(spec, 0, sizeof(av_spec));

    if (lcname_equals(attribute->lcname, array_attribute_names[3].lcname, array_attribute_names[3].length)) {
        uint32_t key_index, of_index;
        if (!find_dict_arguments(attribute, prop_info, &key_index, &of_index)) {
            efree(spec);
            return NULL;
        }

        zval key_value;
        if (!evaluate_attribute_argument(&key_value, attribute, key_index, scope)) {
            efree(spec);
            return NULL;
        }
        zval of_value;
        if (!evaluate_attribute_argument(&of_value, attribute, of_index, scope)) {
            zval_ptr_dtor(&key_value);
            efree(spec);
            return NULL;
        }

        bool result = build_key_arms(&key_value, ctx, spec);
        if (result) {
            result = build_value_arms(&of_value, ctx, spec);
        }

        zval_ptr_dtor(&key_value);
        zval_ptr_dtor(&of_value);

        if (!result) {
            free_spec(spec);
            return NULL;
        }

        return spec;
    }

    spec->is_intersection = lcname_equals(attribute->lcname, array_attribute_names[2].lcname, array_attribute_names[2].length);

    if (attribute->argc == 0) {
        zend_throw_exception_ex(
            zend_ce_value_error, 0, "Attribute \"%s\" for property \"%s\" of class \"%s\" must declare at least one type.", ZSTR_VAL(attribute->lcname), ZSTR_VAL(prop_info->property->name),
            ZSTR_VAL(prop_info->property->ce->name)
        );
        efree(spec);
        return NULL;
    }

    // Sequence declares one element type: Union and Intersection arguments
    // flatten into the element type list, like the Dict `of` position
    if (lcname_equals(attribute->lcname, array_attribute_names[0].lcname, array_attribute_names[0].length)) {
        zval of_value;
        if (!evaluate_attribute_argument(&of_value, attribute, 0, scope)) {
            efree(spec);
            return NULL;
        }

        bool result = build_value_arms(&of_value, ctx, spec);
        zval_ptr_dtor(&of_value);

        if (!result) {
            free_spec(spec);
            return NULL;
        }

        return spec;
    }

    av_arm *arms = emalloc(attribute->argc * sizeof(av_arm));
    uint32_t built = 0;
    bool result = true;

    for (uint32_t i = 0; i < attribute->argc && result; i++) {
        zval arm_value;
        if (!evaluate_attribute_argument(&arm_value, attribute, i, scope)) {
            result = false;
            break;
        }

        result = build_arm(&arm_value, false, ctx, &arms[built]);
        if (result) {
            built++;
        }
        zval_ptr_dtor(&arm_value);
    }

    if (!result) {
        free_arms(arms, built);
        efree(spec);
        return NULL;
    }

    spec->arms = arms;
    spec->arms_count = built;

    if (spec->is_intersection && !validate_intersection_arms(spec)) {
        free_spec(spec);
        return NULL;
    }

    return spec;
}

/* =========================================================================
 * Expected type strings for error messages
 * ========================================================================= */

static zend_string *spec_to_nested_string(const av_spec *spec);
static zend_string *arms_to_expected_string(const av_arm *arms, uint32_t arms_count, bool is_intersection);

/*
 * Renders a single arm for an error message: basic types by name (a
 * combined bitmask renders as the union of its types), class arms by
 * their written name and nested array specs as "array<...>".
 */
static zend_string *arm_to_expected_string(const av_arm *arm)
{
    if (arm->nested != NULL) {
        return spec_to_nested_string(arm->nested);
    }

    if (arm->class_name != NULL) {
        return zend_string_copy(arm->class_name);
    }

    for (size_t i = 0; i < BASIC_TYPE_NAMES_SIZE; i++) {
        if (arm->basic_mask == basic_type_names[i].mask) {
            return zend_string_init(basic_type_names[i].name, basic_type_names[i].length, 0);
        }
    }

    // Bitmask arms carry several type bits: render each as a union arm
    av_arm bit_arms[BASIC_TYPE_NAMES_SIZE];
    uint32_t bit_arms_count = 0;

    for (size_t i = 0; i < BASIC_TYPE_NAMES_SIZE; i++) {
        uint32_t bit_mask = basic_type_names[i].mask;
        if ((arm->basic_mask & bit_mask) != bit_mask) {
            continue;
        }

        memset(&bit_arms[bit_arms_count], 0, sizeof(av_arm));
        bit_arms[bit_arms_count].basic_mask = bit_mask;
        bit_arms_count++;
    }

    if (bit_arms_count > 0) {
        return arms_to_expected_string(bit_arms, bit_arms_count, false);
    }

    return zend_string_init("mixed", sizeof("mixed") - 1, 0);
}

/*
 * Joins arm names with ", " and a trailing " or " (union semantics) or with
 * " and " (intersection semantics).
 */
static zend_string *arms_to_expected_string(const av_arm *arms, uint32_t arms_count, bool is_intersection)
{
    if (arms_count == 0) {
        return zend_string_init("mixed", sizeof("mixed") - 1, 0);
    }

    const char *last_separator = is_intersection ? " and " : " or ";
    size_t last_separator_len = is_intersection ? (sizeof(" and ") - 1) : (sizeof(" or ") - 1);

    zend_string *result = arm_to_expected_string(&arms[0]);
    for (uint32_t i = 1; i < arms_count; i++) {
        const char *separator = (i + 1 == arms_count) ? last_separator : ", ";
        size_t separator_len = (i + 1 == arms_count) ? last_separator_len : (sizeof(", ") - 1);

        zend_string *part = arm_to_expected_string(&arms[i]);
        zend_string *joined = av_string_concat3(ZSTR_VAL(result), ZSTR_LEN(result), separator, separator_len, ZSTR_VAL(part), ZSTR_LEN(part));
        zend_string_release(result);
        zend_string_release(part);
        result = joined;
    }

    return result;
}

/*
 * Renders a spec in nested arm position: the arm accepts arrays, so the
 * accepted shape is "array<values>" or "array<keys, values>".
 */
static zend_string *spec_to_nested_string(const av_spec *spec)
{
    zend_string *value = arms_to_expected_string(spec->arms, spec->arms_count, spec->is_intersection);

    if (spec->key_arms == NULL) {
        zend_string *rendered = av_string_concat3("array<", sizeof("array<") - 1, ZSTR_VAL(value), ZSTR_LEN(value), ">", 1);
        zend_string_release(value);
        return rendered;
    }

    zend_string *key = arms_to_expected_string(spec->key_arms, spec->key_arms_count, false);
    zend_string *rendered = av_string_concat3("array<", sizeof("array<") - 1, ZSTR_VAL(key), ZSTR_LEN(key), ", ", 2);
    zend_string_release(key);
    zend_string *with_value = av_string_concat3(ZSTR_VAL(rendered), ZSTR_LEN(rendered), ZSTR_VAL(value), ZSTR_LEN(value), ">", 1);
    zend_string_release(rendered);
    zend_string_release(value);
    return with_value;
}

/* =========================================================================
 * Element validation
 * ========================================================================= */

/*
 * Converts an array element in place after zend_verify_scalar_type_hint()
 * accepted it in loose mode. Elements are validated before the property
 * write, so unlike whole-property scalar coercion there is no engine type
 * juggling to rely on: the bucket zval must be converted directly.
 */
static void coerce_scalar_element(uint32_t mask, zval *value)
{
    switch (mask) {
        case MAY_BE_LONG:
            convert_to_long(value);
            break;
        case MAY_BE_DOUBLE:
            convert_to_double(value);
            break;
        case MAY_BE_STRING:
            convert_to_string(value);
            break;
        default:
            break;
    }
}

/*
 * Checks the key kind of an element against the Dict key spec. The list
 * form has no key spec and accepts any key.
 */
static bool key_matches_arms(const zend_string *str_key, const av_spec *spec)
{
    if (spec->key_arms == NULL) {
        return true;
    }

    uint32_t needed = str_key != NULL ? MAY_BE_STRING : MAY_BE_LONG;
    for (uint32_t i = 0; i < spec->key_arms_count; i++) {
        if (spec->key_arms[i].basic_mask & needed) {
            return true;
        }
    }

    return false;
}

/*
 * Validates a single array element against a spec.
 *
 * This is the attribute twin of the union loop in av_validate_type_hint(),
 * with the same arm ordering semantics as the native fast path: exact
 * matches win over coercions, so a value that already fits an arm is never
 * rewritten through an earlier arm.
 *
 *  - pass 1 accepts values that already match an arm: basic mask bits,
 *    instanceof checks for class arms, and full nested validation for
 *    array shape arms
 *  - pass 2 tries coercions in declaration order: class arms reuse
 *    av_handle_class_by_ce() (nested BaseModel hydration and DateTime
 *    string coercion behave exactly like native class hints), bool arms
 *    use av_coerce_bool(), basic arms use zend_verify_scalar_type_hint()
 *    with a direct in-place conversion
 *
 * Coercions happen in place: element_field->value points into the parent
 * array bucket, so replacing the zval rewrites the validated array
 * directly.
 */
static av_element_result validate_value_against_arms(av_field *element_field, av_property_info *prop_info, const av_spec *spec, av_model_configs_properties *properties, zval *errors)
{
    bool reported = false;
    zend_uchar type = Z_TYPE_P(element_field->value);

    if (spec->is_intersection) {
        if (type != IS_OBJECT) {
            return AV_ELEMENT_INVALID;
        }

        for (uint32_t i = 0; i < spec->arms_count; i++) {
            if (spec->arms[i].ce == NULL || !instanceof_function(Z_OBJCE_P(element_field->value), spec->arms[i].ce)) {
                return AV_ELEMENT_INVALID;
            }
        }

        return AV_ELEMENT_VALID;
    }

    // Pass 1: exact matches only
    for (uint32_t i = 0; i < spec->arms_count; i++) {
        const av_arm *arm = &spec->arms[i];

        if (arm->nested != NULL) {
            if (type == IS_ARRAY && validate_array_elements(element_field, prop_info, arm->nested, properties, errors)) {
                return AV_ELEMENT_VALID;
            }
            continue;
        }

        if (arm->ce != NULL) {
            if (type == IS_OBJECT && instanceof_function(Z_OBJCE_P(element_field->value), arm->ce)) {
                return AV_ELEMENT_VALID;
            }
            continue;
        }

        if (arm->basic_mask & (1u << type)) {
            return AV_ELEMENT_VALID;
        }
    }

    // Pass 2: coercions in declaration order. A mask arm that exactly
    // matches cannot appear here (pass 1 already accepted it).
    for (uint32_t i = 0; i < spec->arms_count; i++) {
        const av_arm *arm = &spec->arms[i];

        if (arm->nested != NULL) {
            if (type == IS_ARRAY) {
                // Already fully validated (and failed) in pass 1: its
                // element errors are already in the errors collection
                reported = true;
            }
            continue;
        }

        if (arm->ce != NULL) {
            // Class arm coercions reuse av_handle_class_by_ce(), which
            // derives nested error paths from the field parent path.
            // Element fields follow the parent-excludes-own-name
            // invariant, so the element key is appended here first.
            av_field hydration_field = {
                .parent = element_field->parent,
                .name = element_field->name,
                .value = element_field->value,
            };

            bool handled = av_handle_class_by_ce(&hydration_field, prop_info, arm->ce, properties, errors);

            if (handled) {
                return AV_ELEMENT_VALID;
            }

            // A raw array rejected by a BaseModel class arm already
            // reported its field errors through the recursive validation,
            // mirroring is_basemodel_class_type_hint()
            if (type == IS_ARRAY && arm->ce != AV_BaseModel_ce && instanceof_function(arm->ce, AV_BaseModel_ce)) {
                reported = true;
            }
            continue;
        }

        if (!properties->strict && (arm->basic_mask & MAY_BE_BOOL) != 0) {
            if (av_coerce_bool(element_field)) {
                return AV_ELEMENT_VALID;
            }
        }

        // Scalar coercions run per bit, in the fixed bool, integer, float,
        // string order: bitmask arguments lose the declaration order a
        // plain argument list keeps, so a consistent order replaces it
        static const uint32_t scalar_coercion_masks[] = {MAY_BE_LONG, MAY_BE_DOUBLE, MAY_BE_STRING};

        for (size_t i = 0; i < sizeof(scalar_coercion_masks) / sizeof(scalar_coercion_masks[0]); i++) {
            uint32_t scalar_mask = scalar_coercion_masks[i];
            if ((arm->basic_mask & scalar_mask) == 0) {
                continue;
            }

            if (zend_verify_scalar_type_hint(scalar_mask, element_field->value, properties->strict, 0)) {
                coerce_scalar_element(scalar_mask, element_field->value);
                return AV_ELEMENT_VALID;
            }
        }
    }

    return reported ? AV_ELEMENT_INVALID_REPORTED : AV_ELEMENT_INVALID;
}

/*
 * Builds the full path of the array property on first use (field->parent
 * excludes the property's own name): "parent.name".
 */
static zend_string *ensure_base_path(av_field *field, zend_string *base_path)
{
    if (base_path == NULL) {
        base_path = av_string_dot_concat(field->parent, field->name);
        if (base_path == NULL) {
            base_path = zend_string_copy(field->name);
        }
    }

    return base_path;
}

/*
 * Computes whether validating an element of the spec can read its name:
 * a key spec, a class arm or a nested array arm. Pure basic-type specs
 * keep the happy path free of per-element string allocations.
 */
static void compute_needs_element_names(av_spec *spec)
{
    spec->needs_element_names = spec->key_arms != NULL;

    for (uint32_t i = 0; i < spec->arms_count; i++) {
        if (spec->arms[i].ce != NULL || spec->arms[i].nested != NULL) {
            spec->needs_element_names = true;
        }
        if (spec->arms[i].nested != NULL) {
            compute_needs_element_names(spec->arms[i].nested);
        }
    }
}

/*
 * Walks the array and validates/coerces every key and element in place
 * against the spec.
 *
 * Elements are addressed as av_fields so error paths follow the nested
 * model dot notation ("users.0.email"): the element parent path carries
 * the key, the reported error field addresses the key through the array
 * property path.
 *
 * Returns true only when every key and element validated.
 */
static bool validate_array_elements(av_field *field, av_property_info *prop_info, const av_spec *spec, av_model_configs_properties *properties, zval *errors)
{
    bool all_valid = true;
    zend_string *key_expected = NULL;
    zend_string *value_expected = NULL;
    zend_string *base_path = NULL;
    zend_string *str_key;
    zend_ulong num_key;
    zval *element;

    // The full path of the array property and the per-element names are
    // composed lazily, only when something reads them: an error report, a
    // class-arm hydration or a nested array validation. Specs of plain
    // basic types never build either, so the happy path of large valid
    // arrays makes no string allocations.
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(field->value), num_key, str_key, element)
    {
        if (UNEXPECTED(EG(exception) != NULL)) {
            break;
        }

        zend_string *element_name = NULL;

        // Element fields follow the parent-excludes-own-name invariant:
        // the key is the name, so error paths and the BaseModel recursion
        // both derive "users.0" from it
        av_field element_field = {
            .parent = NULL,
            .name = NULL,
            .value = element,
        };

        if (!key_matches_arms(str_key, spec)) {
            element_name = str_key != NULL ? zend_string_copy(str_key) : zend_long_to_str(num_key);
            base_path = ensure_base_path(field, base_path);
            element_field.parent = base_path;
            element_field.name = element_name;

            if (key_expected == NULL) {
                key_expected = arms_to_expected_string(spec->key_arms, spec->key_arms_count, false);
            }
            av_add_field_error_with_expected(AV_ERROR_TYPE, &element_field, prop_info, errors, key_expected);
            zend_string_release(element_name);
            all_valid = false;
            if (properties->stop_first_error) {
                break;
            }
            continue;
        }

        // An array element under a class or nested arm validates (and
        // possibly reports) through its own name; every other arm kind
        // reads the name only on the error path below
        if (spec->needs_element_names && Z_TYPE_P(element) == IS_ARRAY) {
            element_name = str_key != NULL ? zend_string_copy(str_key) : zend_long_to_str(num_key);
            base_path = ensure_base_path(field, base_path);
            element_field.parent = base_path;
            element_field.name = element_name;
        }

        av_element_result result = validate_value_against_arms(&element_field, prop_info, spec, properties, errors);

        if (result != AV_ELEMENT_VALID && element_name == NULL) {
            element_name = str_key != NULL ? zend_string_copy(str_key) : zend_long_to_str(num_key);
            base_path = ensure_base_path(field, base_path);
            element_field.parent = base_path;
            element_field.name = element_name;
        }

        if (result == AV_ELEMENT_INVALID) {
            if (value_expected == NULL) {
                value_expected = arms_to_expected_string(spec->arms, spec->arms_count, spec->is_intersection);
            }
            av_add_field_error_with_expected(AV_ERROR_TYPE, &element_field, prop_info, errors, value_expected);
        }

        if (element_name != NULL) {
            zend_string_release(element_name);
        }

        if (result != AV_ELEMENT_VALID) {
            all_valid = false;
            if (properties->stop_first_error) {
                break;
            }
        }
    }
    ZEND_HASH_FOREACH_END();

    if (key_expected != NULL) {
        zend_string_release(key_expected);
    }
    if (value_expected != NULL) {
        zend_string_release(value_expected);
    }
    if (base_path != NULL) {
        zend_string_release(base_path);
    }

    return all_valid;
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

/*
 * Spec cache, keyed by the property info and stored in the module
 * globals (see av_globals.h): attribute arguments never change within a
 * request, so the spec tree is built once per property and freed at
 * request shutdown. Failed builds (spec errors) stay uncached so their
 * errors keep firing.
 */
static void av_spec_cache_entry_dtor(zval *entry)
{
    free_spec(Z_PTR_P(entry));
}

void av_clear_array_spec_cache(void)
{
    if (AV_G(av_spec_cache) != NULL) {
        zend_hash_destroy(AV_G(av_spec_cache));
        efree(AV_G(av_spec_cache));
        AV_G(av_spec_cache) = NULL;
    }
}

bool av_validate_array_typehint_cached(av_field *field, av_compiled_field *cf, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors)
{
    if (cf->array_state == AV_ARRAY_ABSENT) {
        return true;
    }

    if (cf->array_state == AV_ARRAY_SPEC) {
        return validate_array_elements(field, prop_info, cf->array_spec, properties, errors);
    }

    // First array-valued validation for this slot: probe the attributes
    // and the spec cache once, then borrow the spec for later calls
    if (prop_info->property->attributes == NULL) {
        cf->array_state = AV_ARRAY_ABSENT;
        return true;
    }

    const zend_attribute *attribute = find_array_attribute(prop_info->property->attributes);
    if (attribute == NULL) {
        cf->array_state = AV_ARRAY_ABSENT;
        return true;
    }

    av_spec *spec = NULL;

    if (AV_G(av_spec_cache) != NULL) {
        spec = zend_hash_index_find_ptr(AV_G(av_spec_cache), (zend_ulong)(uintptr_t)prop_info->property);
    }

    if (spec == NULL) {
        av_spec_context ctx = {
            .prop_info = prop_info,
            .root_attribute = attribute,
        };

        spec = build_spec_from_attribute(attribute, &ctx);
        if (spec == NULL) {
            // Failed builds stay unprobed so their errors keep firing
            return false;
        }

        compute_needs_element_names(spec);

        if (AV_G(av_spec_cache) == NULL) {
            AV_G(av_spec_cache) = emalloc(sizeof(HashTable));
            zend_hash_init(AV_G(av_spec_cache), 8, NULL, av_spec_cache_entry_dtor, 0);
        }

        zend_hash_index_add_ptr(AV_G(av_spec_cache), (zend_ulong)(uintptr_t)prop_info->property, spec);
    }

    cf->array_spec = spec;
    cf->array_state = AV_ARRAY_SPEC;

    return validate_array_elements(field, prop_info, spec, properties, errors);
}
