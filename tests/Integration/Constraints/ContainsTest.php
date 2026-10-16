<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Contains;
use TypeError;

use function Attributes\Validation\validate;

describe('Contains constructor argument validation', function () {
    it('collects the variadic values on the $values property', function () {
        $constraint = new Contains('red', 2, 3.5);

        expect($constraint->values)->toBe(['red', 2, 3.5]);
    });

    it('accepts zero values per the stub', function () {
        $constraint = new Contains;

        expect($constraint->values)->toBeEmpty();
    });

    it('throws a TypeError for unsupported element types', function (mixed $value) {
        expect(fn () => new Contains($value))->toThrow(TypeError::class);
    })->with([
        'array' => [[]],
        'null' => [null],
        'bool' => [true],
        'object' => [new \stdClass],
    ]);
});

describe('Contains validation', function () {
    it('accepts arrays containing every declared value', function (array $value) {
        $model = new class extends BaseModel {
            #[Contains('red', 'blue')]
            public array $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        [['red', 'blue']],
        [['blue', 'red']],
        [['red', 'blue', 'green']],
    ]);

    it('reports an error for arrays missing a declared value', function (array $value) {
        $model = new class extends BaseModel {
            #[Contains('red', 'blue')]
            public $value;
        };
        $model->value = 'sentinel';

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must contain 'red', 'blue'"]]);
            expect($model->value)->toBe('sentinel');
        }
    })->with([
        [['red']],
        [['green', 'red']],
        [[]],
    ]);

    it('compares array elements strictly', function () {
        $model = new class extends BaseModel {
            #[Contains('1')]
            public $value;
        };

        try {
            validate(['value' => [1]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must contain '1'"]]);
        }
    });

    it('accepts strings containing every declared value', function (string $value) {
        $model = new class extends BaseModel {
            #[Contains('@', 'test')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'a@test.com',
        'test@x',
        'x@test.test.com',
    ]);

    it('reports an error for strings missing a declared value', function () {
        $model = new class extends BaseModel {
            #[Contains('@')]
            public string $value;
        };

        try {
            validate(['value' => 'no-at-sign'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must contain '@'"]]);
        }
    });

    it('matches numeric declared values by their string representation', function () {
        $model = new class extends BaseModel {
            #[Contains(42)]
            public string $value;
        };

        $result = validate(['value' => 'the answer is 42'], $model);
        expect($result->value)->toBe('the answer is 42');
    });

    it('reports an error for other value types', function () {
        $model = new class extends BaseModel {
            #[Contains('x')]
            public $value;
        };

        try {
            validate(['value' => 42], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must contain 'x'"]]);
        }
    });
});
