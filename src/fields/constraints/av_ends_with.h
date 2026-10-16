#ifndef AV_Fields_Constraints_EndsWith_H
#define AV_Fields_Constraints_EndsWith_H

#include "php.h"
#include "../../helpers/av_structs.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_EndsWith_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_EndsWith___construct, 0, 0, 0)
/* Union-typed variadics need PHP 8.3 arginfo macros; untyped on 8.2 */
ZEND_ARG_VARIADIC_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_EndsWith, __construct);

static const zend_function_entry class_AV_Fields_Constraints_EndsWith_methods[] = {ZEND_ME(AV_Fields_Constraints_EndsWith, __construct, arginfo_class_AV_Fields_Constraints_EndsWith___construct, ZEND_ACC_PUBLIC)
                                                                                       ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool av_validate_EndsWith_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors);

/* Registration function */
void av_register_EndsWith_class(void);

#endif /* AV_Fields_Constraints_EndsWith_H */
