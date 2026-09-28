#ifndef AV_ERROR_MESSAGES_H
#define AV_ERROR_MESSAGES_H

#include "av_structs.h"
#include "php.h"
#include "zend_compile.h"
#include "zend_types.h"

typedef enum {
    AV_ERROR_REQUIRED,
    AV_ERROR_TYPE
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
static bool is_type_enum(const zend_type type);
zend_string *build_union_type_string(zend_type property_type);
static zend_string *get_custom_error_template(av_error_type type, av_property_info *property);
static zend_string *get_property_full_path(av_field *field);

// Value conversion
zend_string *av_value_to_string(zval *value);

// Updates errors
static zend_always_inline void add_field_error_to_array(zval *errors_array, const char *error_message, size_t length);
static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, const char *error_message, size_t length);
void av_add_field_error_with_prefix(av_error_type type, av_field *field, av_property_info *property, zval *errors);

#endif /* AV_ERROR_MESSAGES_H */