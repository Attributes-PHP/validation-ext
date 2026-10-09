<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchCategory extends BaseModel
{
    public string $slug;

    public string $name;
}
