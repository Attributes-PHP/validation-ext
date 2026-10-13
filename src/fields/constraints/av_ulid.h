#ifndef AV_Fields_Constraints_Ulid_H
#define AV_Fields_Constraints_Ulid_H

#include "php.h"

/* Class entry */
extern zend_class_entry *AV_Fields_Constraints_Ulid_ce;

static const zend_function_entry class_AV_Fields_Constraints_Ulid_methods[] = {ZEND_FE_END};

/* Registration function */
void av_register_Ulid_class(void);

#endif /* AV_Fields_Constraints_Ulid_H */
