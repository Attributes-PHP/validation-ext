#include "av_config_aware_constraint.h"
#include "av_constraint.h"
#include "Zend/zend_API.h"

zend_class_entry *AV_Fields_ConfigAwareConstraint_ce;

void av_register_ConfigAwareConstraint_interface(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "ConfigAwareConstraint", class_AV_Fields_ConfigAwareConstraint_methods);
    AV_Fields_ConfigAwareConstraint_ce = zend_register_internal_interface(&ce);
    ZEND_ASSERT(AV_Fields_ConfigAwareConstraint_ce != NULL);

    ZEND_ASSERT(AV_Fields_Constraint_ce != NULL);
    zend_class_implements(AV_Fields_ConfigAwareConstraint_ce, 1, AV_Fields_Constraint_ce);
}
