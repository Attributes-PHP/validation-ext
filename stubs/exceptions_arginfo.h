/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 6d1b37deef2dc0236dea69e1c473809115c43407 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Attributes_Validation_Exceptions_ValidationException_getAllErrors, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Attributes_Validation_Exceptions_ValidationException, getAllErrors);


static const zend_function_entry class_Attributes_Validation_Exceptions_BaseException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Attributes_Validation_Exceptions_ValidationException_methods[] = {
	ZEND_ME(Attributes_Validation_Exceptions_ValidationException, getAllErrors, arginfo_class_Attributes_Validation_Exceptions_ValidationException_getAllErrors, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Attributes_Validation_Exceptions_BaseException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Exceptions", "BaseException", class_Attributes_Validation_Exceptions_BaseException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Attributes_Validation_Exceptions_ValidationException(zend_class_entry *class_entry_Attributes_Validation_Exceptions_BaseException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Exceptions", "ValidationException", class_Attributes_Validation_Exceptions_ValidationException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Attributes_Validation_Exceptions_BaseException);

	zval property_allErrors_default_value;
	ZVAL_UNDEF(&property_allErrors_default_value);
	zend_string *property_allErrors_name = zend_string_init("allErrors", sizeof("allErrors") - 1, 1);
	zend_declare_typed_property(class_entry, property_allErrors_name, &property_allErrors_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_allErrors_name);

	return class_entry;
}
