/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 088857659e289fd4d0b24873533fe0cebd61071a */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_BaseModel_beforeValidation, 0, 2, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
	ZEND_ARG_OBJ_INFO(0, options, Attributes\\Validation\\Options, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_BaseModel_afterValidation, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
	ZEND_ARG_OBJ_INFO(0, options, Attributes\\Validation\\Options, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Attributes_Validation_BaseModel, beforeValidation);
ZEND_METHOD(Attributes_Validation_BaseModel, afterValidation);


static const zend_function_entry class_Attributes_Validation_BaseModel_methods[] = {
	ZEND_ME(Attributes_Validation_BaseModel, beforeValidation, arginfo_class_Attributes_Validation_BaseModel_beforeValidation, ZEND_ACC_PUBLIC)
	ZEND_ME(Attributes_Validation_BaseModel, afterValidation, arginfo_class_Attributes_Validation_BaseModel_afterValidation, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Attributes_Validation_BaseModel(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation", "BaseModel", class_Attributes_Validation_BaseModel_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
