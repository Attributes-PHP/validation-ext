PHP_ARG_ENABLE(attributes_validation, whether to enable attributes_validation support,
[  --enable-attributes-validation   Enable attributes_validation support])

if test "$PHP_ATTRIBUTES_VALIDATION" != "no"; then
  PHP_REQUIRE_CC()
  PHP_ADD_LIBRARY(stdc++, 1, ATTRIBUTES_VALIDATION_SHARED_LIBADD)
  PHP_NEW_EXTENSION(attributes_validation, attributes_validation.c src/validate_function.c src/call_function.c src/base_model.c src/arrayof.c src/exception.c src/model_configs.c src/helpers/options.c, $ext_shared)
fi
