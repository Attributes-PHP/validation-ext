# validation_ext - PHP Extension

A PHP extension providing validation functionality under the `Attributes\Validation` namespace.

## Prerequisites

- PHP 8.1 or later
- PHP development headers and tools (phpize, php-config)
- GCC or Clang compiler
- GNU Make
- autoconf 2.68 or later
- libtool

### macOS

```bash
brew install php autoconf libtool
```

### Ubuntu/Debian

```bash
sudo apt-get install php-dev phpize autoconf libtool build-essential
```

### Fedora/RHEL

```bash
sudo dnf install php-devel autoconf libtool gcc gcc-c++
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

```bash
php -d extension=modules/validation_ext.so -r "Attributes\Validation\hello_world();"
php -d extension=modules/validation_ext.so -r "var_dump(function_exists('Attributes\Validation\hello_world'));"
```

Attributes Validation extension was created by **[André Gil](https://www.linkedin.com/in/andre-gil/)** and is open-sourced software licensed under the **[MIT license](https://opensource.org/licenses/MIT)**.