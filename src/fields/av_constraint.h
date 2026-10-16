#ifndef AV_FIELDS_Constraint_H
#define AV_FIELDS_Constraint_H

#include "php.h"
#include "../helpers/av_structs.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraint_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_Fields_Constraint_validate, 0, 1, _IS_BOOL, 0)
ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_Fields_Constraint_message, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry class_AV_Fields_Constraint_methods[] = {ZEND_ABSTRACT_ME_WITH_FLAGS(AV_Fields_Constraint, validate, arginfo_class_AV_Fields_Constraint_validate, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
                                                                             ZEND_ABSTRACT_ME_WITH_FLAGS(AV_Fields_Constraint, message, arginfo_class_AV_Fields_Constraint_message, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
                                                                                 ZEND_FE_END};

/* Registration function */
void av_register_Constraint_interface(void);

/* Validation logic for userland Constraint implementations: calls the
 * instance's validate() and, on failure, message() methods. */
bool av_validate_userland_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors);

#endif /* AV_FIELDS_Constraint_H */
