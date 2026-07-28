#include "options.h"
#include <string.h>
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "ext/spl/spl_exceptions.h"


bool attributes_validation_validate_method_parameter(char *option, char **all_valid_options, size_t length, attributes_validation_invalid_method_parameter *invalid_parameter_error)
{
    for (size_t i = 0; i < length; i++) {
        if (strcasecmp(option, all_valid_options[i]) == 0) {
            return true;
        }
    }

    throw_invalid_parameter_exception(option, all_valid_options, length, invalid_parameter_error);
    return false;
}

static inline void throw_invalid_parameter_exception(char *option, char **all_options, size_t length, attributes_validation_invalid_method_parameter *invalid_option_error)
{
    // Create list of valid options
    char options_list[256] = "";
    size_t pos = 0;
    for (size_t i = 0; i < length; i++) {
        if (i > 0) {
            if (pos + 2 < sizeof(options_list)) {
                options_list[pos++] = ',';
                options_list[pos++] = ' ';
            }
        }
        size_t opt_len = strlen(all_options[i]);
        if (pos + opt_len < sizeof(options_list)) {
            memcpy(options_list + pos, all_options[i], opt_len);
            pos += opt_len;
        }
    }
    options_list[pos] = '\0';

    zend_argument_value_error(1, "must be of one of the following options: %s", options_list);
}
