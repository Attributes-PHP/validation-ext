#include "av_string.h"
#include "Zend/zend_API.h"
#include "av_wrappers.h"
#include "testing.h"
#include <ctype.h>
#include <stddef.h>

/**
 * Checks if a character is uppercase
 */
STATIC_ZEND_INLINE bool is_uppercase(char c)
{
    return c >= 'A' && c <= 'Z';
}

/**
 * Checks if a character is lowercase
 */
STATIC_ZEND_INLINE bool is_lowercase(char c)
{
    return c >= 'a' && c <= 'z';
}

/**
 * Checks if a character is alphanumeric
 */
STATIC_ZEND_INLINE bool is_alphanumeric(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

STATIC_INLINE void to_pascal_case(av_string *input, av_string *output)
{
    bool next_upper = true;
    for (size_t i = 0; i < input->length; i++) {
        char c = input->value[i];

        if (c == '_' || c == '-' || c == ' ') {
            next_upper = true;
            continue;
        }

        if (next_upper && is_lowercase(c)) {
            output->value[output->length++] = toupper(c);
            next_upper = false;
        } else if (next_upper && is_uppercase(c)) {
            output->value[output->length++] = c;
            next_upper = false;
        } else {
            output->value[output->length++] = c;
        }
    }

    output->value[output->length] = '\0';
}

STATIC_INLINE void to_camel_case(av_string *input, av_string *output)
{
    bool next_upper = false;
    bool first_char = true;

    for (size_t i = 0; i < input->length; i++) {
        char c = input->value[i];

        if (c == '_' || c == '-' || c == ' ') {
            next_upper = true;
            continue;
        }

        if (first_char) {
            output->value[output->length++] = tolower(c);
            first_char = false;
            next_upper = false;
        } else if (next_upper && is_lowercase(c)) {
            output->value[output->length++] = toupper(c);
            next_upper = false;
        } else if (next_upper && is_uppercase(c)) {
            output->value[output->length++] = c;
            next_upper = false;
        } else {
            output->value[output->length++] = c;
        }
    }

    output->value[output->length] = '\0';
}

STATIC_INLINE void to_snake_case(av_string *input, av_string *output)
{
    for (size_t i = 0; i < input->length; i++) {
        char c = input->value[i];

        if (c == '-' || c == ' ') {
            output->value[output->length++] = '_';
            continue;
        }

        if (is_uppercase(c)) {
            // Add underscore before uppercase letter (except at start)
            if (output->length > 0 && output->value[output->length - 1] != '_') {
                output->value[output->length++] = '_';
            }
            output->value[output->length++] = tolower(c);
        } else {
            output->value[output->length++] = c;
        }
    }

    output->value[output->length] = '\0';
}

STATIC_INLINE void to_kebab_case(av_string *input, av_string *output)
{
    for (size_t i = 0; i < input->length; i++) {
        char c = input->value[i];

        if (c == '_' || c == ' ') {
            output->value[output->length++] = '-';
            continue;
        }

        if (is_uppercase(c)) {
            // Add dash before uppercase letter (except at start)
            if (output->length > 0 && output->value[output->length - 1] != '-') {
                output->value[output->length++] = '-';
            }
            output->value[output->length++] = tolower(c);
        } else {
            output->value[output->length++] = c;
        }
    }

    output->value[output->length] = '\0';
}

/**
 * Converts a property name to PascalCase
 * Example: "firstName" -> "FirstName", "first_name" -> "FirstName"
 */
zend_string* av_to_pascal_case(zend_string *str)
{
    av_string input, output;
    input.value = ZSTR_VAL(str);
    input.length = ZSTR_LEN(str);

    // Worst case: same length as input
    output.value = av_emalloc(input.length + 1);
    output.length = 0;

    to_pascal_case(&input, &output);
    zend_string *result = av_string_init(output.value, output.length, 0);
    av_efree(output.value);
    return result;
}

/**
 * Converts a property name to camelCase
 * Example: "FirstName" -> "firstName", "first_name" -> "firstName"
 */
zend_string* av_to_camel_case(zend_string *str)
{
    av_string input, output;
    input.value = ZSTR_VAL(str);
    input.length = ZSTR_LEN(str);

    // Worst case: same length as input
    output.value = av_emalloc(input.length + 1);
    output.length = 0;

    to_camel_case(&input, &output);
    zend_string *result = av_string_init(output.value, output.length, 0);
    av_efree(output.value);
    return result;
}

/**
 * Converts a property name to snake_case
 * Example: "FirstName" -> "first_name", "firstName" -> "first_name"
 */
zend_string* av_to_snake_case(zend_string *str)
{
    av_string input, output;
    input.value = ZSTR_VAL(str);
    input.length = ZSTR_LEN(str);

    // Worst case: each character becomes 2 (char + underscore)
    output.value = av_emalloc(input.length * 2 + 1);
    output.length = 0;

    to_snake_case(&input, &output);
    zend_string *result = av_string_init(output.value, output.length, 0);
    av_efree(output.value);
    return result;
}

/**
 * Converts a property name to kebab-case
 * Example: "FirstName" -> "first-name", "firstName" -> "first-name"
 */
zend_string* av_to_kebab_case(zend_string *str)
{
    av_string input, output;
    input.value = ZSTR_VAL(str);
    input.length = ZSTR_LEN(str);

    // Worst case scenario: each character becomes 2 (char + dash)
    output.value = av_emalloc(input.length * 2 + 1);
    output.length = 0;

    to_kebab_case(&input, &output);
    zend_string *result = av_string_init(output.value, output.length, 0);
    av_efree(output.value);
    return result;
}
