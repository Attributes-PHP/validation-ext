#include "src/arrayof.h"
#include "Zend/zend_API.h"

zend_class_entry *validation_ext_ArrayOf_ce;

/* Object fetch */
static inline validation_ext_ArrayOf_object *validation_ext_ArrayOf_fetch_object(zend_object *obj) {
    return (validation_ext_ArrayOf_object *)((char *)obj - XtOffsetOf(validation_ext_ArrayOf_object, std));
}

static zend_object_handlers validation_ext_ArrayOf_handlers;

/* Method entries */
static zend_function_entry validation_ext_ArrayOf_methods[] = {
    ZEND_ME(ArrayOf, __construct, arginfo_ArrayOf___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_FE_END
};

/* Object creation and destruction */
static zend_object *validation_ext_ArrayOf_create_object(zend_class_entry *ce)
{
    validation_ext_ArrayOf_object *obj = (validation_ext_ArrayOf_object *)zend_object_alloc(sizeof(validation_ext_ArrayOf_object), ce);
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    return &obj->std;
}

static void validation_ext_ArrayOf_free_object(zend_object *object)
{
    validation_ext_ArrayOf_object *obj = validation_ext_ArrayOf_fetch_object(object);
    zend_object_std_dtor(&obj->std);
}

/* Method implementations */
PHP_METHOD(ArrayOf, __construct)
{
    // Empty constructor for now - just need the class to exist
}

/* Registration function */
void register_ArrayOf_class()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Attributes\\Validation\\ArrayOf", validation_ext_ArrayOf_methods);
    ce.create_object = validation_ext_ArrayOf_create_object;
    
    validation_ext_ArrayOf_ce = zend_register_internal_class(&ce);
    
    /* Setup object handlers */
    memcpy(&validation_ext_ArrayOf_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    validation_ext_ArrayOf_handlers.offset = XtOffsetOf(validation_ext_ArrayOf_object, std);
    validation_ext_ArrayOf_handlers.free_obj = validation_ext_ArrayOf_free_object;
    validation_ext_ArrayOf_handlers.clone_obj = NULL;
}
