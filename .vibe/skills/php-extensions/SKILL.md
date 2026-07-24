---
name: php-extensions
description: LLM skill to build PHP extensions
author: Attributes-PHP
tags:
  - php
  - extensions
  - c
  - development
---  

# PHP Extension Builder Skill

This skill provides tools and guidance for building PHP extensions in C.

## When to Use

Use this skill when you need to:
- Create new PHP extensions
- Modify existing PHP extensions
- Understand PHP extension architecture
- Compile and test PHP extensions

## Getting Started

### Prerequisites
- PHP development environment
- C compiler (GCC, Clang, or MSVC)
- php-dev or php-devel package
- autoconf, automake, libtool (for Unix-like systems)

### Basic Extension Structure

A minimal PHP extension requires:

```
ext/myextension/
├── config.m4              # Autoconf configuration
├── php_myextension.h     # Header file
├── myextension.c         # Main implementation
└── tests/                 # Test cases
```

### Creating a Simple Function

1. **Define the function in C:**
```c
#include "php.h"

PHP_FUNCTION(hello_world)
{
    php_printf("Hello, World!\n");
    RETURN_TRUE;
}
```

2. **Register the function:**
```c
static const zend_function_entry myextension_functions[] = {
    PHP_FE(hello_world, NULL)
    PHP_FE_END
};

zend_module_entry myextension_module_entry = {
    STANDARD_MODULE_HEADER,
    "myextension",
    myextension_functions,
    NULL, /* Module startup */
    NULL, /* Module shutdown */
    NULL, /* Request startup */
    NULL, /* Request shutdown */
    NULL, /* Module info */
    "1.0",
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_MYEXTENSION
ZEND_GET_MODULE(myextension)
#endif
```

3. **Create config.m4:**
```m4
PHP_ARG_ENABLE(myextension, whether to enable myextension support,
[  --enable-myextension      Enable myextension support])

if test "$PHP_MYEXTENSION" != "no"; then
    PHP_REQUIRE_CC()
    PHP_ADD_LIBRARY(stdc++, 1, MYEXTENSION_SHARED_LIBADD)
    PHP_NEW_EXTENSION(myextension, myextension.c, $ext_shared)
fi
```

## Compilation

### On Linux/macOS:
```bash
phpize
./configure --enable-myextension
make
sudo make install
```

### On Windows:
```bash
# Use the appropriate Visual Studio command prompt
phpize
configure --enable-myextension
nmake
nmake install
```

## Testing

Create a PHP script:
```php
<?php
if (!extension_loaded('myextension')) {
    dl('myextension.so') or die('Cannot load extension');
}
hello_world();
```

## Common Patterns

### Accepting Parameters
```c
PHP_FUNCTION(greet)
{
    char *name = NULL;
    size_t name_len;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(name, name_len)
    ZEND_PARSE_PARAMETERS_END();
    
    php_printf("Hello, %s!\n", name);
    RETURN_TRUE;
}
```

### Returning Values
```c
PHP_FUNCTION(add_numbers)
{
    zend_long a, b;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(a)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_LONG(a + b);
}
```

### Working with Arrays
```c
PHP_FUNCTION(array_double)
{
    zval *arr;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(arr)
    ZEND_PARSE_PARAMETERS_END();
    
    zend_ulong index;
    zval *value;
    
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), index, value) {
        convert_to_long(value);
        ZEND_HASH_UPDATE(Z_ARRVAL_P(arr), index, value);
    } ZEND_HASH_FOREACH_END();
    
    RETURN_ZVAL(arr, 1, 0);
}
```

## Resources

- [PHP Internals Book](https://www.phpinternalsbook.com/)
- [PHP Source Code](https://github.com/php/php-src)
- [PHP Extension Writing Guide](https://www.php.net/manual/en/internals2.php)
- [EXT Skeletons](https://github.com/php/ext-skeleton)

---

*Skill based on Attributes-PHP/php-extension-skill*
