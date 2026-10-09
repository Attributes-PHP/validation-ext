<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchInvoice extends BaseModel
{
    public string $number;

    public string $customer;

    public float $total;
}
