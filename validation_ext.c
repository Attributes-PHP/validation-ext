#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "Zend/zend_API.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_inheritance.h"
#include "ext/standard/info.h"

/* Class entries */
zend_class_entry *validation_ext_Validator_ce;
zend_class_entry *validation_ext_Validatable_ce;

/* Object definition */
typedef struct {
    zend_object std;
    zval context;
    zval validator;
} validation_ext_Validator_object;

static inline validation_ext_Validator_object *validation_ext_Validator_fetch_object(zend_object *obj) {
    return (validation_ext_Validator_object *)((char *)obj - XtOffsetOf(validation_ext_Validator_object, std));
}

/* Object handlers */
static zend_object_handlers validation_ext_Validator_handlers;

/* Method declarations */
PHP_METHOD(Validation_Validator, __construct);
PHP_METHOD(Validation_Validator, validate);
PHP_METHOD(Validation_Validator, validateCallable);
PHP_METHOD(Validation_Validator, getDefaultPropertyValidator);
PHP_METHOD(Validation_Validator, getDefaultAliasGenerator);
PHP_METHOD(Validation_Validator, getAliasName);
PHP_METHOD(Validation_Validator, isToValidate);

/* Argument info */
ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator___construct, 0, 0, 0)
    ZEND_ARG_INFO(0, validator)
    ZEND_ARG_INFO(0, stopFirstError)
    ZEND_ARG_INFO(0, strict)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator_validate, 0, 0, 2)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator_validateCallable, 0, 0, 2)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, call)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator_getDefaultPropertyValidator, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator_getDefaultAliasGenerator, 0, 0, 1)
    ZEND_ARG_INFO(0, reflection)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator_getAliasName, 0, 0, 2)
    ZEND_ARG_INFO(0, reflection)
    ZEND_ARG_INFO(0, defaultAliasGenerator)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Validation_Validator_isToValidate, 0, 0, 1)
    ZEND_ARG_INFO(0, reflection)
ZEND_END_ARG_INFO()

/* Method entries for Validator class */
static zend_function_entry validation_ext_Validator_methods[] = {
    PHP_ME(Validation_Validator, __construct, arginfo_Validation_Validator___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Validation_Validator, validate, arginfo_Validation_Validator_validate, ZEND_ACC_PUBLIC)
    PHP_ME(Validation_Validator, validateCallable, arginfo_Validation_Validator_validateCallable, ZEND_ACC_PUBLIC)
    PHP_ME(Validation_Validator, getDefaultPropertyValidator, arginfo_Validation_Validator_getDefaultPropertyValidator, ZEND_ACC_PROTECTED)
    PHP_ME(Validation_Validator, getDefaultAliasGenerator, arginfo_Validation_Validator_getDefaultAliasGenerator, ZEND_ACC_PROTECTED)
    PHP_ME(Validation_Validator, getAliasName, arginfo_Validation_Validator_getAliasName, ZEND_ACC_PROTECTED)
    PHP_ME(Validation_Validator, isToValidate, arginfo_Validation_Validator_isToValidate, ZEND_ACC_PROTECTED)
    PHP_FE_END
};

/* Object creation and destruction */
static zend_object *validation_ext_Validator_create_object(zend_class_entry *ce)
{
    validation_ext_Validator_object *obj = (validation_ext_Validator_object *)zend_object_alloc(sizeof(validation_ext_Validator_object), ce);

    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);

    ZVAL_NULL(&obj->context);
    ZVAL_NULL(&obj->validator);

    return &obj->std;
}

static void validation_ext_Validator_free_object(zend_object *object)
{
    validation_ext_Validator_object *obj = validation_ext_Validator_fetch_object(object);

    zval_ptr_dtor(&obj->context);
    zval_ptr_dtor(&obj->validator);

    zend_object_std_dtor(&obj->std);
}

/* Method implementations */
PHP_METHOD(Validation_Validator, __construct)
{
    zval *validator = NULL;
    zend_bool stopFirstError = 0;
    zend_bool strict = 0;
    zval *context = NULL;
    
    ZEND_PARSE_PARAMETERS_START(0, 4)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(validator)
        Z_PARAM_BOOL(stopFirstError)
        Z_PARAM_BOOL(strict)
        Z_PARAM_ZVAL(context)
    ZEND_PARSE_PARAMETERS_END();

    validation_ext_Validator_object *obj = validation_ext_Validator_fetch_object(Z_OBJ_P(getThis()));

    if (validator) {
        ZVAL_COPY(&obj->validator, validator);
    }
    
    if (context) {
        ZVAL_COPY(&obj->context, context);
    }
}

PHP_METHOD(Validation_Validator, validate)
{
    // Empty implementation as requested
}

PHP_METHOD(Validation_Validator, validateCallable)
{
    // Empty implementation as requested
}

PHP_METHOD(Validation_Validator, getDefaultPropertyValidator)
{
    // Empty implementation as requested
}

PHP_METHOD(Validation_Validator, getDefaultAliasGenerator)
{
    // Empty implementation as requested
}

PHP_METHOD(Validation_Validator, getAliasName)
{
    // Empty implementation as requested
}

PHP_METHOD(Validation_Validator, isToValidate)
{
    // Empty implementation as requested
}

/* Module startup */
PHP_MINIT_FUNCTION(validation_ext)
{
    zend_class_entry ce;
    
    /* Register Validatable interface */
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\Validatable", NULL);
    validation_ext_Validatable_ce = zend_register_internal_interface(&ce);

    /* Register Validator class */
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\Validator", validation_ext_Validator_methods);
    ce.create_object = validation_ext_Validator_create_object;
    
    validation_ext_Validator_ce = zend_register_internal_class(&ce);
    
    /* Setup object handlers */
    memcpy(&validation_ext_Validator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    validation_ext_Validator_handlers.offset = XtOffsetOf(validation_ext_Validator_object, std);
    validation_ext_Validator_handlers.free_obj = validation_ext_Validator_free_object;
    validation_ext_Validator_handlers.clone_obj = NULL;

    /* Declare properties */
    zend_declare_property_null(validation_ext_Validator_ce, ZEND_STRL("context"), ZEND_ACC_PROTECTED);
    zend_declare_property_null(validation_ext_Validator_ce, ZEND_STRL("validator"), ZEND_ACC_PROTECTED);

    /* Implement Validatable interface */
    zend_do_implement_interface(validation_ext_Validator_ce, validation_ext_Validatable_ce);

    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(validation_ext)
{
    return SUCCESS;
}

/* Request startup */
PHP_RINIT_FUNCTION(validation_ext)
{
    return SUCCESS;
}

/* Request shutdown */
PHP_RSHUTDOWN_FUNCTION(validation_ext)
{
    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(validation_ext)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "validation_ext", "enabled");
    php_info_print_table_end();
    
    DISPLAY_INI_ENTRIES();
}

/* Module entry */
zend_module_entry validation_ext_module_entry = {
    STANDARD_MODULE_HEADER,
    "validation_ext",
    NULL, /* Function entries */
    PHP_MINIT(validation_ext),
    PHP_MSHUTDOWN(validation_ext),
    PHP_RINIT(validation_ext),
    PHP_RSHUTDOWN(validation_ext),
    PHP_MINFO(validation_ext),
    "1.0",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_VALIDATION_EXT
ZEND_GET_MODULE(validation_ext)
#endif
