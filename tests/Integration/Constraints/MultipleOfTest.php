<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\MultipleOf;

use function Attributes\Validation\validate;

describe('MultipleOf constructor argument validation', function () {
    it('promotes value to a public property', function () {
        $constraint = new MultipleOf(3);

        expect($constraint->value)->toBe(3);
    });
});

describe('MultipleOf validation', function () {
    it('accepts numbers that are multiples of the divisor', function (int|float $value) {
        $model = new class extends BaseModel {
            #[MultipleOf(3)]
            public $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([9, 3, 0, -6, 300]);

    it('accepts float multiples within representation noise', function () {
        $model = new class extends BaseModel {
            #[MultipleOf(0.1)]
            public $value;
        };

        $result = validate(['value' => 0.3], $model);
        expect($result->value)->toBe(0.3);
    });

    it('reports an error for non-multiples', function (int|float $value) {
        $model = new class extends BaseModel {
            #[MultipleOf(3)]
            public $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a multiple of 3']]);
        }
    })->with([7, 1, -2, 3.5]);

    it('reports an error for non-numeric values', function () {
        $model = new class extends BaseModel {
            #[MultipleOf(3)]
            public $value;
        };

        try {
            validate(['value' => 'x'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a multiple of 3']]);
        }
    });
});
