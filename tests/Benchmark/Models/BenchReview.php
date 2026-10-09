<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchReview extends BaseModel
{
    public string $author;

    public string $comment;

    public int $rating;
}
