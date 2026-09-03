#ifndef AV_STRUCTS_H
#define AV_STRUCTS_H

#include "php.h"

typedef struct {
    zend_string *parent;
    zend_string *name;
    zval *value;
} av_field;

typedef struct {
    zval *model;
    zend_class_entry *model_ce;
    zend_property_info *property;
} av_property_info;

#endif /* AV_STRUCTS_H */