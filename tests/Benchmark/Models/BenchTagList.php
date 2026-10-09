<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Type;

class BenchTagList extends BaseModel
{
    public string $name;

    #[Sequence(Type::string)]
    public array $tags;
}
