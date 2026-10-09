<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchSession extends BaseModel
{
    public string $sessionId;

    public string $userId;

    public bool $rememberMe;
}
