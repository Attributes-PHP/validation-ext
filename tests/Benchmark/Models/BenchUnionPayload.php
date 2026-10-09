<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Union;
use Attributes\Validation\ModelConfigs;

use const Attributes\Validation\bool;
use const Attributes\Validation\float;
use const Attributes\Validation\int;
use const Attributes\Validation\string;

#[ModelConfigs(strict: false)]
class BenchUnionPayload extends BaseModel
{
    #[Union(int, string, bool)]
    public array $mixed;

    #[Union(int, float)]
    public array $coerced;
}
