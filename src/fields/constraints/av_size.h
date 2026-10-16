#ifndef AV_Fields_Constraints_Size_H
#define AV_Fields_Constraints_Size_H

#include "php.h"
#include "../../helpers/av_structs.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_Size_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_Size___construct, 0, 0, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, min, IS_LONG, 1, "null")
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, max, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_Size, __construct);

static const zend_function_entry class_AV_Fields_Constraints_Size_methods[] = {ZEND_ME(AV_Fields_Constraints_Size, __construct, arginfo_class_AV_Fields_Constraints_Size___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool av_validate_Size_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors);

/* Registration function */
void av_register_Size_class(void);

#endif /* AV_Fields_Constraints_Size_H */
