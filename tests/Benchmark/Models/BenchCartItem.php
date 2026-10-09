<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchCartItem extends BaseModel
{
    public string $sku;

    public int $quantity;

    public float $unitPrice;
}
