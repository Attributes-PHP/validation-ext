# Architecture Overview

This document describes the architecture of Attributes Validation Extension, a PHP C extension that provides Pydantic-inspired data validation using PHP 8.2+ attributes and type hints.

## Table of Contents

- [High-Level Architecture](#high-level-architecture)
- [Component Overview](#component-overview)
- [Validation Pipeline](#validation-pipeline)
- [C/PHP Interaction](#cphp-interaction)
- [Memory Management](#memory-management)
- [Error Handling](#error-handling)
- [Type System](#type-system)

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        PHP Userland                            │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │  User Model     │  │  validate()      │  │  call()      │ │
│  │  extends         │  │  function        │  │  function    │ │
│  │  BaseModel      │  │                 │  │              │ │
│  └────────┬────────┘  └────────┬────────┘  └──────┬──────┘ │
└───────────┼─────────────────────┼─────────────────────┼─────────┘
            │                     │                     │
            ▼                     ▼                     ▼
┌─────────────────────────────────────────────────────────────┐
│                      C Extension Layer                          │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │                    PHP Zend Engine                         │ │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │ │
│  │  │ BaseModel   │  │ validate()   │  │ call_function()  │  │ │
│  │  │ Class       │  │ ZEND_FUNCTION │  │ ZEND_FUNCTION    │  │ │
│  │  └─────────────┘  └─────────────┘  └─────────────────┘  │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │                    Validation Logic                         │ │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │ │
│  │  │ validate_   │  │ get_property │  │ validate_field_  │  │ │
│  │  │ field_value│  │    _name     │  │ value            │  │ │
│  │  └─────────────┘  └─────────────┘  └─────────────────┘  │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │                    Configuration                           │ │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │ │
│  │  │ ModelConfigs │  │  Alias       │  │  Field           │  │ │
│  │  │ Attribute    │  │  Attribute   │  │  Interface       │  │ │
│  │  └─────────────┘  └─────────────┘  └─────────────────┘  │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Component Overview

### 1. PHP Userland Components

#### BaseModel
- **File**: `stubs/base_model.stub.php`
- **Purpose**: Abstract base class for all validation models
- **Methods**:
  - `beforeValidation(array $rawData, ModelConfigs $configs): array` - Hook called before validation
  - `afterValidation(array $rawData, ModelConfigs $configs): void` - Hook called after validation

#### Functions
- **File**: `stubs/functions.stub.php`
- **Purpose**: Global validation functions exposed to PHP
- **Functions**:
  - `validate(array $rawData, BaseModel $model): BaseModel` - Validates and transforms data
  - `call(string|callable $function, array $params, array $dependencies = []): mixed` - Calls function with validated parameters

#### Attributes
- **File**: `stubs/model_configs.stub.php`, `stubs/fields/alias.stub.php`
- **Purpose**: PHP 8 attributes for configuration
- **Attributes**:
  - `#[ModelConfigs]` - Class-level configuration (aliasGenerator, strict mode, etc.)
  - `#[Alias]` - Property/parameter-level field name mapping

#### Exceptions
- **File**: `stubs/exceptions.stub.php`
- **Purpose**: Custom exception classes
- **Classes**:
  - `BaseException` - Base exception class
  - `ValidationException` - Exception thrown on validation failure with error details

### 2. C Extension Components

#### Core Files

| File | Purpose |
|------|---------|
| `attributes_validation.c` | Extension entry point and module registration |
| `attributes_validation.h` | Main header with module globals |
| `config.m4` | Autoconf configuration for building |

#### Validation Logic

| File | Purpose |
|------|---------|
| `src/validate_function.c` | Main validation function implementation |
| `src/validate_function.h` | Validation function declarations |
| `src/base_model.c` | BaseModel class implementation |
| `src/base_model.h` | BaseModel class declarations |

#### Configuration

| File | Purpose |
|------|---------|
| `src/model_configs.c` | ModelConfigs attribute implementation |
| `src/model_configs.h` | ModelConfigs declarations |

#### Fields

| File | Purpose |
|------|---------|
| `src/fields/field.c` | Field interface implementation |
| `src/fields/field.h` | Field interface declarations |
| `src/fields/alias.c` | Alias attribute implementation |
| `src/fields/alias.h` | Alias attribute declarations |

#### Exceptions

| File | Purpose |
|------|---------|
| `src/exception.c` | Exception classes implementation |
| `src/exception.h` | Exception declarations |

#### Helpers

| File | Purpose |
|------|---------|
| `src/helpers/av_string.c` | String manipulation utilities |
| `src/helpers/av_string.h` | String utilities declarations |
| `src/helpers/av_wrappers.c` | Mockable Zend wrapper functions |
| `src/helpers/av_wrappers.h` | Wrapper declarations |
| `src/helpers/options.c` | Option validation utilities |
| `src/helpers/options.h` | Options declarations |
| `src/helpers/testing.h` | Testing mode macros |

#### Call Function

| File | Purpose |
|------|---------|
| `src/call_function.c` | Function calling with validation |
| `src/call_function.h` | Call function declarations |

## Validation Pipeline

The validation process follows this pipeline:

```
1. Parse Parameters
   ├─ Validate rawData is an array
   └─ Validate model is a BaseModel instance

2. Create ModelConfigs
   ├─ Walk up inheritance chain
   ├─ Find ModelConfigs attribute
   ├─ Parse attribute arguments
   └─ Set default properties

3. Call beforeValidation Hook
   ├─ Call model->beforeValidation($rawData, $configs)
   └─ Optionally modify rawData

4. Initialize Errors Array

5. Iterate Through Properties
   ├─ For each class in inheritance chain (child to parent)
   │  └─ For each property in class
   │     ├─ Get property name (with alias support)
   │     ├─ Get property value from rawData
   │     │  ├─ Check if field exists in rawData
   │     │  └─ Check for default value
   │     ├─ If value is NULL
   │     │  ├─ If nullable, accept NULL
   │     │  └─ Otherwise, add error and continue
   │     ├─ Validate field value
   │     │  ├─ Check type hint
   │     │  │  ├─ If strict mode, check exact type match
   │     │  │  └─ Otherwise, attempt coercion
   │     │  ├─ Handle union types (TODO)
   │     │  └─ Handle custom validators (TODO)
   │     └─ If validation fails, add error
   │        ├─ If stopAtFirstError, throw exception
   │        └─ Otherwise, continue to next property
   └─ Set validated value on model object

6. Check for Errors
   ├─ If errors exist and not stopAtFirstError
   │  └─ Throw ValidationException with all errors

7. Call afterValidation Hook
   └─ Call model->afterValidation($rawData, $configs)

8. Return Validated Model
```

## C/PHP Interaction

### PHP to C Communication

1. **Function Calls**: PHP calls C functions through the Zend Engine
   - PHP: `validate($rawData, $model)` → C: `ZEND_FUNCTION(validate)`
   - PHP: `new BaseModel()` → C: `AV_BaseModel_ce` class entry

2. **Type Information**: Property type hints are available through `zend_property_info->type`
   - Contains `zend_type` which includes type mask and flags
   - Supports nullable types, union types (PHP 8.0+), etc.

3. **Attribute Access**: PHP attributes are accessible through `zend_attribute` API
   - `zend_get_attribute_str()` - Find attribute by name
   - `zend_get_attribute_value()` - Get attribute value

### Memory Management

The extension uses Zend Engine memory management:

1. **Allocation**:
   - `emalloc()` - Allocate memory (tracked by Zend MM)
   - `zend_string_init()` - Create a zend_string
   - `zval` allocation - Use stack or `emalloc()` for zval structures

2. **Release**:
   - `efree()` - Free memory allocated with emalloc
   - `zend_string_release()` - Release zend_string reference
   - `zval_ptr_dtor()` - Destroy zval and free memory

3. **Reference Counting**:
   - Zend uses reference counting for strings and arrays
   - `ZVAL_STRING()` increments refcount
   - `zend_hash_str_add()` increments refcount when adding to hash

### Ownership Tracking

The code tracks ownership of allocated strings to prevent double-free:

```c
zend_string *field_name = get_property_name(...);
bool field_name_owned = (field_name != property_name);

// Use field_name...

if (field_name_owned) {
    zend_string_release(field_name);
}
```

## Error Handling

### Error Collection

1. **Error Array**: Validation errors are collected in a `zval` array
2. **Error Format**: `['field_name' => 'error_message']`
3. **Error Types**:
   - Required field missing
   - Type mismatch (strict mode)
   - Type coercion failure
   - Custom validation failure (TODO)

### Exception Throwing

1. **ValidationException**: Thrown when validation fails
   - Contains all collected errors
   - Can be caught in PHP for custom error handling

2. **Throw Mechanism**:
   ```c
   av_throw_validation_exception(&errors);
   ```

3. **Exception Classes**:
   - `BaseException` - Extends PHP `Exception`
   - `ValidationException` - Extends `BaseException`, contains error array

## Type System

### Type Representation

PHP types are represented using Zend Engine types:

| PHP Type | Zend Type | Type Mask |
|----------|-----------|-----------|
| int | IS_LONG | MAY_BE_LONG |
| float | IS_DOUBLE | MAY_BE_DOUBLE |
| string | IS_STRING | MAY_BE_STRING |
| bool | IS_BOOL | MAY_BE_BOOL |
| array | IS_ARRAY | MAY_BE_ARRAY |
| object | IS_OBJECT | MAY_BE_OBJECT |
| null | IS_NULL | MAY_BE_NULL |

### Type Coercion

The extension supports automatic type coercion in non-strict mode:

| From \ To | int | float | string | bool | array |
|-----------|-----|-------|--------|------|-------|
| int | ✓ | ✓ | ✓ | ✓ | ✗ |
| float | ✗ | ✓ | ✓ | ✓ | ✗ |
| string | ✓ | ✓ | ✓ | ✓ | ✗ |
| bool | ✗ | ✗ | ✗ | ✓ | ✗ |
| array | ✗ | ✗ | ✗ | ✗ | ✓ |
| object | ✗ | ✗ | ✗ | ✗ | ✓ |
| null | ✗ | ✗ | ✗ | ✗ | ✗ |

### Nullable Types

Nullable types are represented with the `ZEND_TYPE_NULLABLE` flag:

```c
zend_type property_type = prop_info->type;
bool is_nullable = (property_type & ZEND_TYPE_NULLABLE) != 0;
property_type &= ~ZEND_TYPE_NULLABLE; // Remove nullable flag

if (Z_TYPE_P(value) == IS_NULL && is_nullable) {
    // NULL is valid for nullable types
}
```

## Build System

### Ceedling

The project uses [Ceedling](https://www.throwtheswitch.org/ceedling) for C testing:

- **Configuration**: `project.yml`
- **Test Files**: `tests/c/`
- **Source Files**: `src/`
- **Commands**:
  - `ceedling test:all` - Run all tests
  - `ceedling build` - Build the project
  - `ceedling clean` - Clean build artifacts

### PHP Build

Standard PHP extension build process:

```bash
phpize         # Generate configure script
./configure    # Configure the extension
make           # Build the extension
make install   # Install the extension
```

### Configuration

- **config.m4**: Autoconf configuration
  - Checks for PHP version (8.2+)
  - Registers source files
  - Sets up extension metadata

- **phpize**: Generates configure script from config.m4

## Future Enhancements

The following features are planned but not yet implemented:

1. **Union Type Support**: Handle `int|string` type hints
2. **Array Validation**: Support `#[ArrayOf(Type)]` attribute
3. **Custom Validators**: Support custom validation rules
4. **Nested Models**: Recursive validation of nested BaseModel objects
5. **DateTime Support**: Special handling for DateTime types
6. **Length Validation**: Support `#[Length(min, max)]` attribute
7. **Sensitive Parameters**: Support `#[SensitiveParameter]` attribute

## Performance Considerations

1. **Inlining**: Use `zend_always_inline` for hot paths
2. **Memory**: Minimize allocations in validation loop
3. **Caching**: Cache attribute lookups where possible
4. **String Operations**: Use `zend_string` for efficient string handling

## Directory Structure

```
Attributes-PHP/validation-ext/
├── src/                          # C source files
│   ├── attributes_validation.c    # Extension entry point
│   ├── validate_function.c       # Main validation logic
│   ├── base_model.c              # BaseModel implementation
│   ├── model_configs.c           # ModelConfigs implementation
│   ├── exception.c               # Exception classes
│   ├── call_function.c           # Function calling logic
│   └── helpers/                  # Helper utilities
│       ├── av_string.c           # String manipulation
│       ├── av_wrappers.c         # Zend wrappers
│       └── options.c             # Option validation
├── stubs/                       # PHP stub files
│   ├── base_model.stub.php       # BaseModel stub
│   ├── functions.stub.php       # Function stubs
│   ├── exceptions.stub.php      # Exception stubs
│   ├── model_configs.stub.php    # ModelConfigs stub
│   └── fields/                  # Field stubs
│       ├── field.stub.php        # Field interface
│       └── alias.stub.php        # Alias attribute
├── tests/                       # Test files
│   ├── php/                     # PHP tests
│   │   └── tests/               # Pest tests
│   │       ├── ValidateFunctionTest.php
│   │       ├── BaseModelTest.php
│   │       ├── AliasAttributeTest.php
│   │       ├── CallFunctionTest.php
│   │       ├── ExceptionTest.php
│   │       └── HooksTest.php
│   └── c/                       # C tests
│       ├── test_string.c        # String helper tests
│       └── test_validate_function.c  # Validation tests
├── project.yml                  # Ceedling configuration
├── config.m4                   # Autoconf configuration
├── composer.json               # PHP dependencies
├── README.md                   # Project documentation
└── LICENSE                     # License file
```
