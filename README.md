# Attributes Validation Extension

<p align="center">
    <img src="https://raw.githubusercontent.com/Attributes-PHP/validation-ext/main/docs/assets/images/validation-ext-wallpaper.png" alt="validation-ext logo" width="600">
</p>
<p align="center">
    <a href="https://github.com/Attributes-PHP/validation-ext/actions"><img alt="GitHub Actions Workflow Status (main)" src="https://img.shields.io/github/actions/workflow/status/Attributes-PHP/validation-ext/tests.yml"></a>
    <a href="https://codecov.io/gh/Attributes-PHP/validation-ext" ><img alt="Code Coverage" src="https://codecov.io/gh/Attributes-PHP/validation-ext/graph/badge.svg?token=8N7N9NMGLG"/></a>
    <a href="https://packagist.org/packages/Attributes-PHP/validation-ext"><img alt="Latest Version" src="https://img.shields.io/packagist/v/Attributes-PHP/validation-ext"></a>
    <a href="https://packagist.org/packages/Attributes-PHP/validation-ext"><img alt="Supported PHP Versions" src="https://img.shields.io/badge/8.2%2B?label=versions"></a>
    <a href="https://opensource.org/licenses/MIT"><img alt="Software License" src="https://img.shields.io/badge/Licence-MIT-brightgreen"></a>
</p>

------

**Attributes Validation Extension** is a Pydantic-inspired PHP library for validating data via type hints at lightning speed ⚡️

## Prerequisites

- PHP 8.2 or later

We aim to support versions that haven't reached their end-of-life.

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

## Installation

```bash
pie install attributes-php/validation
```

Attributes Validation Extension was created by **[André Gil](https://www.linkedin.com/in/andre-gil/)** and is open-sourced software licensed under the **[MIT license](https://opensource.org/licenses/MIT)**.