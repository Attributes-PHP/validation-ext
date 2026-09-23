<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Models;

use Attributes\Validation\Tests\Models\Interfaces\IterableInterface;

class IterableClass implements IterableInterface
{
    public function iterate(): string
    {
        return 'IterableClass->iterate(...)';
    }
}
