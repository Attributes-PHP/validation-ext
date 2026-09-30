<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Fields;

use Attributes\Validation\Fields\Intersection;
use Attributes\Validation\Tests\Models\Enums\EnumStrTwoValues;
use Attributes\Validation\Type;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

describe('Intersection constructor argument validation', function () {
    it('accepts every supported argument type', function ($of) {
        expect(new Intersection($of))->toBeInstanceOf(Intersection::class);
    })->with([
        DateTimeInterface::class,
        stdClass::class,
    ]);

    it('throws a TypeError when an argument is not a string', function () {
        new Intersection(DateTimeInterface::class, Type::int);
    })->throws(
        TypeError::class,
        'Intersection::__construct(): Argument #2 must be of type string, Attributes\Validation\Type given',
    );

    it('throws a ValueError when a string argument is not a valid class', function () {
        new Intersection(DateTimeInterface::class, 'DoesNotExist');
    })->throws(
        ValueError::class,
        'Intersection::__construct(): Argument #2 must be a class or interface, "DoesNotExist" given',
    );

    it('throws a ValueError when a string argument is an enum', function () {
        new Intersection(EnumStrTwoValues::class);
    })->throws(
        ValueError::class,
        'Intersection::__construct(): Argument #1 must be a class or interface, "Attributes\Validation\Tests\Models\Enums\EnumStrTwoValues" given',
    );

    it('throws a ValueError when a string argument is the Type enum', function () {
        new Intersection(Type::class);
    })->throws(
        ValueError::class,
        'Intersection::__construct(): Argument #1 must be a class or interface, "Attributes\Validation\Type" given',
    );
});
