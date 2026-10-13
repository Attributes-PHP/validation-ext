#ifndef AV_Fields_Constraints_Contains_H
#define AV_Fields_Constraints_Contains_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_Contains_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Constraints_Contains___construct, 0, 0, 1)
/* Union-typed variadics need PHP 8.3 arginfo macros; untyped on 8.2 */
ZEND_ARG_VARIADIC_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Constraints_Contains, __construct);

static const zend_function_entry class_AV_Fields_Constraints_Contains_methods[] = {ZEND_ME(AV_Fields_Constraints_Contains, __construct, arginfo_class_AV_Fields_Constraints_Contains___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void av_register_Contains_class(void);

#endif /* AV_Fields_Constraints_Contains_H */
