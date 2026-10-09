---
title: "Overview"
description: "Pydantic-inspired PHP extension that validates data via type hints at native speed."
---

# Attributes Validation

![Attributes Validation](assets/images/validation-ext-wallpaper.png)

**Attributes Validation** is a Pydantic-inspired PHP extension for validating data via type hints at lightning speed. The validation logic is implemented in C and compiled into the PHP engine, so your model types are the schema: no configuration files, no YAML.

```php
<?php

use function Attributes\Validation\validate;
use Attributes\Validation\BaseModel;

class User extends BaseModel
{
    public float|int $age;
    public ?DateTime $birthday;
}

$rawData = [
    'age' => 30,
    'birthday' => '1994-01-01T09:00:00+00:00',
];

$user = validate($rawData, new User());

var_dump($user->age);      // int(30)
var_dump($user->birthday); // object(DateTime) "1994-01-01 09:00:00.000000" (+00:00)
```

## Requirements

- PHP 8.2 or later

## Installation

```bash
pie install attributes-php/validation
```

## What's next

- [validate()](/validate/) — the full function reference: parameters, return value, validation rules and exceptions.

## Running this documentation locally

```bash
npx @docmd/core dev
```

The site serves at `http://localhost:3000` and reloads as you edit the Markdown under `docs/`.
