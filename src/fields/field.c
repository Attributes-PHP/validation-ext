#include "field.h"

zend_class_entry *Attributes_Validation_Fields_Field_ce;

void attributes_validation_register_Field_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Field", class_Attributes_Validation_Fields_Field_methods);
    Attributes_Validation_Fields_Field_ce = zend_register_internal_interface(&ce);
}