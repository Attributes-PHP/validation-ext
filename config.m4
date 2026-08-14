PHP_ARG_ENABLE(attributes_validation, whether to enable attributes_validation support,
[  --enable-attributes-validation   Enable attributes_validation support])

if test "$PHP_ATTRIBUTES_VALIDATION" != "no"; then
  PHP_REQUIRE_CC()
  
  # Require PHP 8.2 or later (ZEND_MODULE_API_NO 20220829)
  if test "$PHP_API_VERSION" -lt 20220829; then
    AC_MSG_ERROR([PHP 8.2 or later is required])
  fi
  
  PHP_ADD_LIBRARY(stdc++, 1, ATTRIBUTES_VALIDATION_SHARED_LIBADD)
  PHP_NEW_EXTENSION(attributes_validation, attributes_validation.c src/av_validate_function.c src/av_call_function.c src/av_base_model.c src/av_exception.c src/av_model_configs.c src/helpers/av_wrappers.c src/helpers/av_options.c src/helpers/av_string.c src/fields/av_field.c src/fields/av_alias.c src/validators/av_typehint_validator.c, $ext_shared)
fi
