<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Dict;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Type;

class BenchNestedShape extends BaseModel
{
    #[Sequence(new Sequence(Type::int))]
    public array $matrix;

    #[Dict(key: Type::string, of: new Sequence(Type::int))]
    public array $groups;
}
