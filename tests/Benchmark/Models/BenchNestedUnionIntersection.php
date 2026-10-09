<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Intersection;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Fields\Union;

use const Attributes\Validation\int;

class BenchNestedUnionIntersection extends BaseModel
{
    #[Sequence(of: new Union(int, new Intersection(BenchServiceInterface::class, BenchBootableInterface::class)))]
    public array $payloads;
}
