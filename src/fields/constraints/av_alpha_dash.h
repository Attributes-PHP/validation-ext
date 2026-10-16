#ifndef AV_Fields_Constraints_AlphaDash_H
#define AV_Fields_Constraints_AlphaDash_H

#include "php.h"
#include "../../helpers/av_structs.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_AlphaDash_ce;

static const zend_function_entry class_AV_Fields_Constraints_AlphaDash_methods[] = {ZEND_FE_END};

/* Validation logic: returns true when the field value satisfies the
 * constraint, false after appending the constraint error message. */
bool av_validate_AlphaDash_constraint(zval *constraint, av_field *field, av_property_info *property, zval *errors);

/* Registration function */
void av_register_AlphaDash_class(void);

#endif /* AV_Fields_Constraints_AlphaDash_H */
