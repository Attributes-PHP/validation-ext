<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Alias;
use Attributes\Validation\ModelConfigs;

#[ModelConfigs(strict: false)]
class BenchSignup extends BaseModel
{
    #[Alias('user_name')]
    public string $name;

    public string $email;

    public int $age;
}
