#include "php.h"
#include "zend_string.h"
#include "zend_type_info.h"
#include "zend_types.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "../av_model_configs.h"
#include "../av_base_model.h"
#include "../av_exception.h"
#include <stdint.h>
#include "av_typehint_validator.h"
#include <math.h>

zend_class_entry *datetime_ce;
zend_class_entry *datetime_interface_ce;

/**
 * Initializes necessary class entries.
 */
void av_init_typehint_validator() {
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

/**
 * Checks if a character starts with a vowel sound (for "an" vs "a" article selection).
 *
 * @param c The character to check
 * @return true if the character starts a word that should use "an"
 */
static bool av_vowel_sound(char c)
{
    c = (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

static bool is_type_enum(zend_type *type)
{
    if (!ZEND_TYPE_HAS_NAME(*type)) {
        return false;
    }

    zend_class_entry *ce = zend_lookup_class_ex(ZEND_TYPE_NAME(*type), NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    if (!ce) {
        return false;
    }

    return (ce->ce_flags & ZEND_ACC_ENUM);
}

static zend_class_entry *resolve_single_class_type(zend_string *name, zend_class_entry *self_ce) {
    if (zend_string_equals_literal_ci(name, "self")) {
        return self_ce;
    } else if (zend_string_equals_literal_ci(name, "parent")) {
        return self_ce->parent;
    } else {
        return zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    }
}

static zend_always_inline zend_class_entry *get_ce_from_type(zend_property_info *info, zend_type *type) {
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(*type));
    zend_string *name = ZEND_TYPE_NAME(*type);
    if (ZSTR_HAS_CE_CACHE(name)) {
        zend_class_entry *ce = ZSTR_GET_CE_CACHE(name);
        if (!ce) {
            ce = zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
        }
        return ce;
    }
    return resolve_single_class_type(name, info->ce);
}

static bool handle_intersection(
    zend_string *field_name,
    zval *value,
    zend_property_info *prop_info,
    zend_type *value_type
) {
    zend_type *intersection_type;
    ZEND_ASSERT(ZEND_TYPE_IS_INTERSECTION(*value_type));

    ZEND_TYPE_LIST_FOREACH(ZEND_TYPE_LIST(*value_type), intersection_type) {
        ZEND_ASSERT(!ZEND_TYPE_HAS_LIST(*intersection_type));

        zend_class_entry *ce = get_ce_from_type(prop_info, intersection_type);
        if (!ce || !instanceof_function(Z_OBJCE_P(value), ce)) {
            return false;
        }
    } ZEND_TYPE_LIST_FOREACH_END();
    return false;
}

/**
 * Check if a class entry is DateTime or implements DateTimeInterface.
 */
static bool is_datetime_class(zend_class_entry *ce) {
    if (!ce) return false;
    return (ce == datetime_ce || ce == datetime_interface_ce || 
            instanceof_function(ce, datetime_interface_ce));
}

static bool coerce_datetime(zval *value, zend_class_entry *target_ce, av_model_configs_properties *properties) {
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

static bool handle_class(
    zend_string *field_name,
    zval *value,
    zend_property_info *prop_info,
    zend_type *value_type,
    av_model_configs_properties *properties,
    zval *errors,
    zend_string *parent_path
) {
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(*value_type));

    zend_class_entry *ce = get_ce_from_type(prop_info, value_type);
    if (!ce) {
        return false;
    }

    if (Z_TYPE_P(value) == IS_STRING && is_datetime_class(ce)) {
        return coerce_datetime(value, ce, properties);
    }

    if (Z_TYPE_P(value) == IS_OBJECT) {
        return instanceof_function(Z_OBJCE_P(value), ce);
    }

    if (Z_TYPE_P(value) == IS_ARRAY) {
        if (!ce || ce == AV_BaseModel_ce || !instanceof_function(ce, AV_BaseModel_ce)) return false;

        zval model_obj;
        object_init_ex(&model_obj, ce);

        zend_string *nested_path = parent_path ?
            zend_string_copy(parent_path) :
            zend_string_copy(field_name);

        bool result = av_validate_model_internal(value, &model_obj, ce, properties, errors, nested_path);

        zend_string_release(nested_path);

        if (result) {
            zval_ptr_dtor(value);
            ZVAL_COPY(value, &model_obj);
        } else {
            zval_ptr_dtor(&model_obj);
        }

        return result;
    }

    return false;
}

static bool coerce_bool(
    zend_string *field_name,
    zval *value
) {
    zend_uchar type_code = Z_TYPE_P(value);

    switch (type_code) {
        case IS_LONG: {
            zend_long lval = Z_LVAL_P(value);
            if (lval == 0) {
                ZVAL_FALSE(value);
                return true;
            }

            if (lval == 1) {
                ZVAL_TRUE(value);
                return true;
            }

            break;
        }
        case IS_DOUBLE: {
            double dval = Z_DVAL_P(value);
            if (fabs(dval - 1.0) <= AV_EPSILON) {
                ZVAL_TRUE(value);
                return true;
            }

            if (fabs(dval - 0.0) <= AV_EPSILON) {
                ZVAL_FALSE(value);
                return true;
            }

            break;
        }
        case IS_STRING: {
            zend_string *s = Z_STR_P(value);
            size_t len = ZSTR_LEN(s);
            char *str = ZSTR_VAL(s);

            switch (len) {
                case 1: // '1', 't', 'y' vs '0', 'f', 'n'
                    if (str[0] == '1' || str[0] == 't' || str[0] == 'T' || str[0] == 'y' || str[0] == 'Y') {
                        ZVAL_TRUE(value);
                        return true;
                    }
                    if (str[0] == '0' || str[0] == 'f' || str[0] == 'F' || str[0] == 'n' || str[0] == 'N') {
                        ZVAL_FALSE(value);
                        return true;
                    }

                    break;
                case 2: // "on" vs "no"
                    if (zend_binary_strcasecmp(str, 2, "on", 2) == 0) {
                        ZVAL_TRUE(value);
                        return true;
                    }
                    if (zend_binary_strcasecmp(str, 2, "no", 2) == 0) {
                        ZVAL_FALSE(value);
                        return true;
                    }

                    break;

                case 3: // "yes" vs "off"
                    if (zend_binary_strcasecmp(str, 3, "yes", 3) == 0) {
                        ZVAL_TRUE(value);
                        return true;
                    }
                    if (zend_binary_strcasecmp(str, 3, "off", 3) == 0) {
                        ZVAL_FALSE(value);
                        return true;
                    }

                    break;

                case 4: // "true"
                    if (zend_binary_strcasecmp(str, 4, "true", 4) == 0) {
                        ZVAL_TRUE(value);
                        return true;
                    }

                    break;

                case 5: // "false"
                    if (zend_binary_strcasecmp(str, 5, "false", 5) == 0) {
                        ZVAL_FALSE(value);
                        return true;
                    }

                    break;
            }

            break;
        }
    }

    return false;
}

static zend_string* generate_type_name(zend_type *type)
{
    if (ZEND_TYPE_IS_INTERSECTION(*type)) {
        return zend_string_init("mixed", 5, 0);
    }

    if (ZEND_TYPE_HAS_NAME(*type)) {
        return zend_string_copy(ZEND_TYPE_NAME(*type));
    }

    uint32_t type_mask = ZEND_TYPE_PURE_MASK(*type);

    if (type_mask == MAY_BE_BOOL) return zend_string_init("boolean", 7, 0);
    if (type_mask == MAY_BE_LONG) return zend_string_init("integer", 7, 0);
    if (type_mask == MAY_BE_DOUBLE) return zend_string_init("float", 5, 0);
    if (type_mask == MAY_BE_STRING) return zend_string_init("string", 6, 0);
    if (type_mask == MAY_BE_ARRAY) return zend_string_init("array", 5, 0);
    if (type_mask == MAY_BE_OBJECT) return zend_string_init("object", 6, 0);
    if (type_mask == MAY_BE_RESOURCE) return zend_string_init("resource", 8, 0);
    if (type_mask == MAY_BE_NULL) return zend_string_init("null", 4, 0);
    if (type_mask == MAY_BE_FALSE) return zend_string_init("false", 5, 0);
    if (type_mask == MAY_BE_TRUE) return zend_string_init("true", 4, 0);
    if (type_mask == MAY_BE_CALLABLE) return zend_string_init("callable", 8, 0);
    if (type_mask == MAY_BE_VOID) return zend_string_init("void", 4, 0);

    return zend_string_init("mixed", 5, 0);
}

static zend_always_inline zend_string* build_single_type_with_article(zend_type *type)
{
    zend_string *type_name = generate_type_name(type);
    const char *article = av_vowel_sound(ZSTR_VAL(type_name)[0]) ? "an" : "a";
    zend_string *result = zend_string_concat3(
        article, strlen(article),
        " ", 1,
        ZSTR_VAL(type_name), ZSTR_LEN(type_name)
    );
    zend_string_release(type_name);
    return result;
}

static zend_always_inline zend_string* build_union_type_string(zend_type property_type)
{
    uint32_t pure_mask = ZEND_TYPE_PURE_MASK(property_type);
    bool is_simple_union = !ZEND_TYPE_HAS_LIST(property_type) &&
                          ZEND_TYPE_IS_SET(property_type) &&
                          (pure_mask & (pure_mask - 1)) != 0;

    if (is_simple_union) {
        struct { uint32_t mask; const char *name; } type_mappings[] = {
            {MAY_BE_BOOL, "boolean"},
            {MAY_BE_LONG, "integer"},
            {MAY_BE_DOUBLE, "float"},
            {MAY_BE_STRING, "string"},
            {MAY_BE_ARRAY, "array"},
            {MAY_BE_OBJECT, "object"},
            {MAY_BE_RESOURCE, "resource"},
            {MAY_BE_NULL, "null"},
            {MAY_BE_FALSE, "false"},
            {MAY_BE_TRUE, "true"},
            {MAY_BE_CALLABLE, "callable"},
            {MAY_BE_VOID, "void"},
            {0, NULL}
        };

        zend_string *result = NULL;
        bool first = true;

        for (int i = 0; type_mappings[i].name != NULL; i++) {
            if (!(pure_mask & type_mappings[i].mask)) continue;

            const char *article = av_vowel_sound(type_mappings[i].name[0]) ? "an" : "a";
            zend_string *type_part = zend_string_concat3(
                article, strlen(article),
                " ", 1,
                type_mappings[i].name, strlen(type_mappings[i].name)
            );

            if (first) {
                result = type_part;
                first = false;
            } else {
                zend_string *prefix = zend_string_init(" or ", 4, 0);
                zend_string *temp = zend_string_concat3(
                    ZSTR_VAL(result), ZSTR_LEN(result),
                    ZSTR_VAL(prefix), ZSTR_LEN(prefix),
                    ZSTR_VAL(type_part), ZSTR_LEN(type_part)
                );
                zend_string_release(result);
                zend_string_release(prefix);
                zend_string_release(type_part);
                result = temp;
            }
        }

        return result;
    }

    zend_string *result = NULL;
    zend_type *type;

    ZEND_TYPE_FOREACH(property_type, type) {
        if (ZEND_TYPE_IS_INTERSECTION(*type)) continue;

        zend_string *type_part = build_single_type_with_article(type);

        if (!result) {
            result = type_part;
        } else {
            zend_string *prefix = zend_string_init(" or ", 4, 0);
            zend_string *temp = zend_string_concat3(
                ZSTR_VAL(result), ZSTR_LEN(result),
                ZSTR_VAL(prefix), ZSTR_LEN(prefix),
                ZSTR_VAL(type_part), ZSTR_LEN(type_part)
            );
            zend_string_release(result);
            zend_string_release(prefix);
            zend_string_release(type_part);
            result = temp;
        }
    } ZEND_TYPE_FOREACH_END();

    return result;
}

static zend_string* generate_error_message(zend_type property_type, zend_string *field_name, zend_string *parent_path)
{
    zend_string *full_path = field_name;
    if (parent_path && ZSTR_LEN(parent_path) > 0) {
        full_path = zend_string_concat3(
            ZSTR_VAL(parent_path), ZSTR_LEN(parent_path),
            ".", 1,
            ZSTR_VAL(field_name), ZSTR_LEN(field_name)
        );
    }

    zend_type *type;
    ZEND_TYPE_FOREACH(property_type, type) {
        if (ZEND_TYPE_HAS_NAME(*type) && is_type_enum(type)) {
            zend_string *msg = zend_string_concat3(
                "The selected ", sizeof("The selected ") - 1,
                ZSTR_VAL(full_path), ZSTR_LEN(full_path),
                " is invalid.", sizeof(" is invalid.") - 1
            );
            if (full_path != field_name) zend_string_release(full_path);
            return msg;
        }
    } ZEND_TYPE_FOREACH_END();

    zend_string *type_string = build_union_type_string(property_type);
    if (!type_string) {
        type_string = zend_string_init("mixed", 5, 0);
    }

    size_t message_len = sizeof("The ") - 1 + ZSTR_LEN(field_name) + sizeof(" must be ") - 1 + ZSTR_LEN(type_string) + sizeof(".") - 1;
    zend_string *message = zend_string_alloc(message_len, 0);

    snprintf(ZSTR_VAL(message), message_len + 1, "The %s must be %s.",
             ZSTR_VAL(field_name), ZSTR_VAL(type_string));

    zend_string_release(type_string);
    if (full_path != field_name) zend_string_release(full_path);

    return message;
}

/**
 * Validates that a value matches the property's type hint.
 *
 * For union types, tries each type in the union.
 *
 * @param field_name    The name of the field being validated
 * @param value         The value to validate (may be modified in-place)
 * @param prop_info     Property type information
 * @param properties    Model configuration properties (for recursive validation)
 * @param parent_path   Parent path for nested error messages
 * @param errors        Error collection array
 * @return              true if validation succeeds, false otherwise
 */
bool av_validate_type_hint(
    zend_string *field_name,
    zval *value,
    zend_property_info *prop_info,
    av_model_configs_properties *properties,
    zval *errors,
    zend_string *parent_path
)
{
    zend_type property_type = prop_info->type;

    if (!ZEND_TYPE_IS_SET(property_type)) return true;

    if (ZEND_TYPE_CONTAINS_CODE(property_type, Z_TYPE_P(value))) return true;

    zend_type *type;
    ZEND_TYPE_FOREACH(property_type, type) {
        if (ZEND_TYPE_IS_INTERSECTION(*type)) {
            if (handle_intersection(field_name, value, prop_info, type)) return true;
            continue;
        }

        if (ZEND_TYPE_HAS_NAME(*type)) {
            if (handle_class(field_name, value, prop_info, type, properties, errors, parent_path)) return true;
            continue;
        }

        uint32_t type_mask = ZEND_TYPE_PURE_MASK(*type);
        if (!properties->strict && type_mask & MAY_BE_BOOL) {
            if (coerce_bool(field_name, value)) return true;
            continue;
        }

        if (zend_verify_scalar_type_hint(type_mask, value, properties->strict, 0)) return true;
    }  ZEND_TYPE_FOREACH_END();

    zend_string *error_msg = generate_error_message(property_type, field_name, parent_path);
    av_add_field_error_with_prefix(errors, parent_path, field_name, ZSTR_VAL(error_msg), ZSTR_LEN(error_msg));
    zend_string_release(error_msg);
    return false;
}
