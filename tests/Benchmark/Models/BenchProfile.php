<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchProfile extends BaseModel
{
    public string $username;

    public string $email;

    public int $loginCount;
}
