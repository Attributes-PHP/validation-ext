#include "av_exception.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_types.h"

zend_class_entry *class_AV_Exceptions_BaseException;
zend_class_entry *class_AV_Exceptions_ValidationException;

void av_register_all_exception_classes(void)
{
    register_BaseException_class();
    register_ValidationException_class();
}

zend_always_inline void av_add_field_error_to_array(zval *errors_array, char *error_message, size_t length)
{
    zval error_msg;
    ZVAL_STRINGL(&error_msg, error_message, length);
    zend_hash_next_index_insert(Z_ARRVAL_P(errors_array), &error_msg);
}

zend_always_inline void av_add_field_error(zval *errors, zend_string *field_name, char *error_message, size_t length)
{
    zval *existing = zend_hash_find(Z_ARRVAL_P(errors), field_name);
    
    if (existing && Z_TYPE_P(existing) == IS_ARRAY) {
        av_add_field_error_to_array(existing, error_message, length);
    } else {
        zval error_array;
        array_init(&error_array);
        av_add_field_error_to_array(&error_array, error_message, length);
        zend_hash_add(Z_ARRVAL_P(errors), field_name, &error_array);
    }
}

void av_add_field_error_with_prefix(zval *errors, zend_string *parent_path, zend_string *field_name, char *error_message, size_t length)
{
    if (parent_path && ZSTR_LEN(parent_path) > 0) {
        zend_string *full_path = zend_string_concat3(
            ZSTR_VAL(parent_path), ZSTR_LEN(parent_path),
            ".", 1,
            ZSTR_VAL(field_name), ZSTR_LEN(field_name)
        );
        av_add_field_error(errors, full_path, error_message, length);
        zend_string_release(full_path);
    } else {
        av_add_field_error(errors, field_name, error_message, length);
    }
}

void av_throw_validation_exception(zval *errors)
{
    zval exception_val;

    object_init_ex(&exception_val, class_AV_Exceptions_ValidationException);
    zend_call_method_with_1_params(Z_OBJ(exception_val), class_AV_Exceptions_ValidationException, NULL, "__construct", NULL, errors);
    zend_throw_exception_object(&exception_val);
}

static void register_BaseException_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Exceptions", "BaseException", class_AV_Exceptions_BaseException_methods);
    class_AV_Exceptions_BaseException = zend_register_internal_class_ex(&ce, zend_ce_exception);
}

static void register_ValidationException_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Exceptions", "ValidationException", class_AV_Exceptions_ValidationException_methods);
    class_AV_Exceptions_ValidationException = zend_register_internal_class_ex(&ce, class_AV_Exceptions_BaseException);

    zval property_allErrors_default_value;
    ZVAL_EMPTY_ARRAY(&property_allErrors_default_value);
    zend_string *property_allErrors_name = zend_string_init("allErrors", sizeof("allErrors") - 1, 1);
    zend_declare_typed_property(class_AV_Exceptions_ValidationException, property_allErrors_name, &property_allErrors_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
    zend_string_release(property_allErrors_name);
}

ZEND_METHOD(AV_Exceptions_ValidationException, __construct)
{
    zval *all_errors;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(all_errors)
    ZEND_PARSE_PARAMETERS_END();

    zend_object *this = Z_OBJ_P(getThis());

    zend_update_property(class_AV_Exceptions_ValidationException, this, "allErrors", sizeof("allErrors") - 1, all_errors);

    zval default_error_message;
    ZVAL_STRING(&default_error_message, "Invalid data");
    zend_call_method_with_1_params(this, class_AV_Exceptions_BaseException, NULL, "__construct", NULL, &default_error_message);
    zval_ptr_dtor(&default_error_message);
}

ZEND_METHOD(AV_Exceptions_ValidationException, getErrors)
{
    zval *allErrors;
    zend_object *this = Z_OBJ_P(getThis());

    allErrors = zend_read_property(class_AV_Exceptions_ValidationException, this, "allErrors", sizeof("allErrors") - 1, 0, NULL);

    if (Z_TYPE_P(allErrors) == IS_NULL) {
        array_init(return_value);
        return;
    }

    RETURN_ZVAL(allErrors, 1, 0);
}
