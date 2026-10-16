<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Size;
use TypeError;

use function Attributes\Validation\validate;

describe('Size constructor argument validation', function () {
    it('applies the stub defaults when no arguments are given', function () {
        $constraint = new Size;

        expect($constraint->min)->toBeNull()->and($constraint->max)->toBeNull();
    });

    it('supports nullable named bounds', function () {
        $constraint = new Size(min: 3);
        expect($constraint->min)->toBe(3)->and($constraint->max)->toBeNull();

        $constraint = new Size(max: 5);
        expect($constraint->min)->toBeNull()->and($constraint->max)->toBe(5);

        $constraint = new Size(3, 5);
        expect($constraint->min)->toBe(3)->and($constraint->max)->toBe(5);

        $constraint = new Size(null, null);
        expect($constraint->min)->toBeNull()->and($constraint->max)->toBeNull();
    });

    it('throws a TypeError for non-nullable-int bounds', function () {
        expect(fn () => new Size(1.5))->toThrow(TypeError::class);
        expect(fn () => new Size('x'))->toThrow(TypeError::class);
    });
});

describe('Size validation', function () {
    it('accepts strings and arrays whose size is within both bounds', function (string|array $value) {
        $model = new class extends BaseModel {
            #[Size(min: 3, max: 5)]
            public $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with(['abc', 'abcd', 'abcde', [[1, 2, 3]], [[1, 2, 3, 4, 5]]]);

    it('reports an error for sizes outside both bounds', function (string|array $value) {
        $model = new class extends BaseModel {
            #[Size(min: 3, max: 5)]
            public $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must have a size between 3 and 5']]);
        }
    })->with(['ab', 'abcdef', [[1, 2]], [[1, 2, 3, 4, 5, 6]], [[]]]);

    it('enforces a min-only bound', function (string $value) {
        $model = new class extends BaseModel {
            #[Size(min: 2)]
            public $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);

        try {
            validate(['value' => 'x'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must have a size of at least 2']]);
        }
    })->with(['ab']);

    it('enforces a max-only bound', function (array $value) {
        $model = new class extends BaseModel {
            #[Size(max: 2)]
            public $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);

        try {
            validate(['value' => [1, 2, 3]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must have a size of at most 2']]);
        }
    })->with([[[1, 2]]]);

    it('passes every value when no bound is set', function () {
        $model = new class extends BaseModel {
            #[Size]
            public $value;
        };

        $result = validate(['value' => 'anything at all'], $model);
        expect($result->value)->toBe('anything at all');
    });

    it('reports an error for other value types', function () {
        $model = new class extends BaseModel {
            #[Size(min: 1)]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must have a size of at least 1']]);
        }
    });
});
