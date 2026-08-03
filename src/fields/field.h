#ifndef AV_FIELDS_Field_H
#define AV_FIELDS_Field_H

#include "php.h"

extern zend_class_entry *AV_Fields_Field_ce;

static const zend_function_entry class_AV_Fields_Field_methods[] = {
    ZEND_FE_END
};

/* Registration function */
void av_register_Field_interface(void);

#endif /* AV_FIELDS_Alias_H */