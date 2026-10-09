<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\ErrorMessage;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Type;

class BenchAttributeUser extends BaseModel
{
    #[Alias('user_name')]
    public string $name;

    #[ErrorMessage(type: 'Expected {expected}')]
    public int $age;

    #[Sequence(Type::string)]
    public array $tags;
}
