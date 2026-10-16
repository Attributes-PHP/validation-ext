#ifndef AV_ERROR_MESSAGES_H
#define AV_ERROR_MESSAGES_H

#include "av_structs.h"
#include "php.h"
#include "zend_compile.h"
#include "zend_types.h"

typedef enum {
    AV_ERROR_REQUIRED,
    AV_ERROR_TYPE,
    /* Constraint error types: one per constraint, each with its own
     * default template. The #[ErrorMessage] attribute only declares
     * the required and type arguments, so constraint errors always use
     * their default template. */
    AV_ERROR_ALPHA_DASH,
    AV_ERROR_ASCII,
    AV_ERROR_BETWEEN,
    AV_ERROR_CONTAINS,
    AV_ERROR_DATETIME,
    AV_ERROR_ENDS_WITH,
    AV_ERROR_IN,
    AV_ERROR_MAX,
    AV_ERROR_MIN,
    AV_ERROR_MULTIPLE_OF,
    AV_ERROR_REGEX,
    AV_ERROR_SIZE,
    AV_ERROR_STARTS_WITH,
    AV_ERROR_ULID,
    AV_ERROR_TYPE_COUNT
} av_error_type;

typedef struct {
    uint32_t mask;
    const char *name;
    size_t length;
} av_basic_type_mapping;

// Default error message templates, one per error type (defined in av_error_messages.c).
// Supported placeholders: {field}, {value} and {expected}
extern const char *av_default_error_type_messages[];

// Placeholder substitution
zend_string *av_replace_placeholders(const char *template, size_t length, av_field *field, av_property_info *prop_info);

// Error message generation
static zend_string *generate_type_name(const zend_type type);
zend_string *build_union_type_string(zend_type property_type);
static zend_string *get_custom_error_template(av_error_type type, av_property_info *property);
static zend_string *get_property_full_path(av_field *field);

// Value conversion
zend_string *av_value_to_string(zval *value);

// Renders a list of scalar values as "a, b, c" (each through
// av_value_to_string), for constraint messages listing allowed or
// searched values. The result is a freshly allocated zend_string the
// caller must release.
zend_string *av_values_to_list_string(zval *values);

// Updates errors
static zend_always_inline void add_field_error_to_array(zval *errors_array, const char *error_message, size_t length);
static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, const char *error_message, size_t length);
void av_add_field_error_with_prefix(av_error_type type, av_field *field, av_property_info *property, zval *errors);

// Frees the per-property error template cache (request shutdown)
void av_clear_error_templates_cache(void);

// Error message generation with an explicit {expected} substitution, for
// errors whose expected type cannot be derived from the native type hint
void av_add_field_error_with_expected(av_error_type type, av_field *field, av_property_info *property, zval *errors, const zend_string *expected);

// Appends an already-rendered message under the field's dot path, for
// userland constraints whose message() result is final text
void av_add_field_message(zend_string *message, av_field *field, zval *errors);

#endif /* AV_ERROR_MESSAGES_H */