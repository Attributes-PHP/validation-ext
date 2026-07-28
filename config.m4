PHP_ARG_ENABLE(attributes_validation, whether to enable attributes_validation support,
[  --enable-attributes-validation   Enable attributes_validation support])

if test "$PHP_ATTRIBUTES_VALIDATION" != "no"; then
  PHP_REQUIRE_CC()
  
  # Require PHP 8.2 or later (ZEND_MODULE_API_NO 20220829)
  if test "$PHP_API_VERSION" -lt 20220829; then
    AC_MSG_ERROR([PHP 8.2 or later is required])
  fi
  
  PHP_ADD_LIBRARY(stdc++, 1, ATTRIBUTES_VALIDATION_SHARED_LIBADD)
  PHP_NEW_EXTENSION(attributes_validation, attributes_validation.c src/validate_function.c src/call_function.c src/base_model.c src/arrayof.c src/exception.c src/model_configs.c src/helpers/options.c src/fields/field.c src/fields/alias.c, $ext_shared)
fi
