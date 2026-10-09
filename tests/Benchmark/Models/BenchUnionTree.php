<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Union;

use const Attributes\Validation\int;
use const Attributes\Validation\string;

class BenchUnionTree extends BaseModel
{
    public string $name;

    #[Union(int, string, BenchUnionTree::class)]
    public array $children;
}
