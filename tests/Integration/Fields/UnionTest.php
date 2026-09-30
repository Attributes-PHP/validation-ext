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

describe('Union constructor argument validation', function () {
    it('accepts every supported arm type', function ($arm) {
        expect(new Union($arm))->toBeInstanceOf(Union::class);
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

    it('throws a TypeError when an arm type is unsupported', function () {
        new Union(Type::int, 3.14);
    })->throws(
        TypeError::class,
        'Union::__construct(): Argument #2 must be of type int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict, float given',
    );

    it('throws a ValueError when a string arm is not a valid class', function () {
        new Union(Type::int, 'DoesNotExist');
    })->throws(
        ValueError::class,
        'Union::__construct(): Argument #2 must be a valid class, interface or enum, "DoesNotExist" given',
    );
});
