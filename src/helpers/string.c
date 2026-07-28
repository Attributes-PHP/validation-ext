#include "string.h"
#include "Zend/zend_API.h"
#include <ctype.h>

/**
 * Checks if a character is uppercase
 */
static zend_always_inline bool is_uppercase(char c)
{
    return c >= 'A' && c <= 'Z';
}

/**
 * Checks if a character is lowercase
 */
static zend_always_inline bool is_lowercase(char c)
{
    return c >= 'a' && c <= 'z';
}

/**
 * Checks if a character is alphanumeric
 */
static zend_always_inline bool is_alphanumeric(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

/**
 * Converts a property name to PascalCase
 * Example: "firstName" -> "FirstName", "first_name" -> "FirstName"
 */
zend_string* attributes_validation_to_pascal_case(zend_string *str)
{
    const char *input = ZSTR_VAL(str);
    size_t len = ZSTR_LEN(str);
    char *output;
    size_t output_len = 0;
    bool next_upper = true;

    // Allocate buffer for worst case (all characters become uppercase + separators removed)
    output = emalloc(len + 1);

    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        
        if (c == '_' || c == '-' || c == ' ') {
            next_upper = true;
            continue;
        }

        if (next_upper && is_lowercase(c)) {
            output[output_len++] = toupper(c);
            next_upper = false;
        } else if (next_upper && is_uppercase(c)) {
            output[output_len++] = c;
            next_upper = false;
        } else {
            output[output_len++] = c;
        }
    }

    output[output_len] = '\0';
    zend_string *result = zend_string_init(output, output_len, 0);
    efree(output);
    return result;
}

/**
 * Converts a property name to camelCase
 * Example: "FirstName" -> "firstName", "first_name" -> "firstName"
 */
zend_string* attributes_validation_to_camel_case(zend_string *str)
{
    const char *input = ZSTR_VAL(str);
    size_t len = ZSTR_LEN(str);
    char *output;
    size_t output_len = 0;
    bool next_upper = false;
    bool first_char = true;

    // Allocate buffer for worst case
    output = emalloc(len + 1);

    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        
        if (c == '_' || c == '-' || c == ' ') {
            next_upper = true;
            continue;
        }

        if (first_char) {
            output[output_len++] = tolower(c);
            first_char = false;
            next_upper = false;
        } else if (next_upper && is_lowercase(c)) {
            output[output_len++] = toupper(c);
            next_upper = false;
        } else if (next_upper && is_uppercase(c)) {
            output[output_len++] = c;
            next_upper = false;
        } else {
            output[output_len++] = c;
        }
    }

    output[output_len] = '\0';
    zend_string *result = zend_string_init(output, output_len, 0);
    efree(output);
    return result;
}

/**
 * Converts a property name to snake_case
 * Example: "FirstName" -> "first_name", "firstName" -> "first_name"
 */
zend_string* attributes_validation_to_snake_case(zend_string *str)
{
    const char *input = ZSTR_VAL(str);
    size_t len = ZSTR_LEN(str);
    char *output;
    size_t output_len = 0;

    // Worst case: each character becomes 2 (char + underscore)
    output = emalloc(len * 2 + 1);

    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        
        if (c == '-' || c == ' ') {
            output[output_len++] = '_';
            continue;
        }

        if (is_uppercase(c)) {
            // Add underscore before uppercase letter (except at start)
            if (output_len > 0 && output[output_len - 1] != '_') {
                output[output_len++] = '_';
            }
            output[output_len++] = tolower(c);
        } else {
            output[output_len++] = c;
        }
    }

    output[output_len] = '\0';
    zend_string *result = zend_string_init(output, output_len, 0);
    efree(output);
    return result;
}

/**
 * Converts a property name to kebab-case
 * Example: "FirstName" -> "first-name", "firstName" -> "first-name"
 */
zend_string* attributes_validation_to_kebab_case(zend_string *str)
{
    const char *input = ZSTR_VAL(str);
    size_t len = ZSTR_LEN(str);
    char *output;
    size_t output_len = 0;

    // Worst case: each character becomes 2 (char + dash)
    output = emalloc(len * 2 + 1);

    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        
        if (c == '_' || c == ' ') {
            output[output_len++] = '-';
            continue;
        }

        if (is_uppercase(c)) {
            // Add dash before uppercase letter (except at start)
            if (output_len > 0 && output[output_len - 1] != '-') {
                output[output_len++] = '-';
            }
            output[output_len++] = tolower(c);
        } else {
            output[output_len++] = c;
        }
    }

    output[output_len] = '\0';
    zend_string *result = zend_string_init(output, output_len, 0);
    efree(output);
    return result;
}
