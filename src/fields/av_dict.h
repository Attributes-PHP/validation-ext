#ifndef AV_FIELDS_Dict_H
#define AV_FIELDS_Dict_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Dict_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_AV_Fields_Dict___construct, 0, 0, 2)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, of)
ZEND_END_ARG_INFO()

ZEND_METHOD(AV_Fields_Dict, __construct);

static const zend_function_entry class_AV_Fields_Dict_methods[] = {ZEND_ME(AV_Fields_Dict, __construct, arginfo_class_AV_Fields_Dict___construct, ZEND_ACC_PUBLIC) ZEND_FE_END};

/* Registration function */
void av_register_Dict_class(void);

#endif /* AV_FIELDS_Dict_H */
