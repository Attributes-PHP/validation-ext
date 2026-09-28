#include "php.h"
#include "zend_string.h"
#include "zend_type_info.h"
#include "zend_types.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_operators.h"
#include "../av_model_configs.h"
#include "../av_base_model.h"
#include "../av_exception.h"
#include "../helpers/av_error_messages.h"
#include <stdint.h>
#include <math.h>
#include "av_typehint_validator.h"
#include "../helpers/av_docstring_parser.h"
#include "../helpers/av_string.h"

zend_class_entry *datetime_ce;
zend_class_entry *datetime_interface_ce;

/**
 * Initializes necessary class entries.
 */
void av_init_typehint_validator()
{
    zend_string *datetime_str = zend_string_init("DateTime", sizeof("DateTime") - 1, 0);
    datetime_ce = zend_lookup_class_ex(datetime_str, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    zend_string_release(datetime_str);

    zend_string *datetime_interface_str = zend_string_init("DateTimeInterface", sizeof("DateTimeInterface") - 1, 0);
    datetime_interface_ce = zend_lookup_class_ex(datetime_interface_str, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    zend_string_release(datetime_interface_str);

    if (!datetime_ce || !datetime_interface_ce) {
        zend_throw_error(NULL, "DateTime or DateTimeInterface class not available. Ensure the datetime extension is loaded.");
    }
}

static zend_class_entry *resolve_single_class_type(zend_string *name, zend_class_entry *self_ce)
{
    if (zend_string_equals_literal_ci(name, "self")) {
        return self_ce;
    } else if (zend_string_equals_literal_ci(name, "parent")) {
        return self_ce->parent;
    } else {
        return zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    }
}

/*
 * Resolves a class/alias type name to its class entry: "self" and "parent"
 * resolve against the declaring class, cached names through the string CE
 * cache, everything else through a class lookup.
 */
static zend_class_entry *resolve_class_name(zend_string *name, zend_class_entry *self_ce)
{
    if (ZSTR_HAS_CE_CACHE(name)) {
        zend_class_entry *ce = ZSTR_GET_CE_CACHE(name);
        if (!ce) {
            ce = zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
        }
        return ce;
    }

    return resolve_single_class_type(name, self_ce);
}

static zend_always_inline zend_class_entry *get_ce_from_type(zend_property_info *info, const zend_type *type)
{
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(*type));
    return resolve_class_name(ZEND_TYPE_NAME(*type), info->ce);
}

static bool handle_intersection(av_field *field, av_property_info *prop_info, const zend_type *value_type)
{
    const zend_type *intersection_type;
    ZEND_ASSERT(ZEND_TYPE_IS_INTERSECTION(*value_type));

    ZEND_TYPE_LIST_FOREACH(ZEND_TYPE_LIST(*value_type), intersection_type)
    {
        ZEND_ASSERT(!ZEND_TYPE_HAS_LIST(*intersection_type));

        zend_class_entry *ce = get_ce_from_type(prop_info->property, intersection_type);
        if (!ce || !instanceof_function(Z_OBJCE_P(field->value), ce)) {
            return false;
        }
    }
    ZEND_TYPE_LIST_FOREACH_END();
    return false;
}

/**
 * Check if a class entry is DateTime or implements DateTimeInterface.
 */
static bool is_datetime_class(zend_class_entry *ce)
{
    if (!ce)
        return false;
    return (ce == datetime_ce || ce == datetime_interface_ce || instanceof_function(ce, datetime_interface_ce));
}

static bool coerce_datetime(zval *value, zend_class_entry *target_ce, av_model_configs_properties *properties)
{
    ZEND_ASSERT(Z_TYPE_P(value) == IS_STRING);

    const zend_string *str = Z_STR_P(value);
    if (ZSTR_LEN(str) <= 12) {
        return false;
    }

    zval format, datetime_obj;
    ZVAL_STRING(&format, "X-m-d\\TH:i:sP");

    zval args[2];
    ZVAL_COPY(&args[0], &format);
    ZVAL_COPY(&args[1], value);

    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval function_name;

    ZVAL_STRING(&function_name, "date_create_from_format");

    fci.size = sizeof(fci);
    fci.object = NULL;
    fci.function_name = function_name;
    fci.retval = &datetime_obj;
    fci.param_count = 2;
    fci.params = args;
    fci.named_params = NULL;

    fcc.function_handler = NULL;
    fcc.calling_scope = NULL;
    fcc.called_scope = NULL;
    fcc.object = NULL;

    zend_result result = zend_call_function(&fci, &fcc);

    zval_ptr_dtor(&format);
    zval_ptr_dtor(&function_name);

    if (result == SUCCESS && Z_TYPE(datetime_obj) == IS_OBJECT && Z_OBJCE_P(&datetime_obj) == datetime_ce) {
        zval_ptr_dtor(value);
        ZVAL_COPY(value, &datetime_obj);
        return true;
    }

    if (Z_TYPE(datetime_obj) != IS_UNDEF) {
        zval_ptr_dtor(&datetime_obj);
    }

    return false;
}

/**
 * Validates a value against a resolved class entry. Shared by the native
 * type hint path (handle_class()) and by docstring array element specs
 * such as "array<Address>".
 */
static bool handle_class_by_ce(av_field *field, av_property_info *prop_info, zend_class_entry *ce, av_model_configs_properties *properties, zval *errors)
{
    if (!ce) {
        return false;
    }

    if (Z_TYPE_P(field->value) == IS_STRING && is_datetime_class(ce)) {
        return coerce_datetime(field->value, ce, properties);
    }

    if (Z_TYPE_P(field->value) == IS_OBJECT) {
        return instanceof_function(Z_OBJCE_P(field->value), ce);
    }

    if (Z_TYPE_P(field->value) == IS_ARRAY) {
        if (ce == AV_BaseModel_ce || !instanceof_function(ce, AV_BaseModel_ce))
            return false;

        zval model_obj;
        object_init_ex(&model_obj, ce);

        zend_string *nested_path = av_string_dot_concat(field->parent, field->name);
        if (nested_path == NULL) {
            nested_path = zend_string_copy(field->name);
        }

        av_property_info property_info = {
            .model = &model_obj,
            .model_ce = ce,
        };
        bool result = av_validate_model_internal(field->value, &property_info, properties, errors, nested_path);

        zend_string_release(nested_path);

        if (result) {
            zval_ptr_dtor(field->value);
            ZVAL_COPY(field->value, &model_obj);
        } else {
            zval_ptr_dtor(&model_obj);
        }

        return result;
    }

    return false;
}

/*
 * Builds the declaring-class-namespace-qualified form of an unqualified
 * docstring class name ("Address" in a class "Ns\User" -> "Ns\Address").
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
 * Resolves a docstring class name the way PHP resolves unqualified type
 * hint names: relative to the declaring class namespace first, then in
 * the global namespace. Leading backslashes are honored, "self" and
 * "parent" resolve against the declaring class.
 *
 * Unlike compiled type hints, docstring names are resolved at validation
 * time, so the lookups allow autoloading: a model class that is defined
 * but not yet loaded must still resolve.
 *
 * Returns the class entry or NULL when the name cannot be resolved.
 */
static zend_class_entry *resolve_docstring_class(zend_string *name, zend_class_entry *self_ce)
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
 * Throws the hard error for a docstring class that cannot be resolved.
 * The message carries the property, the declaring class and every fully
 * qualified name the resolution attempted, so the docstring can be fixed
 * without further lookups.
 */
static void throw_docstring_class_not_found(zend_string *name, av_property_info *prop_info)
{
    zend_class_entry *declaring_ce = prop_info->property->ce;
    zend_string *qualified = qualify_name_with_namespace(name, declaring_ce);

    if (qualified != NULL) {
        zend_throw_exception_ex(
            zend_ce_value_error, 0,
            "Docstring class \"%s\" for property \"%s\" of class \"%s\" could not be found: looked for \"%s\" and \"%s\". Docstring type names do not resolve PHP imports; use the fully qualified name.", ZSTR_VAL(name),
            ZSTR_VAL(prop_info->property->name), ZSTR_VAL(declaring_ce->name), ZSTR_VAL(qualified), ZSTR_VAL(name)
        );
        zend_string_release(qualified);
        return;
    }

    zend_throw_exception_ex(
        zend_ce_value_error, 0, "Docstring class \"%s\" for property \"%s\" of class \"%s\" could not be found: looked for \"%s\". Docstring type names do not resolve PHP imports; use the fully qualified name.",
        ZSTR_VAL(name), ZSTR_VAL(prop_info->property->name), ZSTR_VAL(declaring_ce->name), ZSTR_VAL(name)
    );
}

/*
 * Resolves a docstring class arm's class entry, memoizing the result on
 * the node for the spec's lifetime (one validation call): without the
 * memo, an "array<Address>" spec would repeat the namespace-qualified
 * lookup for every element and every union pass.
 *
 * A class that cannot be resolved is a docstring error, not a data error:
 * a ValueError naming the attempted fully qualified names is thrown on
 * first resolution. NULL is returned only after that throw (or after an
 * earlier resolution already threw).
 */
static zend_class_entry *docstring_node_ce(av_docstring_type_node *node, av_property_info *prop_info)
{
    if (!node->ce_resolved) {
        node->ce = resolve_docstring_class(node->class_name, prop_info->property->ce);
        node->ce_resolved = true;

        if (node->ce == NULL) {
            throw_docstring_class_not_found(node->class_name, prop_info);
        }
    }

    return node->ce;
}

/*
 * Resolves every class arm of a spec (including nested shapes) eagerly,
 * so a docstring class that cannot be found fails fast regardless of the
 * validated data: an empty or fully valid array still throws.
 */
static void resolve_docstring_spec_classes(av_array_spec *spec, av_property_info *prop_info)
{
    for (av_docstring_type_node *current = spec->value_spec; current != NULL; current = current->next) {
        if (current->array_spec != NULL) {
            resolve_docstring_spec_classes(current->array_spec, prop_info);
            continue;
        }

        if (current->is_class) {
            docstring_node_ce(current, prop_info);
        }
    }
}

static bool handle_class(av_field *field, av_property_info *prop_info, const zend_type *value_type, av_model_configs_properties *properties, zval *errors)
{
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(*value_type));

    zend_class_entry *ce = resolve_class_name(ZEND_TYPE_NAME(*value_type), prop_info->property->ce);
    return handle_class_by_ce(field, prop_info, ce, properties, errors);
}

static bool coerce_bool(av_field *field)
{
    zend_uchar type_code = Z_TYPE_P(field->value);

    switch (type_code) {
        case IS_LONG: {
            zend_long lval = Z_LVAL_P(field->value);
            if (lval == 0) {
                ZVAL_FALSE(field->value);
                return true;
            }

            if (lval == 1) {
                ZVAL_TRUE(field->value);
                return true;
            }

            break;
        }
        case IS_DOUBLE: {
            double dval = Z_DVAL_P(field->value);
            if (fabs(dval - 1.0) <= AV_EPSILON) {
                ZVAL_TRUE(field->value);
                return true;
            }

            if (fabs(dval - 0.0) <= AV_EPSILON) {
                ZVAL_FALSE(field->value);
                return true;
            }

            break;
        }
        case IS_STRING: {
            zend_string *s = Z_STR_P(field->value);
            size_t len = ZSTR_LEN(s);
            char *str = ZSTR_VAL(s);

            switch (len) {
                case 1: // '1', 't', 'y' vs '0', 'f', 'n'
                    if (str[0] == '1' || str[0] == 't' || str[0] == 'T' || str[0] == 'y' || str[0] == 'Y') {
                        ZVAL_TRUE(field->value);
                        return true;
                    }
                    if (str[0] == '0' || str[0] == 'f' || str[0] == 'F' || str[0] == 'n' || str[0] == 'N') {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;
                case 2: // "on" vs "no"
                    if (zend_binary_strcasecmp(str, 2, "on", 2) == 0) {
                        ZVAL_TRUE(field->value);
                        return true;
                    }
                    if (zend_binary_strcasecmp(str, 2, "no", 2) == 0) {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;

                case 3: // "yes" vs "off"
                    if (zend_binary_strcasecmp(str, 3, "yes", 3) == 0) {
                        ZVAL_TRUE(field->value);
                        return true;
                    }
                    if (zend_binary_strcasecmp(str, 3, "off", 3) == 0) {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;

                case 4: // "true"
                    if (zend_binary_strcasecmp(str, 4, "true", 4) == 0) {
                        ZVAL_TRUE(field->value);
                        return true;
                    }

                    break;

                case 5: // "false"
                    if (zend_binary_strcasecmp(str, 5, "false", 5) == 0) {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;
            }

            break;
        }
    }

    return false;
}

static bool is_basemodel_class_type_hint(av_property_info *prop_info, const zend_type *property_type)
{
    uint32_t pure_mask = ZEND_TYPE_PURE_MASK(*property_type);
    bool is_single_class = !ZEND_TYPE_HAS_LIST(*property_type) && ZEND_TYPE_HAS_NAME(*property_type) && (pure_mask & (pure_mask - 1)) == 0;
    if (!is_single_class)
        return false;

    zend_class_entry *model_ce = get_ce_from_type(prop_info->property, property_type);
    return model_ce && instanceof_function(model_ce, AV_BaseModel_ce);
}

/*
 * Result of validating an array element against a docstring type union.
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

static bool validate_array_elements(av_field *field, av_property_info *prop_info, av_array_spec *spec, av_model_configs_properties *properties, zval *errors);

/**
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

/**
 * Checks the key kind of an element against the dict form key spec
 * ("array<int, ...>" / "array<string, ...>"). The list form has no key
 * spec and accepts any key.
 */
static bool key_matches_spec(const zend_string *str_key, const av_docstring_type_node *key_spec)
{
    if (key_spec == NULL) {
        return true;
    }

    uint32_t needed = str_key != NULL ? MAY_BE_STRING : MAY_BE_LONG;
    for (const av_docstring_type_node *current = key_spec; current != NULL; current = current->next) {
        if (current->mask & needed) {
            return true;
        }
    }

    return false;
}

/**
 * Validates a single array element against a docstring type union chain.
 *
 * This is the docstring twin of the union loop in av_validate_type_hint(),
 * with the same arm ordering semantics as the native fast path: exact
 * matches win over coercions, so a value that already fits an arm is
 * never rewritten through an earlier arm.
 *
 *  - pass 1 accepts values that already match an arm: basic mask bits,
 *    instanceof checks for class arms, and full nested validation for
 *    array shape arms
 *  - pass 2 tries coercions in declaration order: class arms reuse
 *    handle_class_by_ce() (nested BaseModel hydration and DateTime string
 *    coercion behave exactly like native class hints), bool arms use
 *    coerce_bool(), basic arms use zend_verify_scalar_type_hint() with a
 *    direct in-place conversion
 *
 * Coercions happen in place: element_field->value points into the parent
 * array bucket, so replacing the zval rewrites the validated array
 * directly.
 */
static av_element_result validate_value_against_docstring_type(av_field *element_field, av_property_info *prop_info, av_docstring_type_node *node, av_model_configs_properties *properties, zval *errors)
{
    bool reported = false;
    zend_uchar type = Z_TYPE_P(element_field->value);

    // Pass 1: exact matches only
    for (av_docstring_type_node *current = node; current != NULL; current = current->next) {
        if (current->array_spec != NULL) {
            if (type == IS_ARRAY && validate_array_elements(element_field, prop_info, current->array_spec, properties, errors)) {
                return AV_ELEMENT_VALID;
            }
            continue;
        }

        if (current->is_class) {
            if (type != IS_OBJECT) {
                continue;
            }
            zend_class_entry *ce = docstring_node_ce(current, prop_info);
            if (ce != NULL && instanceof_function(Z_OBJCE_P(element_field->value), ce)) {
                return AV_ELEMENT_VALID;
            }
            continue;
        }

        if (current->mask & (1u << type)) {
            return AV_ELEMENT_VALID;
        }
    }

    // Pass 2: coercions in declaration order. A mask arm that exactly
    // matches cannot appear here (pass 1 already accepted it).
    for (av_docstring_type_node *current = node; current != NULL; current = current->next) {
        if (current->array_spec != NULL) {
            if (type == IS_ARRAY) {
                // Already fully validated (and failed) in pass 1: its
                // element errors are already in the errors collection
                reported = true;
            }
            continue;
        }

        if (current->is_class) {
            zend_class_entry *ce = docstring_node_ce(current, prop_info);
            if (ce == NULL) {
                // Unresolvable: docstring_node_ce() already threw
                continue;
            }

            if (handle_class_by_ce(element_field, prop_info, ce, properties, errors)) {
                return AV_ELEMENT_VALID;
            }

            // A raw array rejected by a BaseModel class arm already
            // reported its field errors through the recursive validation,
            // mirroring is_basemodel_class_type_hint().
            if (type == IS_ARRAY && ce != AV_BaseModel_ce && instanceof_function(ce, AV_BaseModel_ce)) {
                reported = true;
            }
            continue;
        }

        if (!properties->strict && current->mask == MAY_BE_BOOL) {
            if (coerce_bool(element_field)) {
                return AV_ELEMENT_VALID;
            }
            continue;
        }

        if (zend_verify_scalar_type_hint(current->mask, element_field->value, properties->strict, 0)) {
            coerce_scalar_element(current->mask, element_field->value);
            return AV_ELEMENT_VALID;
        }
    }

    return reported ? AV_ELEMENT_INVALID_REPORTED : AV_ELEMENT_INVALID;
}

/**
 * Walks the array and validates/coerces every key and element in place
 * against the docstring spec.
 *
 * Elements are addressed as av_fields so error paths follow the nested
 * model dot notation ("users.0.email"): the element parent path carries
 * the key, the reported error field addresses the key through the array
 * property path.
 *
 * Returns true only when every key and element validated.
 */
static bool validate_array_elements(av_field *field, av_property_info *prop_info, av_array_spec *spec, av_model_configs_properties *properties, zval *errors)
{
    bool all_valid = true;
    zend_string *key_expected = NULL;
    zend_string *value_expected = NULL;
    zend_string *str_key;
    zend_ulong num_key;
    zval *element;

    // Full path of the array property (field->parent excludes the
    // property's own name), so element errors and nested model recursion
    // compose dot-notation paths like "users.0.email"
    zend_string *base_path = av_string_dot_concat(field->parent, field->name);
    if (base_path == NULL) {
        base_path = zend_string_copy(field->name);
    }

    // Resolve the spec's classes before touching the data: an
    // unresolvable docstring class is a docstring error and must throw
    // regardless of the validated elements
    resolve_docstring_spec_classes(spec, prop_info);
    if (UNEXPECTED(EG(exception))) {
        zend_string_release(base_path);
        return false;
    }

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(field->value), num_key, str_key, element)
    {
        if (UNEXPECTED(EG(exception))) {
            break;
        }

        zend_string *element_name = str_key != NULL ? zend_string_copy(str_key) : zend_long_to_str(num_key);

        // Element fields follow the parent-excludes-own-name invariant:
        // the key is the name, so error paths and the BaseModel recursion
        // both derive "users.0" from it
        av_field element_field = {
            .parent = base_path,
            .name = element_name,
            .value = element,
        };

        if (!key_matches_spec(str_key, spec->key_spec)) {
            if (key_expected == NULL) {
                key_expected = av_docstring_type_list_to_string(spec->key_spec);
            }
            av_add_field_error_with_expected(AV_ERROR_TYPE, &element_field, prop_info, errors, key_expected);
            zend_string_release(element_name);
            all_valid = false;
            if (properties->stop_first_error) {
                break;
            }
            continue;
        }

        av_element_result result = validate_value_against_docstring_type(&element_field, prop_info, spec->value_spec, properties, errors);

        if (result == AV_ELEMENT_INVALID) {
            if (value_expected == NULL) {
                value_expected = av_docstring_type_list_to_string(spec->value_spec);
            }
            av_add_field_error_with_expected(AV_ERROR_TYPE, &element_field, prop_info, errors, value_expected);
        }

        zend_string_release(element_name);

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
    zend_string_release(base_path);

    return all_valid;
}

/**
 * Entry point for docstring array shapes: parses the property doc comment
 * and, when it declares a shape ("@var array<...>"), validates the array
 * keys and elements against it.
 *
 * Returns true when there is no shape to enforce (plain `array` behaves as
 * before) or when every key and element validated.
 */
static bool validate_array_typehint(av_field *field, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors)
{
    av_array_spec *spec = av_docstring_parse_array_spec(prop_info->property);
    if (spec == NULL) {
        return true;
    }

    bool result = validate_array_elements(field, prop_info, spec, properties, errors);

    av_docstring_free_array_spec(spec);
    return result;
}

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
bool av_validate_type_hint(av_field *field, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors)
{
    ZEND_ASSERT(field->value != NULL);

    zend_type property_type = prop_info->property->type;

    if (!ZEND_TYPE_IS_SET(property_type))
        return true;

    if (ZEND_TYPE_CONTAINS_CODE(property_type, Z_TYPE_P(field->value))) {
        if (Z_TYPE_P(field->value) == IS_ARRAY) {
            // The native hint accepted the array: enforce the docstring
            // shape ("@var array<...>"), when the property declares one
            return validate_array_typehint(field, prop_info, properties, errors);
        }

        return true;
    }

    const zend_type *type;
    ZEND_TYPE_FOREACH(property_type, type)
    {
        if (ZEND_TYPE_IS_INTERSECTION(*type)) {
            if (handle_intersection(field, prop_info, type))
                return true;
            continue;
        }

        if (ZEND_TYPE_HAS_NAME(*type)) {
            if (handle_class(field, prop_info, type, properties, errors))
                return true;
            continue;
        }

        uint32_t type_mask = ZEND_TYPE_PURE_MASK(*type);
        if (!properties->strict && type_mask & MAY_BE_BOOL) {
            if (coerce_bool(field))
                return true;
            continue;
        }

        if (zend_verify_scalar_type_hint(type_mask, field->value, properties->strict, 0))
            return true;
    }
    ZEND_TYPE_FOREACH_END();

    if (!is_basemodel_class_type_hint(prop_info, &property_type)) {
        av_add_field_error_with_prefix(AV_ERROR_TYPE, field, prop_info, errors);
    }

    return false;
}
