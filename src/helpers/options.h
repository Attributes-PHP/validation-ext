#ifndef VALIDATION_EXT_HELPERS_OPTIONS_H
#define VALIDATION_EXT_HELPERS_OPTIONS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *class_name;
    char *method_name;
    int parameter_number;
    char *name;
} validation_ext_invalid_method_parameter;

/* Registration function */
bool attributes_validation_validate_method_parameter(char *value, char **all_options, size_t length, validation_ext_invalid_method_parameter *invalid_parameter_error);
static inline void throw_invalid_parameter_exception(char *value, char **all_options, size_t length, validation_ext_invalid_method_parameter *invalid_parameter_error);

#endif /* VALIDATION_EXT_HELPERS_OPTIONS_H */
