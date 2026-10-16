#include "av_constraint.h"
#include "av_field.h"
#include "../helpers/av_error_messages.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"

zend_class_entry *AV_Fields_Constraint_ce;

void av_register_Constraint_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Constraint", class_AV_Fields_Constraint_methods);
    AV_Fields_Constraint_ce = zend_register_internal_interface(&ce);
    ZEND_ASSERT(AV_Fields_Constraint_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Constraint_ce, 1, AV_Fields_Field_ce);
}

/*
 * Userland constraint validation: the interface contract guarantees the
 * validate() and message() methods, with the engine enforcing their
 * signatures since PHP classes cannot implement an interface without
 * them. Exceptions thrown by either method propagate to the caller of
 * validate() like model hook errors.
 */
bool av_validate_userland_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors)
{
    (void)property;

    zend_class_entry *ce = Z_OBJCE_P(constraint);
    zend_function *validate = zend_hash_str_find_ptr(&ce->function_table, "validate", sizeof("validate") - 1);
    ZEND_ASSERT(validate != NULL);

    zval result;
    ZVAL_UNDEF(&result);
    zend_call_known_instance_method_with_1_params(validate, Z_OBJ_P(constraint), &result, field->value);

    bool valid = Z_TYPE(result) == IS_TRUE;
    if (Z_TYPE(result) != IS_UNDEF) {
        zval_ptr_dtor(&result);
    }

    if (EG(exception) != NULL) {
        return false;
    }

    if (valid) {
        return true;
    }

    zend_function *message = zend_hash_str_find_ptr(&ce->function_table, "message", sizeof("message") - 1);
    ZEND_ASSERT(message != NULL);

    zval message_value;
    ZVAL_UNDEF(&message_value);
    zend_call_known_instance_method_with_0_params(message, Z_OBJ_P(constraint), &message_value);
    if (EG(exception) != NULL) {
        if (Z_TYPE(message_value) != IS_UNDEF) {
            zval_ptr_dtor(&message_value);
        }
        return false;
    }

    ZEND_ASSERT(Z_TYPE(message_value) == IS_STRING);
    zend_string *rendered = av_replace_placeholders(Z_STRVAL(message_value), Z_STRLEN(message_value), field, property);
    zval_ptr_dtor(&message_value);

    av_add_field_message(rendered, field, errors);
    zend_string_release(rendered);
    return false;
}
