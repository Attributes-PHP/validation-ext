<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Models;

class IterableCountableClass implements CountableInterface, IterableInterface
{
    public function count(): string
    {
        return 'IterableCountableClass->class(...)';
    }

    public function iterate(): string
    {
        return 'IterableCountableClass->iterate(...)';
    }
}
