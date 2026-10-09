<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Fields;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Intersection;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\ModelConfigs;
use Attributes\Validation\Tests\Models\CountableClass;
use Attributes\Validation\Tests\Models\Enums\EnumStrTwoValues;
use Attributes\Validation\Tests\Models\Interfaces\CountableInterface;
use Attributes\Validation\Tests\Models\Interfaces\IterableInterface;
use Attributes\Validation\Tests\Models\IterableCountableClass;
use Attributes\Validation\Type;
use DateTimeInterface;
use stdClass;
use TypeError;
use ValueError;

use function Attributes\Validation\validate;

class IntersectionBothConcrete implements CountableInterface, IterableInterface
{
    public function count(): string
    {
        return 'count';
    }

    public function iterate(): string
    {
        return 'iterate';
    }
}

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

describe('Intersection attribute array data validation', function () {
    it('accepts an array of instances implementing every interface', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(CountableInterface::class, IterableInterface::class)]
                public array $items;
            };

        $result = validate(['items' => [new IterableCountableClass, new IntersectionBothConcrete]], $model);
        expect($result->items[0])->toBeInstanceOf(IterableCountableClass::class);
        expect($result->items[1])->toBeInstanceOf(IntersectionBothConcrete::class);
    });

    it('rejects an instance implementing only one interface', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(CountableInterface::class, IterableInterface::class)]
                public array $items;
            };

        try {
            validate(['items' => [new CountableClass]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => [
                    'Must be Attributes\Validation\Tests\Models\Interfaces\CountableInterface and'
                        .' Attributes\Validation\Tests\Models\Interfaces\IterableInterface',
                ],
            ]);
        }
    });

    it('rejects scalar elements', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(CountableInterface::class, IterableInterface::class)]
                public array $items;
            };

        try {
            validate(['items' => ['nope']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => [
                    'Must be Attributes\Validation\Tests\Models\Interfaces\CountableInterface and'
                        .' Attributes\Validation\Tests\Models\Interfaces\IterableInterface',
                ],
            ]);
        }
    });

    it('requires every arm to match, including a concrete class arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(CountableInterface::class, IterableInterface::class, IntersectionBothConcrete::class)]
                public array $items;
            };

        $result = validate(['items' => [new IntersectionBothConcrete]], $model);
        expect($result->items[0])->toBeInstanceOf(IntersectionBothConcrete::class);

        try {
            validate(['items' => [new IterableCountableClass]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => [
                    'Must be Attributes\Validation\Tests\Models\Interfaces\CountableInterface,'
                        .' Attributes\Validation\Tests\Models\Interfaces\IterableInterface and'
                        .' Attributes\Validation\Tests\Integration\Fields\IntersectionBothConcrete',
                ],
            ]);
        }
    });

    it('accepts only instances of a concrete class arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(IntersectionBothConcrete::class)]
                public array $items;
            };

        $result = validate(['items' => [new IntersectionBothConcrete]], $model);
        expect($result->items[0])->toBeInstanceOf(IntersectionBothConcrete::class);

        try {
            validate(['items' => [new IterableCountableClass]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => ['Must be Attributes\Validation\Tests\Integration\Fields\IntersectionBothConcrete'],
            ]);
        }
    });

    it('accepts an empty array', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(CountableInterface::class, IterableInterface::class)]
                public array $items;
            };

        $result = validate(['items' => []], $model);
        expect($result->items)->toBeEmpty();
    });

    it('validates every element of a Sequence through a composed Intersection', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(of: new Intersection(CountableInterface::class, IterableInterface::class))]
                public array $items;
            };

        $result = validate(['items' => [new IterableCountableClass, new IntersectionBothConcrete]], $model);
        expect($result->items)->toHaveLength(2);

        try {
            validate(['items' => [new IterableCountableClass, new CountableClass]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.1' => [
                    'Must be Attributes\Validation\Tests\Models\Interfaces\CountableInterface and'
                        .' Attributes\Validation\Tests\Models\Interfaces\IterableInterface',
                ],
            ]);
        }
    });
});
