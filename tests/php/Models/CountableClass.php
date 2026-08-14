<?php

namespace Attributes\Validation\Tests\Models;

use Attributes\Validation\Tests\Models\Interfaces\CountableInterface;

class CountableClass implements CountableInterface
{
    public function count(): string
    {
        return 'CountableClass->count(...)';
    }
}
