# Contributing to Attributes Validation Extension

Thank you for considering contributing to Attributes Validation Extension! This document provides guidelines for contributing to the project.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Running Tests](#running-tests)
- [Code Style](#code-style)
- [Pull Request Guidelines](#pull-request-guidelines)
- [Reporting Issues](#reporting-issues)

## Getting Started

Attributes Validation Extension is a PHP C extension that provides Pydantic-inspired data validation using PHP 8.2+ attributes and type hints.

## Development Setup

### Prerequisites

- PHP 8.2 or later (8.3+ recommended for full attribute support)
- PHP development headers and tools (`phpize`, `php-config`)
- Git
- Composer
- Make
- GCC or Clang

### Install Dependencies

```bash
# Clone the repository
git clone https://github.com/Attributes-PHP/validation-ext.git
cd validation-ext

# Install PHP dependencies
composer install
```

### Build the Extension

```bash
# Generate configure script
phpize

# Configure the extension
./configure --enable-attributes-validation

# Build the extension
make

# Install the extension (optional)
sudo make install
```

### Enable the Extension

Add the following line to your `php.ini`:

```ini
extension=attributes_validation.so
```

Or load it dynamically in your PHP scripts:

```php
<?php
dl('attributes_validation.so');
```

## Running Tests

### PHP Tests (Pest)

The project uses [Pest](https://pestphp.com/) for PHP testing.

```bash
# Run all PHP tests
composer test:integration

# Run with coverage
composer test:coverage

# Run linting
composer lint
```

### C Tests (Ceedling)

The project uses [Ceedling](https://www.throwtheswitch.org/ceedling) for C testing.

```bash
# Run all C tests
composer test:unit

# Run specific test
ceedling test:test_validate_function

# Clean build artifacts
composer clean:test
```

### Full Test Suite

```bash
# Run all tests (PHP + C)
composer test
```

## Code Style

### PHP Code

- Follow [PSR-12](https://www.php-fig.org/psr/psr-12/) coding standard
- Use Laravel Pint for code style checking:

```bash
composer lint
```

### C Code

- Use `zend_always_inline` for performance-critical functions
- Use `static inline` for less critical inline functions
- Prefix internal functions with `av_` (e.g., `av_validate_type`)
- Use Zend Engine memory management functions (`emalloc`, `efree`, etc.)
- Always check for exceptions after Zend API calls using `EG(exception)`
- Use `UNEXPECTED()` macro for error conditions

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| PHP Classes | PascalCase | `BaseModel`, `ValidationException` |
| PHP Methods | camelCase | `beforeValidation()`, `afterValidation()` |
| PHP Properties | camelCase or snake_case | `$age`, `$first_name` |
| C Functions | snake_case with `av_` prefix | `av_validate_type()`, `av_get_property_name()` |
| C Macros | UPPER_SNAKE_CASE | `AV_PASCAL_CASE`, `AV_BaseModel_ce` |
| C Types | snake_case with `av_` prefix | `av_model_configs_properties` |

## Pull Request Guidelines

1. **Fork the repository** and create a feature branch from `main`
2. **Use descriptive branch names**: `feature/xxx`, `fix/xxx`, `docs/xxx`, etc.
3. **Keep PRs focused**: One feature or fix per PR
4. **Write tests**: Add tests for new functionality or bug fixes
5. **Update documentation**: Update README, docs, or comments as needed
6. **Follow code style**: Ensure your code follows the project's style guidelines
7. **Write clear commit messages**: Use imperative mood, keep first line under 50 chars
8. **Reference issues**: Use `Closes #xxx` or `Fixes #xxx` in PR description

### Commit Message Format

```
Type: Short description (50 chars or less)

Longer description if needed (72 chars per line)

- Bullet points for additional details
- Use imperative mood ("Fix" not "Fixed")

Closes #123
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

### PR Title Format

- Use imperative mood
- Keep under 72 characters
- Capitalize first letter
- No period at the end

Examples:
- "Fix memory leaks in validate_function.c"
- "Add comprehensive C unit tests"
- "Implement core validation logic"

## Reporting Issues

When reporting issues, please include:

1. **PHP version** (`php -v`)
2. **Operating system**
3. **Steps to reproduce**
4. **Expected behavior**
5. **Actual behavior**
6. **Code examples** (if applicable)
7. **Error messages** (if any)

### Issue Template

```markdown
## Description

[Describe the issue]

## Environment

- PHP Version: [e.g., 8.2.32]
- OS: [e.g., Ubuntu 22.04, macOS 14]
- Extension Version: [if known]

## Steps to Reproduce

1. [First step]
2. [Second step]
3. [Third step]

## Expected Behavior

[What should happen]

## Actual Behavior

[What actually happens]

## Additional Context

[Any other relevant information]
```

## Code of Conduct

By participating in this project, you agree to abide by the [Code of Conduct](CODE_OF_CONDUCT.md).

## License

By contributing to this project, you agree that your contributions will be licensed under the [MIT License](LICENSE).
