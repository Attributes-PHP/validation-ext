<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\Constraints\Min;
use Attributes\Validation\Fields\Constraints\MultipleOf;

use function Attributes\Validation\validate;

describe('Min constructor argument validation', function () {
    it('promotes value to a public property', function () {
        $constraint = new Min(3);

        expect($constraint->value)->toBe(3);
    });
});

describe('Min validation', function () {
    it('accepts numbers at least the bound', function (int|float $value) {
        $model = new class extends BaseModel {
            #[Min(3)]
            public int|float $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([3, 4, 100, 3.5]);

    it('reports an error for numbers below the bound', function (int|float $value) {
        $model = new class extends BaseModel {
            #[Min(3)]
            public $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be at least 3']]);
        }
    })->with([2, 0, -5, 2.9]);

    it('reports the error under the resolved alias name', function () {
        $model = new class extends BaseModel {
            #[Alias('min_value')]
            #[Min(3)]
            public int $value;
        };

        try {
            validate(['min_value' => 1], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['min_value' => ['Must be at least 3']]);
        }
    });

    it('collects the errors of every failing constraint on the property', function () {
        $model = new class extends BaseModel {
            #[Min(10), MultipleOf(3)]
            public int $value;
        };

        try {
            validate(['value' => 7], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => [
                'Must be at least 10',
                'Must be a multiple of 3',
            ]]);
        }
    });
});
