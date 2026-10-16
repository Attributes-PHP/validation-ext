PHP_ARG_ENABLE(attributes_validation, whether to enable attributes_validation support,
[  --enable-attributes-validation   Enable attributes_validation support])

if test "$PHP_ATTRIBUTES_VALIDATION" != "no"; then
  PHP_REQUIRE_CC()
  
  # Require PHP 8.2 or later (ZEND_MODULE_API_NO 20220829)
  if test "$PHP_API_VERSION" -lt 20220829; then
    AC_MSG_ERROR([PHP 8.2 or later is required])
  fi

  dnl The Regex constraint compiles patterns through ext/pcre, whose
  dnl php_pcre.h includes pcre2.h. Package-managed PHP builds (Homebrew,
  dnl some Linux distributions) install pcre2.h outside the PHP include
  dnl tree, so probe the usual locations.
  AC_MSG_CHECKING([for pcre2.h])
  AV_PCRE2_INCLUDE=""
  for AV_DIR in "$PHP_PREFIX/include" "$PHP_PREFIX/../include" /usr/include /usr/local/include /opt/homebrew/include; do
    if test -f "$AV_DIR/pcre2.h"; then
      AV_PCRE2_INCLUDE="$AV_DIR"
      break
    fi
  done
  if test -z "$AV_PCRE2_INCLUDE"; then
    AC_MSG_ERROR([pcre2.h not found; install the PCRE2 development headers])
  fi
  AC_MSG_RESULT([$AV_PCRE2_INCLUDE])
  PHP_ADD_INCLUDE([$AV_PCRE2_INCLUDE])
  
  PHP_ADD_LIBRARY(stdc++, 1, ATTRIBUTES_VALIDATION_SHARED_LIBADD)
  PHP_NEW_EXTENSION(attributes_validation, attributes_validation.c src/av_validate_function.c src/av_compiled_model.c src/av_call_function.c src/av_base_model.c src/av_exception.c src/av_model_configs.c src/av_type.c src/helpers/av_wrappers.c src/helpers/av_options.c src/helpers/av_string.c src/helpers/av_error_messages.c src/fields/av_field.c src/fields/av_alias.c src/fields/av_error_message.c src/fields/av_sequence.c src/fields/av_union.c src/fields/av_intersection.c src/fields/av_dict.c src/fields/constraints/av_alpha_dash.c src/fields/constraints/av_ascii.c src/fields/constraints/av_between.c src/fields/constraints/av_contains.c src/fields/constraints/av_datetime.c src/fields/constraints/av_ends_with.c src/fields/constraints/av_in.c src/fields/constraints/av_max.c src/fields/constraints/av_min.c src/fields/constraints/av_multiple_of.c src/fields/constraints/av_regex.c src/fields/constraints/av_size.c src/fields/constraints/av_starts_with.c src/fields/constraints/av_ulid.c src/validators/av_typehint_validator.c src/validators/av_array_typehint_validator.c, $ext_shared)
fi
