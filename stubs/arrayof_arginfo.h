/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: c631e4f0c77172133a99bba782be20a48fb6131e */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Attributes_Validation_ArrayOf___construct, 0, 0, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, type, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Attributes_Validation_ArrayOf, __construct);


static const zend_function_entry class_Attributes_Validation_ArrayOf_methods[] = {
	ZEND_ME(Attributes_Validation_ArrayOf, __construct, arginfo_class_Attributes_Validation_ArrayOf___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Attributes_Validation_ArrayOf(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation", "ArrayOf", class_Attributes_Validation_ArrayOf_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);

	zval property_type_default_value;
	ZVAL_UNDEF(&property_type_default_value);
	zend_string *property_type_name = zend_string_init("type", sizeof("type") - 1, 1);
	zend_declare_typed_property(class_entry, property_type_name, &property_type_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_type_name);

	return class_entry;
}
