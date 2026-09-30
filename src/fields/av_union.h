#ifndef AV_FIELDS_Union_H
#define AV_FIELDS_Union_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Union_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Union___construct, 0, 0, 0)
ZEND_ARG_VARIADIC_INFO(0, of)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Union, __construct);

static const zend_function_entry class_AV_Fields_Union_methods[] = {ZEND_ME(AV_Fields_Union, __construct, arginfo_class_AV_Fields_Union___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void av_register_Union_class(void);

#endif /* AV_FIELDS_Union_H */
