<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Between;
use TypeError;

use function Attributes\Validation\validate;

describe('Between constructor argument validation', function () {
    it('promotes min and max to public properties', function () {
        $constraint = new Between(1, 10.5);

        expect($constraint->min)->toBe(1)->and($constraint->max)->toBe(10.5);
    });

    it('throws a TypeError when min or max is not a number', function () {
        expect(fn () => new Between('a', 10))->toThrow(TypeError::class);
        expect(fn () => new Between(1, [10]))->toThrow(TypeError::class);
    });
});

describe('Between validation', function () {
    it('accepts numbers between min and max inclusive', function (int|float $value) {
        $model = new class extends BaseModel {
            #[Between(1, 10)]
            public int|float $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([1, 5, 10, 5.5, 1.0, 10.0]);

    it('reports an error for numbers outside the bounds', function (int|float $value) {
        $model = new class extends BaseModel {
            #[Between(1, 10)]
            public $value;
        };
        $model->value = 'sentinel';

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be between 1 and 10']]);
            expect($model->value)->toBe('sentinel');
        }
    })->with([0, 11, -1, 10.1, 0.99]);

    it('reports an error for non-numeric values', function () {
        $model = new class extends BaseModel {
            #[Between(1, 10)]
            public $value;
        };

        try {
            validate(['value' => 'nope'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be between 1 and 10']]);
        }
    });

    it('runs after the native type check so type errors stay type errors', function () {
        $model = new class extends BaseModel {
            #[Between(1, 10)]
            public int $value;
        };

        try {
            validate(['value' => 'x'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be integer']]);
        }
    });
});
