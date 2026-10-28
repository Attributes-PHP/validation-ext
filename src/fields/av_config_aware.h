#ifndef AV_FIELDS_ConfigAware_H
#define AV_FIELDS_ConfigAware_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_ConfigAware_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_AV_Fields_ConfigAware_setModelConfigs, 0, 1, IS_VOID, 0)
ZEND_ARG_OBJ_INFO(0, configs, Attributes\\Validation\\ModelConfigs, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry class_AV_Fields_ConfigAware_methods[] = {
    ZEND_ABSTRACT_ME_WITH_FLAGS(AV_Fields_ConfigAware, setModelConfigs, arginfo_class_AV_Fields_ConfigAware_setModelConfigs, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT) ZEND_FE_END
};

/* Registration function */
void av_register_ConfigAware_interface(void);

#endif /* AV_FIELDS_ConfigAware_H */
