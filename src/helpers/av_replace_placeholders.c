#include "av_replace_placeholders.h"
#include "av_error_messages.h"
#include "av_value_to_string.h"
#include "av_wrappers.h"
#include <string.h>

/*
 * Substitutes the {field}, {value} and {expected} placeholders of an error
 * message template:
 *
 *   {field}    -> field->name
 *   {value}    -> av_value_to_string(field->value)
 *   {expected} -> build_union_type_string(prop_info->property->type)
 *
 * Each placeholder may occur multiple times. The result is a freshly
 * allocated zend_string that the caller must release with av_string_release.
 *
 * All Zend internals are reached through the mockable av_wrappers so the
 * function can be unit tested in isolation.
 */
zend_string *av_replace_placeholders(const char *template, size_t length, av_field *field, av_property_info *prop_info)
{
    struct {
        const char *search;
        size_t len;
        size_t counts;
        zend_string *replace;
    } table[] = {{"{field}", sizeof("{field}") - 1, 0, field->name}, {"{value}", sizeof("{value}") - 1, 0, NULL}, {"{expected}", sizeof("{expected}") - 1, 0, NULL}};
    size_t table_size = sizeof(table) / sizeof(table[0]);

    size_t max_template_size = length;
    size_t total_placeholders = 0;
    for (size_t i = 0; i < table_size; i++) {
        const char *search_pos = template;
        while ((search_pos = av_memnstr(search_pos, table[i].search, table[i].len, template + length))) {
            table[i].counts += 1;
            search_pos += table[i].len;
        }

        if (table[i].counts == 0)
            continue;

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
        return av_string_init(template, length, 0);
    }

    // Allocate and process in a single pass
    zend_string *result = av_string_alloc(max_template_size, 0);
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
            if (table[i].counts == 0)
                continue;
            if (input + table[i].len > input_end)
                continue;
            if (memcmp(input, table[i].search, table[i].len) != 0)
                continue;

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
    result = av_string_truncate(result, output_pos, 0);

    for (size_t i = 1; i < table_size; i++) {
        if (table[i].replace != NULL) {
            av_string_release(table[i].replace);
        }
    }

    return result;
}
