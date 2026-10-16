<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\Fields\Constraints\AlphaDash;
use Attributes\Validation\Fields\Constraints\Ascii;
use Attributes\Validation\Fields\Constraints\Between;
use Attributes\Validation\Fields\Constraints\Contains;
use Attributes\Validation\Fields\Constraints\DateTime;
use Attributes\Validation\Fields\Constraints\EndsWith;
use Attributes\Validation\Fields\Constraints\In;
use Attributes\Validation\Fields\Constraints\Max;
use Attributes\Validation\Fields\Constraints\Min;
use Attributes\Validation\Fields\Constraints\MultipleOf;
use Attributes\Validation\Fields\Constraints\Regex;
use Attributes\Validation\Fields\Constraints\Size;
use Attributes\Validation\Fields\Constraints\StartsWith;
use Attributes\Validation\Fields\Constraints\Ulid;
use Attributes\Validation\Fields\Field;

describe('built-in constraint classes', function () {
    it('are final: userland constraints implement the Constraint interface instead of extending built-ins', function (string $class) {
        expect((new \ReflectionClass($class))->isFinal())->toBeTrue()
            ->and(is_subclass_of($class, Field::class))
            ->toBeTrue();
    })->with([
        AlphaDash::class,
        Ascii::class,
        Between::class,
        Contains::class,
        DateTime::class,
        EndsWith::class,
        In::class,
        Max::class,
        Min::class,
        MultipleOf::class,
        Regex::class,
        Size::class,
        StartsWith::class,
        Ulid::class,
    ]);
});
