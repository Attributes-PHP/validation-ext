#ifndef AV_Fields_Constraints_Size_H
#define AV_Fields_Constraints_Size_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_Size_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_Size___construct, 0, 0, 1)
ZEND_ARG_TYPE_MASK(0, value, MAY_BE_LONG | MAY_BE_DOUBLE, NULL)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_Size, __construct);

static const zend_function_entry class_AV_Fields_Constraints_Size_methods[] = {ZEND_ME(AV_Fields_Constraints_Size, __construct, arginfo_class_AV_Fields_Constraints_Size___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void av_register_Size_class(void);

#endif /* AV_Fields_Constraints_Size_H */
