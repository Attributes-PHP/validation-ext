<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchProduct extends BaseModel
{
    public string $sku;

    public string $title;

    public float $price;

    public int $stock;
}
