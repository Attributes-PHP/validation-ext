<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchShallowModel extends BaseModel
{
    public string $name;

    public int $count;

    public float $ratio;
}
