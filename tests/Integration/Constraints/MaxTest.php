<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Max;
use TypeError;

use function Attributes\Validation\validate;

describe('Max constructor argument validation', function () {
    it('promotes value to a public property', function () {
        $constraint = new Max(7);

        expect($constraint->value)->toBe(7);
    });

    it('throws a TypeError for non-numeric values', function () {
        expect(fn () => new Max('x'))->toThrow(TypeError::class);
    });
});

describe('Max validation', function () {
    it('accepts numbers at most the bound', function (int|float $value) {
        $model = new class extends BaseModel {
            #[Max(7)]
            public int|float $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([7, 0, -3, 6.99]);

    it('reports an error for numbers above the bound', function (int|float $value) {
        $model = new class extends BaseModel {
            #[Max(7)]
            public $value;
        };
        $model->value = 'sentinel';

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be at most 7']]);
            expect($model->value)->toBe('sentinel');
        }
    })->with([8, 100, 7.1]);

    it('reports an error for non-numeric values', function () {
        $model = new class extends BaseModel {
            #[Max(7)]
            public $value;
        };

        try {
            validate(['value' => 'x'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be at most 7']]);
        }
    });

    it('supports float bounds', function () {
        $model = new class extends BaseModel {
            #[Max(2.5)]
            public $value;
        };

        try {
            validate(['value' => 2.6], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be at most 2.5']]);
        }
    });
});
