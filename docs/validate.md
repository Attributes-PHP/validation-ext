---
title: "validate()"
description: "Reference for the Attributes\Validation\validate() function: parameters, return value, validation rules and exceptions."
---

# validate()

Validates a raw data array against the type hints and attributes of a `BaseModel` instance, and returns the hydrated model.

```php
namespace Attributes\Validation;

function validate(array $rawData, BaseModel $model): BaseModel
```

```php
use function Attributes\Validation\validate;

$user = validate($rawData, new User());
```

## Parameters

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `$rawData` | `array` | The input data, keyed by property name (or by [alias](#aliases)). Missing keys fail validation unless the property is nullable or declares a default value. |
| `$model` | `BaseModel` | The model instance to hydrate. Its public property type hints and attribute declarations define the validation schema. |

## Return value

Returns the passed `$model` instance with every property populated from `$rawData`. Values are written with the type declared on the property:

- Scalars are coerced to the declared type in [loose mode](#strict-and-loose-mode).
- `DateTimeInterface`-typed properties hydrate from date strings such as `1994-01-01T09:00:00+00:00`.
- Properties typed with a `BaseModel` class hydrate recursively from nested arrays.
- Union-typed properties keep the raw value's type when it already matches one of the arms; otherwise the first arm that can coerce it wins.

## Exceptions

| Exception | Thrown when |
| --------- | ----------- |
| `Attributes\Validation\Exceptions\ValidationException` | Any field fails validation. Call `getErrors()` on the exception for the error list. |
| `ValueError` | An attribute spec is malformed, for example an unresolvable class-name arm, an unsupported `Type` value in an array attribute, or a `Dict` key type outside integer and string. |

`ValidationException::getErrors()` returns an array keyed by field path, each value holding one or more messages:

```php
use Attributes\Validation\Exceptions\ValidationException;

try {
    validate($rawData, new User());
} catch (ValidationException $e) {
    $e->getErrors();
    // [
    //     'name' => ['Field is required'],
    //     'address.street' => ['Must be string'],
    // ]
}
```

Nested structures report dot-notation paths, including array element keys:

```php
// ['users.0.email' => ['Must be string'], 'scores.5' => ['Must be integer']]
```

## Validation rules

### Required fields

Every typed, non-nullable property is required. A missing key produces a `Field is required` error. Nullable properties accept a missing key or an explicit `null`, and properties with a declared default value keep it when the key is missing.

### Strict and loose mode

Validation is **strict by default**: a value must already have the declared type. Declare loose mode on the model to enable scalar coercion:

```php
use Attributes\Validation\ModelConfigs;

#[ModelConfigs(strict: false)]
class User extends BaseModel
{
    public int $age;
    public string $name;
}

validate(['age' => '30', 'name' => 123], new User())->age; // int(30)
// ->name is the string "123"
```

In loose mode:

- Exact type matches always win over coercion, regardless of declaration order.
- When coercion is needed on a union property, arms are tried in declaration order: `int|bool` coerces `'1'` to `int(1)`, while `bool|int` coerces it to `bool(true)`.

`DateTimeInterface` properties coerce from date strings in both modes.

### Aliases

Map an input key to a property with the `Alias` attribute:

```php
use Attributes\Validation\Fields\Alias;

class User extends BaseModel
{
    #[Alias('user_name')]
    public string $name;
}
```

`ModelConfigs` also offers automatic alias generators (`aliasGenerator: 'snake' | 'camel' | 'pascal'`).

### Error messages

Default messages follow the `Must be <type>` pattern, joined for unions (`Must be boolean, integer or string`). Override them per property with the `ErrorMessage` attribute:

```php
use Attributes\Validation\Fields\ErrorMessage;

class User extends BaseModel
{
    #[ErrorMessage(required: 'Name is mandatory', type: 'Expected {expected}')]
    public string $name;
}
```

The `{expected}` placeholder renders the expected type list.

## Array type validation

Typed `array` properties declare their element shape with the `Sequence`, `Union`, `Intersection` and `Dict` attributes:

```php
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Fields\Union;
use Attributes\Validation\Type;

class Blog extends BaseModel
{
    #[Sequence(Type::string)]
    public array $tags;

    #[Union(int, string, sequence)]
    public array $mixed;

    #[Dict(key: Type::int, of: stdClass::class)]
    public array $lookup;
}
```

Element types come from the `Attributes\Validation\Type` enum (`Type::int`, `Type::string`, `Type::bool`, `Type::float`, `Type::sequence`, `Type::object`, `Type::dict`) or the matching namespace constants (`int`, `string`, `bool`, `float`, `sequence`, `object`), and may combine with the bitwise OR operator (`bool | int`). Class, interface and enum names are accepted as strings (`DateTime::class`) and autoload on use.

- `Sequence` validates every element against one element type.
- `Union` accepts an element matching any declared arm.
- `Intersection` requires an element to match every declared class or interface. Its arms must name classes or interfaces only — enums are rejected, mirroring PHP intersection type hints.
- `Dict` validates keys against `$key` (integer and string types only) and values against `$of`.

The same attributes compose with `new` expressions to describe nested arrays:

```php
#[Sequence(of: new Union(string, int, float))]
public array $values;
```

Arguments of a composed `new` expression are validated at construction: an unsupported argument type throws a `TypeError` and a string that names no loadable class, interface or enum throws a `ValueError`.

## Hooks

Models may override two protected hooks, called by `validate()` around the validation run:

```php
abstract class BaseModel
{
    protected function beforeValidation(array $rawData, ModelConfigs $configs): array {}
    protected function afterValidation(array $rawData, ModelConfigs $configs): void {}
}
```
