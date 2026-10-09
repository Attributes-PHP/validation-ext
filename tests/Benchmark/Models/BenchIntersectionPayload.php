<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Intersection;

class BenchIntersectionPayload extends BaseModel
{
    #[Intersection(BenchServiceInterface::class, BenchBootableInterface::class)]
    public array $services;
}
