#include "src/base_model.h"
#include "Zend/zend_API.h"

zend_class_entry *validation_ext_BaseModel_ce;

/* Object fetch */
static inline validation_ext_BaseModel_object *validation_ext_BaseModel_fetch_object(zend_object *obj) {
    return (validation_ext_BaseModel_object *)((char *)obj - XtOffsetOf(validation_ext_BaseModel_object, std));
}

static zend_object_handlers validation_ext_BaseModel_handlers;

/* Method entries */
static zend_function_entry validation_ext_BaseModel_methods[] = {
    ZEND_ME(BaseModel, beforeValidation, arginfo_BaseModel_beforeValidation, ZEND_ACC_PUBLIC)
    ZEND_ME(BaseModel, afterValidation, arginfo_BaseModel_afterValidation, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

/* Object creation and destruction */
static zend_object *validation_ext_BaseModel_create_object(zend_class_entry *ce)
{
    validation_ext_BaseModel_object *obj = (validation_ext_BaseModel_object *)zend_object_alloc(sizeof(validation_ext_BaseModel_object), ce);
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    return &obj->std;
}

static void validation_ext_BaseModel_free_object(zend_object *object)
{
    validation_ext_BaseModel_object *obj = validation_ext_BaseModel_fetch_object(object);
    zend_object_std_dtor(&obj->std);
}

/* Method implementations */
PHP_METHOD(BaseModel, beforeValidation)
{
    // Empty implementation for now
}

PHP_METHOD(BaseModel, afterValidation)
{
    // Empty implementation for now
}

/* Registration function */
void register_BaseModel_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\BaseModel", validation_ext_BaseModel_methods);
    ce.create_object = validation_ext_BaseModel_create_object;
    
    validation_ext_BaseModel_ce = zend_register_internal_class(&ce);
    
    /* Setup object handlers */
    memcpy(&validation_ext_BaseModel_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    validation_ext_BaseModel_handlers.offset = XtOffsetOf(validation_ext_BaseModel_object, std);
    validation_ext_BaseModel_handlers.free_obj = validation_ext_BaseModel_free_object;
    validation_ext_BaseModel_handlers.clone_obj = NULL;
    
    /* Mark BaseModel as abstract */
    validation_ext_BaseModel_ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
}
