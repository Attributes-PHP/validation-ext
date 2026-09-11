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

// Placeholder substitution
zend_string *av_replace_placeholders(const char *template, size_t length, av_field *field, av_property_info *prop_info);

// Error message generation
static zend_string *generate_type_name(const zend_type *type);
static bool is_type_enum(const zend_type *type);
static zend_string *build_single_type_with_article(const zend_type *type);
zend_string *build_union_type_string(zend_type property_type);
static zend_string *generate_error_message(av_field *field, zend_type property_type);
static bool av_vowel_sound(char c);

// Value conversion
zend_string *av_value_to_string(zval *value);

// Updates errors
static zend_always_inline void add_field_error_to_array(zval *errors_array, const char *error_message, size_t length);
static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, const char *error_message, size_t length);
void av_add_field_error_with_prefix(av_error_type type, av_field *field, av_property_info *property, zval *errors);

#endif /* AV_ERROR_MESSAGES_H */