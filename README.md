# Attributes Validation Extension

**Attributes Validation Extension** is a Pydantic-inspired PHP library for validating data via type hints at lightning speed 🚀

## Prerequisites

- PHP 8.2 or later

We aim to support versions that haven't reached their end-of-life.

## Features

- **Type hint validation**: Automatically validate and coerce data based on property type hints
- **Custom field names**: Use `#[Alias]` attribute to map properties to different field names in raw data
- **Validation hooks**: Use `beforeValidation` and `afterValidation` methods to run custom logic
- **Model configuration**: Use `#[ModelConfigs]` to configure validation behavior
- **Error handling**: Collect all validation errors or stop at the first error

## How it works?

```php
<?php

use Attributes\Validation\validate;
use Attributes\Validation\BaseModel;

class User extends BaseModel
{
    public float|int $age;
    public ?DateTime $birthday;
}

$rawData = [
    'age' => '30',
    'birthday' => '1994-01-01T09:00:00+00:00',
];
$user = validate($rawData, new User());

var_dump($user->age);      // int(30)
var_dump($user->birthday); // object(DateTime) { ["date"] => string(26) "1994-01-01 09:00:00.000000", (...) }
```

### Need to call a function?

```php
<?php

use Attributes\Validation\call;

function myFunction(User $user) {
    var_dump($user->age);      // int(30)
    var_dump($user->birthday); // object(DateTime) { ["date"] => string(26) "1994-01-01 09:00:00.000000", (...) }
}

call("myFunction", $rawData);
```

## Using Alias Attribute

```php
<?php

use Attributes\Validation\validate;
use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Alias;

class User extends BaseModel
{
    #[Alias('user_name')]
    public string $name;
}

$rawData = ['user_name' => 'John Doe'];
$user = validate($rawData, new User());

echo $user->name; // 'John Doe'
```

## Using Validation Hooks

```php
<?php

use Attributes\Validation\validate;
use Attributes\Validation\BaseModel;
use Attributes\Validation\ModelConfigs;

class User extends BaseModel
{
    public string $name;

    protected function beforeValidation(array $rawData, ModelConfigs $configs): array
    {
        // Modify raw data before validation
        $rawData['name'] = strtolower($rawData['name'] ?? '');
        return $rawData;
    }

    protected function afterValidation(array $rawData, ModelConfigs $configs): void
    {
        // Run custom logic after validation
        $this->name = ucfirst($this->name);
    }
}

$user = validate(['name' => 'JOHN DOE'], new User());
echo $user->name; // 'John doe'
```

## Installation

```bash
pie install attributes-php/validation
```

Attributes Validation Extension was created by **[André Gil](https://www.linkedin.com/in/andre-gil/)** and is open-sourced software licensed under the **[MIT license](https://opensource.org/licenses/MIT)**.
