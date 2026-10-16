#ifndef AV_Fields_Constraints_DateTime_H
#define AV_Fields_Constraints_DateTime_H

#include "php.h"
#include "../../helpers/av_structs.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_DateTime_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_DateTime___construct, 0, 0, 0)
ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_STRING, 0, "\'X-m-d\\\\TH:i:sP\'")
ZEND_ARG_OBJ_TYPE_MASK(0, timezone, DateTimeZone, MAY_BE_STRING | MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_DateTime, __construct);

static const zend_function_entry class_AV_Fields_Constraints_DateTime_methods[] = {ZEND_ME(AV_Fields_Constraints_DateTime, __construct, arginfo_class_AV_Fields_Constraints_DateTime___construct, ZEND_ACC_PUBLIC)
                                                                                       ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool av_validate_DateTime_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors);

/* Registration function */
void av_register_DateTime_class(void);

#endif /* AV_Fields_Constraints_DateTime_H */
