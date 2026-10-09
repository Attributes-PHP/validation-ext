<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

interface BenchBootableInterface
{
    public function boot(): void;
}
