#ifndef AV_Fields_Constraints_Regex_H
#define AV_Fields_Constraints_Regex_H

#include "php.h"
#include "../../helpers/av_structs.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_Regex_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_Regex___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, regex, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_Regex, __construct);

static const zend_function_entry class_AV_Fields_Constraints_Regex_methods[] = {ZEND_ME(AV_Fields_Constraints_Regex, __construct, arginfo_class_AV_Fields_Constraints_Regex___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool av_validate_Regex_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors);

/* Registration function */
void av_register_Regex_class(void);

#endif /* AV_Fields_Constraints_Regex_H */
