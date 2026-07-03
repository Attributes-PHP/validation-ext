/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1a768f529f154ef2aa28f920a33990a45c4b34b0 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Attributes_Validation_ModelConfigs___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alias, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strToLower, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strToUpper, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stripWhitespaces, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extra, IS_STRING, 0, "\"ignore\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopAtFirstError, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()


ZEND_METHOD(Attributes_Validation_ModelConfigs, __construct);


static const zend_function_entry class_Attributes_Validation_Options_Alias_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Attributes_Validation_ModelConfigs_methods[] = {
	ZEND_ME(Attributes_Validation_ModelConfigs, __construct, arginfo_class_Attributes_Validation_ModelConfigs___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Attributes_Validation_Options_Alias(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Attributes\\Validation\\Options\\Alias", IS_STRING, class_Attributes_Validation_Options_Alias_methods);

	zval enum_case_SNAKE_value;
	zend_string *enum_case_SNAKE_value_str = zend_string_init("case", sizeof("case") - 1, 1);
	ZVAL_STR(&enum_case_SNAKE_value, enum_case_SNAKE_value_str);
	zend_enum_add_case_cstr(class_entry, "SNAKE", &enum_case_SNAKE_value);

	return class_entry;
}

static zend_class_entry *register_class_Attributes_Validation_ModelConfigs(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation", "ModelConfigs", class_Attributes_Validation_ModelConfigs_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
