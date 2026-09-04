#include "av_error_messages.h"
#include "../av_base_model.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_operators.h"
#include "php.h"
#include "zend.h"
#include "zend_portability.h"
#include "zend_string.h"
#include "zend_types.h"
#include <stddef.h>
#include <string.h>

static const char* av_error_type_messages[] = {
    [AV_ERROR_REQUIRED] = "The {field} field is required.",
    [AV_ERROR_TYPE] = "The {field} must be {expected}.",
};

static zend_string* av_value_to_string(zval *value)
{
    // TODO: Implement zval to zend_string implementation
    if (value == NULL) {
        return zend_string_init("null", 4, 0);
    }
    return zval_get_string(value);
}

static zend_string* av_replace_placeholders(const char *template, size_t length, av_field *field, av_property_info *prop_info)
{
    struct { const char *search; size_t len; size_t counts; zend_string *replace; } table[] = {
        {"{field}", sizeof("{field}") - 1, 0, field->name},
        {"{value}", sizeof("{value}") - 1, 0, NULL},
        {"{expected}", sizeof("{expected}") - 1, 0, NULL}
    };
    size_t table_size = sizeof(table) / sizeof(table[0]);

    size_t max_template_size = length;
    size_t total_placeholders = 0;
    for (size_t i = 0; i < table_size; i++) {
        const char *search_pos = template;
        while ((search_pos = php_memnstr(search_pos, table[i].search, table[i].len, template + length))) {
            table[i].counts += 1;
            search_pos += table[i].len;
        }

        if (table[i].counts == 0) continue;

        total_placeholders += table[i].counts;

        if (table[i].replace == NULL) {
            if (i == 1) { // {value}
                table[i].replace = av_value_to_string(field->value);
            } else if (i == 2) { // {expected}
                table[i].replace = build_union_type_string(prop_info->property->type);
            }
        }
        max_template_size += table[i].counts * (ZSTR_LEN(table[i].replace) - table[i].len);
    }

    if (total_placeholders == 0) {
        return zend_string_init(template, length, 0);
    }

    // Allocate and process in a single pass
    zend_string *result = zend_string_alloc(max_template_size, 0);
    char *output = ZSTR_VAL(result);
    size_t output_pos = 0;

    const char *input = template;
    const char *input_end = template + length;

    while (input < input_end) {
        if (total_placeholders == 0) {
            output[output_pos++] = *input++;
            continue;
        }

        bool replaced = false;
        for (size_t i = 0; i < table_size; i++) {
            if (table[i].counts == 0) continue;
            if (input + table[i].len > input_end) continue;
            if (memcmp(input, table[i].search, table[i].len) != 0) continue;

            table[i].counts -= 1;
            total_placeholders -= 1;

            memcpy(output + output_pos, ZSTR_VAL(table[i].replace), ZSTR_LEN(table[i].replace));
            output_pos += ZSTR_LEN(table[i].replace);
            input += table[i].len;
            replaced = true;
            break;
        }

        if (!replaced) {
            output[output_pos++] = *input++;
        }
    }

    ZEND_ASSERT(output_pos == max_template_size);

    // Null-terminate and truncate to actual size
    output[output_pos] = '\0';
    result = zend_string_truncate(result, output_pos, 0);

    for (size_t i = 1; i < table_size; i++) {
        if (table[i].replace != NULL) {
            zend_string_release(table[i].replace);
        }
    }

    return result;
}

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

static zend_always_inline zend_string* build_union_only_basic_types(uint32_t pure_mask)
{
    struct { uint32_t mask; const char *name; size_t length; } type_mappings[] = {
        {MAY_BE_BOOL, "boolean", sizeof("boolean") - 1},
        {MAY_BE_LONG, "integer", sizeof("integer") - 1},
        {MAY_BE_DOUBLE, "float", sizeof("float") - 1},
        {MAY_BE_STRING, "string", sizeof("string") - 1},
        {MAY_BE_ARRAY, "array", sizeof("array") - 1},
        {MAY_BE_OBJECT, "object", sizeof("object") - 1},
        {MAY_BE_RESOURCE, "resource", sizeof("resource") - 1},
        {MAY_BE_NULL, "null", sizeof("null") - 1},
        {MAY_BE_CALLABLE, "callable", sizeof("callable") - 1},
        {MAY_BE_VOID, "void", sizeof("void") - 1},
    };
    size_t type_mappings_size = sizeof(type_mappings) / sizeof(type_mappings[0]);

    struct { size_t max_string_size; size_t total; } count = {0, 0};
    for (int i = 0; i < type_mappings_size; i++) {
        if (!(pure_mask & type_mappings[i].mask)) continue;

        count.max_string_size += type_mappings[i].length;
        count.total += 1;
    }

    ZEND_ASSERT(count.max_string_size > 0);
    ZEND_ASSERT(count.total >= 2);
    ZEND_ASSERT(count.total <= type_mappings_size);

    size_t num_commas = fmax(count.total - 2, 0) * (sizeof(", ") - 1);  // Comma + space after comma
    size_t num_ors = sizeof(" or ") - 1;
    count.max_string_size += num_commas + num_ors;

    zend_string *result = zend_string_alloc(count.max_string_size, 0);
    char *output = ZSTR_VAL(result);
    size_t output_pos = 0;
    int i = 0;

    // Commas
    for (; i < type_mappings_size && count.total - 2 > 0; i++) {
        if (!(pure_mask & type_mappings[i].mask)) continue;

        count.total -= 1;
        // Append type-hint
        memcpy(output + output_pos, type_mappings[i].name, type_mappings[i].length);
        output_pos += type_mappings[i].length;

        // Append comma
        memcpy(output + output_pos, ", ", sizeof(", ") - 1);
        output_pos += sizeof(", ") - 1;
    }

    // or
    for (; i < type_mappings_size && count.total > 0; i++) {
        if (!(pure_mask & type_mappings[i].mask)) continue;

        count.total -= 1;

        // Append type-hint
        memcpy(output + output_pos, type_mappings[i].name, type_mappings[i].length);
        output_pos += type_mappings[i].length;

        if (count.total > 0) {
            // Append or
            memcpy(output + output_pos, " or ", sizeof(" or ") - 1);
            output_pos += sizeof(" or ") - 1;
        }
    }

    output[output_pos] = '\0';

    ZEND_ASSERT(output_pos == count.max_string_size);
    return zend_string_truncate(result, output_pos, 0);
}

static zend_always_inline zend_string* build_union_type_string(zend_type property_type)
{
    uint32_t pure_mask = ZEND_TYPE_PURE_MASK(property_type);
    bool is_simple_union = !ZEND_TYPE_HAS_LIST(property_type) &&
                          ZEND_TYPE_IS_SET(property_type) &&
                          (pure_mask & (pure_mask - 1)) != 0;

    if (is_simple_union) return build_union_only_basic_types(pure_mask);

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
    const char *template = av_error_type_messages[type];
    zend_string *replaced_message = av_replace_placeholders(template, strlen(template), field, property);

    if (!field->parent || ZSTR_LEN(field->parent) == 0) {
        add_field_error(errors, field->name, ZSTR_VAL(replaced_message), ZSTR_LEN(replaced_message));
        zend_string_release(replaced_message);
        return;
    }

    zend_string *full_path = zend_string_concat3(
        ZSTR_VAL(field->parent), ZSTR_LEN(field->parent),
        ".", 1,
        ZSTR_VAL(field->name), ZSTR_LEN(field->name)
    );
    add_field_error(errors, full_path, ZSTR_VAL(replaced_message), ZSTR_LEN(replaced_message));
    zend_string_release(full_path);
    zend_string_release(replaced_message);
}
