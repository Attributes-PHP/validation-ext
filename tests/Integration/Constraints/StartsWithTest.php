<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\StartsWith;
use TypeError;

use function Attributes\Validation\validate;

describe('StartsWith constructor argument validation', function () {
    it('collects the variadic values on the $values property', function () {
        $constraint = new StartsWith('Mr', 2, 3.5);

        expect($constraint->values)->toBe(['Mr', 2, 3.5]);
    });

    it('accepts zero values per the stub', function () {
        $constraint = new StartsWith;

        expect($constraint->values)->toBeEmpty();
    });

    it('throws a TypeError for unsupported element types', function (mixed $value) {
        expect(fn () => new StartsWith($value))->toThrow(TypeError::class);
    })->with([
        'array' => [[]],
        'null' => [null],
        'bool' => [true],
        'object' => [new \stdClass],
    ]);
});

describe('StartsWith validation', function () {
    it('accepts strings starting with one of the declared values', function (string $value) {
        $model = new class extends BaseModel {
            #[StartsWith('Mr', 'Ms')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'Mr Smith',
        'Ms Jones',
        'Mrs Brown',
        'Ms',
    ]);

    it('reports an error for other prefixes', function (string $value) {
        $model = new class extends BaseModel {
            #[StartsWith('Mr', 'Ms')]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must start with one of: 'Mr', 'Ms'"]]);
        }
    })->with([
        'Dr Smith',
        'mr Smith',
        'Sir',
        '',
    ]);

    it('matches numeric declared values by their string representation', function () {
        $model = new class extends BaseModel {
            #[StartsWith(42)]
            public string $value;
        };

        $result = validate(['value' => '42 is the answer'], $model);
        expect($result->value)->toBe('42 is the answer');
    });

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[StartsWith('x')]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must start with one of: 'x'"]]);
        }
    });
});
