# Attributes Validation Extension

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
$person = validate($rawData, new Person);

var_dump($person->age);      // int(30)
var_dump($person->birthday); // object(DateTime) { ["date"] => string(26) "1994-01-01 09:00:00.000000", (...) }
```

### Need to call a function?

```php
<?php

use Attributes\Validation\call;

function myFunction(User $user) {
    var_dump($person->age);      // int(30)
    var_dump($person->birthday); // object(DateTime) { ["date"] => string(26) "1994-01-01 09:00:00.000000", (...) }
}

call("myFunction", $rawData);
```

## Installation

```bash
pie install attributes-php/validation
```

Attributes Validation Extension was created by **[André Gil](https://www.linkedin.com/in/andre-gil/)** and is open-sourced software licensed under the **[MIT license](https://opensource.org/licenses/MIT)**.