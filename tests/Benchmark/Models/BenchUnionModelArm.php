<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Union;

use const Attributes\Validation\int;

class BenchUnionModelArm extends BaseModel
{
    #[Union(BenchAddress::class, int)]
    public array $items;
}
