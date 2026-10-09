<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchSimpleUser extends BaseModel
{
    public int $age;

    public string $name;

    public string $email;

    public bool $active;

    public float $balance;
}
