/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3e1e6a6c39d56a8a1b29f89f11d9f96c20641855 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Attributes_Validation_call, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_MASK(0, function, MAY_BE_STRING|MAY_BE_CALLABLE, NULL)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dependencies, IS_ARRAY, 0, "[]")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()


ZEND_FUNCTION(call);


static const zend_function_entry ext_functions[] = {
	ZEND_NS_FE("Attributes\\Validation", call, arginfo_Attributes_Validation_call)
	ZEND_FE_END
};
