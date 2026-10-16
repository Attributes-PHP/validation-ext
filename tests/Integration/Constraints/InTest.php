<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\In;
use TypeError;

use function Attributes\Validation\validate;

describe('In constructor argument validation', function () {
    it('collects the variadic values on the $values property', function () {
        $constraint = new In('a', 1, 2.5, true, false);

        expect($constraint->values)->toBe(['a', 1, 2.5, true, false]);
    });

    it('accepts zero values per the stub', function () {
        $constraint = new In;

        expect($constraint->values)->toBeEmpty();
    });

    it('throws a TypeError for unsupported element types', function (mixed $value) {
        expect(fn () => new In($value))->toThrow(TypeError::class);
    })->with([
        'array' => [[]],
        'null' => [null],
        'object' => [new \stdClass],
    ]);
});

describe('In validation', function () {
    it('accepts values strictly identical to a declared value', function (string|int|float|bool $value) {
        $model = new class extends BaseModel {
            #[In('a', 'b', 1, 2.5, true)]
            public $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with(['a', 'b', 1, 2.5, true]);

    it('reports an error for other values', function (mixed $value) {
        $model = new class extends BaseModel {
            #[In('a', 'b', 1, 2.5, true)]
            public $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must be one of: 'a', 'b', 1, 2.5, true"]]);
        }
    })->with(['c', 2, '1', 'a b', 0, false]);

    it('never matches when no values are declared', function () {
        $model = new class extends BaseModel {
            #[In]
            public $value;
        };

        try {
            validate(['value' => 'a'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be one of: ']]);
        }
    });
});
