#ifndef AV_FIELDS_Intersection_H
#define AV_FIELDS_Intersection_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Intersection_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Intersection___construct, 0, 0, 0)
ZEND_ARG_VARIADIC_INFO(0, of)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Intersection, __construct);

static const zend_function_entry class_AV_Fields_Intersection_methods[] = {ZEND_ME(AV_Fields_Intersection, __construct, arginfo_class_AV_Fields_Intersection___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void av_register_Intersection_class(void);

#endif /* AV_FIELDS_Intersection_H */
