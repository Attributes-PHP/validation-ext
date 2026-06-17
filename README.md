# Attributes Validation Extension

**Attributes Validation Extension** is the Pydantic validation library for PHP which allows you to validate data via type hints at speed of light ⚡️

## Prerequisites

- PHP 8.1 or later
- PHP development headers and tools (phpize, php-config)
- GCC or Clang compiler
- GNU Make
- autoconf 2.68 or later
- libtool

### macOS

```bash
brew install php autoconf libtool zig
```

### Ubuntu/Debian

```bash
sudo apt-get install php-dev phpize autoconf libtool build-essential zig
```

### Fedora/RHEL

```bash
sudo dnf install php-devel autoconf libtool gcc gcc-c++ zig
```

## Manual Build

```bash
phpize
./configure --enable-validation_ext
make
```

Optionally install:
```bash
sudo make install
```

## Testing

### Basic Functionality

```bash
php -d extension=modules/validation_ext.so -r "var_dump(class_exists('Attributes\Validation\Validator'));"
```

### PHPT Tests

Run the complete PHPT test suite:
```bash
php run-tests.php
```

## Clean

To clean all configure and build files:

```bash
make distclean
```

Validation extension was created by **[André Gil](https://www.linkedin.com/in/andre-gil/)** and is open-sourced software licensed under the **[MIT license](https://opensource.org/licenses/MIT)**.