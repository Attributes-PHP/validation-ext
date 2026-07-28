#ifndef ATTRIBUTES_VALIDATION_FIELDS_Field_H
#define ATTRIBUTES_VALIDATION_FIELDS_Field_H

#include "php.h"

extern zend_class_entry *Attributes_Validation_Fields_Field_ce;

static const zend_function_entry class_Attributes_Validation_Fields_Field_methods[] = {
    ZEND_FE_END
};

/* Registration function */
void attributes_validation_register_Field_interface(void);

#endif /* ATTRIBUTES_VALIDATION_FIELDS_Alias_H */