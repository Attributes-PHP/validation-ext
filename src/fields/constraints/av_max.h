#ifndef AV_Fields_Constraints_Max_H
#define AV_Fields_Constraints_Max_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_Max_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_Max___construct, 0, 0, 1)
ZEND_ARG_TYPE_MASK(0, value, MAY_BE_LONG | MAY_BE_DOUBLE, NULL)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_Max, __construct);

static const zend_function_entry class_AV_Fields_Constraints_Max_methods[] = {ZEND_ME(AV_Fields_Constraints_Max, __construct, arginfo_class_AV_Fields_Constraints_Max___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void av_register_Max_class(void);

#endif /* AV_Fields_Constraints_Max_H */
