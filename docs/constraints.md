---
title: "Constraints"
description: "Reference for the Attributes\Validation constraint attributes: the built-in constraints and how to write custom constraints in PHP userland."
---

# Constraints

Constraints are attributes that add value-level rules on top of the native type hints. They run after the property's type check succeeds (so they always see a value of the declared type, or the coerced value in loose mode) and before the value is written to the model — a failing constraint keeps the value out of the model.

```php
use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Constraints\Between;
use Attributes\Validation\Fields\Constraints\Regex;

class User extends BaseModel
{
    #[Between(18, 99)]
    public int $age;

    #[Regex('/^[a-z][a-z0-9-]*$/')]
    public string $username;
}
```

Constraints compose: every constraint declared on a property runs, and each failing one reports its own error under the resolved field name (the `Alias` name, when one is declared). `ModelConfigs(stopFirstError: true)` aborts the run at the first error, as with type errors.

Attribute constructor arguments are validated at construction — an argument rejected by the constructor (`#[Between('a', 'b')]`) throws a `TypeError` on the first `validate()` call and keeps throwing on every call until the attribute is fixed.

## Built-in constraints

All built-ins live in `Attributes\Validation\Fields\Constraints` and may be declared on properties and parameters.

| Attribute | Constructor | Fails when |
| --------- | ----------- | ---------- |
| `AlphaDash` | `()` | the string contains characters other than letters, numbers, dashes and underscores |
| `Ascii` | `()` | the string contains bytes outside 7-bit ASCII (0x00-0x7F) |
| `Between` | `(int\|float $min, int\|float $max)` | the number is outside the inclusive range |
| `Contains` | `(string\|int\|float ...$values)` | an array is missing one of the declared values, or a string is missing one of them as a substring |
| `DateTime` | `(string $format = 'X-m-d\TH:i:sP', DateTimeZone\|string\|null $timezone = null)` | the value does not parse in the declared format and timezone |
| `EndsWith` | `(string\|int\|float ...$values)` | the string ends with none of the declared values |
| `In` | `(string\|int\|float\|bool ...$values)` | the value matches none of the declared values |
| `Max` | `(int\|float $value)` | the number exceeds the inclusive upper bound |
| `Min` | `(int\|float $value)` | the number is below the inclusive lower bound |
| `MultipleOf` | `(int\|float $value)` | the number is not a multiple of the divisor |
| `Regex` | `(string $regex)` | the string does not match the pattern (`preg_match` semantics: a partial match passes) |
| `Size` | `(?int $min = null, ?int $max = null)` | the string length or array count is outside the inclusive bounds |
| `StartsWith` | `(string\|int\|float ...$values)` | the string starts with none of the declared values |
| `Ulid` | `()` | the string is not a valid ULID (26 Crockford base32 characters, either letter case) |

Named constructor arguments work everywhere: `#[Size(min: 3)]` validates only the lower bound, `#[DateTime(timezone: 'Europe/London')]` parses in that timezone.

### Value semantics

- **Numeric constraints** (`Between`, `Max`, `Min`, `MultipleOf`) apply to `int` and `float` values; any other type fails. `MultipleOf` compares integer pairs with integer modulo and float pairs with `fmod` plus a small tolerance, so `0.3` counts as a multiple of `0.1`. Only `0` is a multiple of `0`.
- **String constraints** (`AlphaDash`, `Ascii`, `Regex`, `StartsWith`, `EndsWith`, `Ulid`) fail for non-string values. `StartsWith`/`EndsWith` accept numeric declared values and match them by their string representation.
- **Membership constraints** (`In`, `Contains`) compare strictly: `In('1')` rejects the integer `1`. On arrays, `Contains` requires every declared value to be present; on strings, every declared value must appear as a substring.
- **`DateTime`** parses with the date extension in the declared format and timezone. `DateTimeInterface` instances pass as-is, and an invalid timezone name throws the `DateTimeZone` exception on the first `validate()` call.
- **`Size`** measures strings in bytes and arrays in element count. With only one bound declared, only that bound applies; with neither, every value passes.

### Error messages

Each constraint has a default message following the `Must be ...` pattern, with the constraint parameters rendered in place of `{expected}`:

```php
// ['age' => ['Must be between 18 and 99']]
// ['username' => ["Must match the pattern '/^[a-z][a-z0-9-]*$/'"]]
// ['size' => ['Must have a size of at least 3']
// ['tag' => ["Must be one of: 'a', 'b', true"]]
```

The `ErrorMessage` attribute only declares `required` and `type` templates, so constraint errors always use their default messages. Constraints applied through userland classes can return any message they want (see below).

## Custom constraints

Write a constraint in PHP userland by implementing `Attributes\Validation\Fields\Constraint`. Any attribute class implementing it is applied during `validate()` exactly like the built-ins — the same run order, error collection and `stopFirstError` handling. PHP enforces the interface methods and their signatures at class declaration time.

```php
use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Constraint;

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER)]
final class Even implements Constraint
{
    public function validate(mixed $value): bool
    {
        return is_int($value) && $value % 2 === 0;
    }

    public function message(): string
    {
        return 'Must be an even number';
    }
}

class Product extends BaseModel
{
    #[Even]
    public int $quantity;
}

validate(['quantity' => 3], new Product());
// ValidationException: ['quantity' => ['Must be an even number']]
```

The contract:

- `validate(mixed $value): bool` receives the value after the native type check and returns whether it satisfies the constraint.
- `message(): string` is called only on failure and returns the error message. It supports the `{field}` and `{value}` placeholders, substituted the same way as in built-in templates.
- Constructor arguments are passed through unchanged, named arguments included: `#[LessThan(bound: 10)]` reaches the constructor as a named parameter.
- Exceptions thrown inside `validate()` or `message()` propagate to the caller of `validate()`, like model hook errors.

The built-in constraint classes are final: custom constraints implement the `Constraint` interface rather than extending a built-in.

Unlike the built-ins, userland constraints may declare `Attribute::IS_REPEATABLE` and be declared multiple times on one property. Instances are compiled once per property and reused for every `validate()` call of the request.

Attribute classes that do not exist or do not implement `Constraint` are ignored, exactly like any other attribute the validation engine does not know — a property can carry unrelated documentation attributes without affecting validation.
