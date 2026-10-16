<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\EndsWith;
use TypeError;

use function Attributes\Validation\validate;

describe('EndsWith constructor argument validation', function () {
    it('collects the variadic values on the $values property', function () {
        $constraint = new EndsWith('.jpg', 2, 3.5);

        expect($constraint->values)->toBe(['.jpg', 2, 3.5]);
    });

    it('accepts zero values per the stub', function () {
        $constraint = new EndsWith;

        expect($constraint->values)->toBeEmpty();
    });

    it('throws a TypeError for unsupported element types', function (mixed $value) {
        expect(fn () => new EndsWith($value))->toThrow(TypeError::class);
    })->with([
        'array' => [[]],
        'null' => [null],
        'bool' => [true],
        'object' => [new \stdClass],
    ]);
});

describe('EndsWith validation', function () {
    it('accepts strings ending with one of the declared values', function (string $value) {
        $model = new class extends BaseModel {
            #[EndsWith('.jpg', '.png')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'photo.jpg',
        'image.png',
        'a.b.jpg',
    ]);

    it('reports an error for other endings', function (string $value) {
        $model = new class extends BaseModel {
            #[EndsWith('.jpg', '.png')]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must end with one of: '.jpg', '.png'"]]);
        }
    })->with([
        'photo.gif',
        'photojpg',
        '',
        '.JPG',
    ]);

    it('matches numeric declared values by their string representation', function () {
        $model = new class extends BaseModel {
            #[EndsWith(42)]
            public string $value;
        };

        $result = validate(['value' => 'the answer is 42'], $model);
        expect($result->value)->toBe('the answer is 42');
    });

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[EndsWith('x')]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must end with one of: 'x'"]]);
        }
    });
});
