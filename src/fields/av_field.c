#include "av_field.h"

zend_class_entry *AV_Fields_Field_ce;

void av_register_Field_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Field", class_AV_Fields_Field_methods);
    AV_Fields_Field_ce = zend_register_internal_interface(&ce);
}