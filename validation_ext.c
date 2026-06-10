#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "Zend/zend_API.h"

/* Function declaration */
ZEND_BEGIN_ARG_INFO_EX(arginfo_hello_world, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(hello_world);

/* Function definition */
PHP_FUNCTION(hello_world)
{
    php_printf("Hello World\n");
    RETURN_TRUE;
}

/* Function entries */
const zend_function_entry validation_ext_functions[] = {
    ZEND_NS_FE("Attributes\\Validation", hello_world, arginfo_hello_world)
    PHP_FE_END
};

/* Module entry */
zend_module_entry validation_ext_module_entry = {
    STANDARD_MODULE_HEADER,
    "validation_ext",
    validation_ext_functions,
    NULL, /* Module startup */
    NULL, /* Module shutdown */
    NULL, /* Request startup */
    NULL, /* Request shutdown */
    NULL, /* Module info */
    "1.0",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_VALIDATION_EXT
ZEND_GET_MODULE(validation_ext)
#endif
