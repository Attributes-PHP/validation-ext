/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: acfa77643eada1ce54b37666624ffb09a7f8c382 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Attributes_Validation_validate, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, rawData, IS_ARRAY, 0)
	ZEND_ARG_OBJ_INFO(0, model, Attributes\\Validation\\BaseModel, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, stopAtFirstError, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()


ZEND_FUNCTION(validate);


static const zend_function_entry ext_functions[] = {
	ZEND_NS_FE("Attributes\\Validation", validate, arginfo_Attributes_Validation_validate)
	ZEND_FE_END
};
