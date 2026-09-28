<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Models;

use Attributes\Validation\BaseModel;

/**
 * Only referenced through a docstring array shape by ArrayTypeHintTest:
 * the extension must resolve it through autoloading at validation time.
 */
class ArrayTypeHintLazyAddress extends BaseModel
{
    public string $street;
}
