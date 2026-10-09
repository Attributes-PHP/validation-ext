<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Dict;
use Attributes\Validation\Type;

class BenchBrokenDict extends BaseModel
{
    #[Dict(key: Type::int)]
    public array $items;
}
