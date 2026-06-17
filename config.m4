PHP_ARG_ENABLE(validation_ext, whether to enable validation_ext support,
[  --enable-validation_ext   Enable validation_ext support])

if test "$PHP_VALIDATION_EXT" != "no"; then
  PHP_REQUIRE_CC()
  PHP_ADD_LIBRARY(stdc++, 1, VALIDATION_EXT_SHARED_LIBADD)
  
  # Check for Zig compiler and compile Zig code during configure
  AC_PATH_PROG([ZIG], [zig])
  if test -n "$ZIG" && test -f "src/validation.zig"; then
    AC_MSG_CHECKING([for Zig compilation])
    if $ZIG build-obj -O ReleaseSafe -o src/validation.o src/validation.zig -lc 2>/dev/null; then
      AC_MSG_RESULT([yes])
      VALIDATION_EXT_SOURCES="validation_ext.c src/validation.o"
    else
      AC_MSG_RESULT([no])
      AC_MSG_WARN([Zig compilation failed, falling back to C-only implementation])
      rm -f src/validation.o
      VALIDATION_EXT_SOURCES="validation_ext.c"
    fi
  else
    AC_MSG_WARN([Zig compiler not found or source missing, using C-only implementation])
    VALIDATION_EXT_SOURCES="validation_ext.c"
  fi
  
  PHP_NEW_EXTENSION(validation_ext, $VALIDATION_EXT_SOURCES, $ext_shared)
fi
