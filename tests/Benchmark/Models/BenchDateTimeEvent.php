<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;

class BenchDateTimeEvent extends BaseModel
{
    public \DateTime $createdAt;

    public \DateTime $updatedAt;

    public \DateTime $deletedAt;

    public \DateTime $archivedAt;
}
