<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Fields;

use Attributes\Validation\Fields\Dict;
use Attributes\Validation\Fields\Intersection;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Fields\Union;
use Attributes\Validation\Tests\Models\Enums\EnumStrTwoValues;
use Attributes\Validation\Type;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

describe('Sequence constructor argument validation', function () {
    it('accepts every supported $of type', function ($of) {
        expect(new Sequence($of))->toBeInstanceOf(Sequence::class);
    })->with([
        Type::int->value,
        stdClass::class,
        DateTimeInterface::class,
        Type::class,
        Type::int,
        new Sequence(Type::int),
        new Union(Type::int, Type::string),
        new Intersection(DateTimeInterface::class),
        new Dict(key: Type::int, of: Type::int),
        EnumStrTwoValues::class,
    ]);

    it('throws a TypeError when the $of type is unsupported', function () {
        new Sequence(3.14);
    })->throws(
        TypeError::class,
        'Sequence::__construct(): Argument #1 ($of) must be of type int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict, float given',
    );

    it('throws a ValueError when the $of string is not a valid class', function () {
        new Sequence('DoesNotExist');
    })->throws(
        ValueError::class,
        'Sequence::__construct(): Argument #1 ($of) must be a valid class, interface or enum, "DoesNotExist" given',
    );
});
