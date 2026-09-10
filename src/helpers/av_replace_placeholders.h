#ifndef AV_HELPERS_REPLACE_PLACEHOLDERS_H
#define AV_HELPERS_REPLACE_PLACEHOLDERS_H

#include "av_structs.h"
#include <Zend/zend_types.h>

/*
 * Substitutes the {field}, {value} and {expected} placeholders of an error
 * message template with the corresponding field/value/type strings.
 *
 * See av_replace_placeholders.c for the substitution rules.
 */
zend_string *av_replace_placeholders(const char *template, size_t length, av_field *field, av_property_info *prop_info);

#endif /* AV_HELPERS_REPLACE_PLACEHOLDERS_H */
