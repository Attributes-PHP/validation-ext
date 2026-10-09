<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Sequence;

class BenchOrder extends BaseModel
{
    public int $id;

    #[Sequence(BenchAddress::class)]
    public array $addresses;
}
