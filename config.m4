PHP_ARG_ENABLE(validation_ext, whether to enable validation_ext support,
[  --enable-validation_ext   Enable validation_ext support])

if test "$PHP_VALIDATION_EXT" != "no"; then
  PHP_REQUIRE_CC()
  PHP_ADD_LIBRARY(stdc++, 1, VALIDATION_EXT_SHARED_LIBADD)
  PHP_NEW_EXTENSION(validation_ext, validation_ext.c src/validate_function.c src/call_function.c src/base_model.c src/arrayof.c src/exception.c src/options.c, $ext_shared)
fi
