<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

class BenchRegistryService implements BenchServiceInterface, BenchBootableInterface
{
    public function __construct(
        private readonly string $id,
    ) {}

    public function serviceId(): string
    {
        return $this->id;
    }

    public function boot(): void {}
}
