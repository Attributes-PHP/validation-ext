#include "av_error_messages.h"
#include "../av_base_model.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "zend_types.h"

static const char* av_error_type_messages[] = {
    [AV_ERROR_REQUIRED] = "The field is required",
    [AV_ERROR_TYPE] = "Invalid type",
    [AV_ERROR_ENUM] = "Invalid enum value"
};

static const size_t av_error_type_message_lengths[] = {
    [AV_ERROR_REQUIRED] = sizeof("The field is required") - 1,
    [AV_ERROR_TYPE] = sizeof("Invalid type") - 1,
    [AV_ERROR_ENUM] = sizeof("Invalid enum value") - 1
};

static zend_always_inline void add_field_error_to_array(zval *errors_array, const char *error_message, size_t length)
{
    zval error_msg;
    ZVAL_STRINGL(&error_msg, error_message, length);
    zend_hash_next_index_insert(Z_ARRVAL_P(errors_array), &error_msg);
}

static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, const char *error_message, size_t length)
{
    zval *existing = zend_hash_find(Z_ARRVAL_P(errors), field_name);

    if (existing && Z_TYPE_P(existing) == IS_ARRAY) {
        add_field_error_to_array(existing, error_message, length);
    } else {
        zval error_array;
        array_init(&error_array);
        add_field_error_to_array(&error_array, error_message, length);
        zend_hash_add(Z_ARRVAL_P(errors), field_name, &error_array);
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

static zend_string* generate_error_message(av_field *field, zend_type property_type)
{
    zend_string *full_path = field->name;
    if (field->parent && ZSTR_LEN(field->parent) > 0) {
        full_path = zend_string_concat3(
            ZSTR_VAL(field->parent), ZSTR_LEN(field->parent),
            ".", 1,
            ZSTR_VAL(field->name), ZSTR_LEN(field->name)
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
            if (full_path != field->name) zend_string_release(full_path);
            return msg;
        }
    } ZEND_TYPE_FOREACH_END();

    zend_string *type_string = build_union_type_string(property_type);
    if (!type_string) {
        type_string = zend_string_init("mixed", sizeof("mixed") - 1, 0);
    }

    size_t message_len = sizeof("The ") - 1 + ZSTR_LEN(field->name) + sizeof(" must be ") - 1 + ZSTR_LEN(type_string) + sizeof(".") - 1;
    zend_string *message = zend_string_alloc(message_len, 0);

    snprintf(ZSTR_VAL(message), message_len + 1, "The %s must be %s.",
             ZSTR_VAL(field->name), ZSTR_VAL(type_string));

    zend_string_release(type_string);
    if (full_path != field->name) zend_string_release(full_path);

    return message;
}

void av_add_field_error_with_prefix(av_error_type type, av_field *field, av_property_info *property, zval *errors)
{
    const char *error_message = av_error_type_messages[type];
    size_t length = av_error_type_message_lengths[type];

    if (!field->parent || ZSTR_LEN(field->parent) == 0) {
        add_field_error(errors, field->name, error_message, length);
        return;
    }

    zend_string *full_path = zend_string_concat3(
        ZSTR_VAL(field->parent), ZSTR_LEN(field->parent),
        ".", 1,
        ZSTR_VAL(field->name), ZSTR_LEN(field->name)
    );
    add_field_error(errors, full_path, error_message, length);
    zend_string_release(full_path);
}
