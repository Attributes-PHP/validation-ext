<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\AlphaDash;

use function Attributes\Validation\validate;

describe('AlphaDash constraint', function () {
    it('accepts values containing only letters, numbers, dashes and underscores', function (string $value) {
        $model = new class extends BaseModel {
            #[AlphaDash]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'abc',
        'A1b2C3',
        'user_name',
        'my-slug',
        'a-b_c1',
        '',
        '123',
    ]);

    it('reports an error for values containing other characters', function (string $value) {
        $model = new class extends BaseModel {
            #[AlphaDash]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must only contain letters, numbers, dashes and underscores']]);
        }
    })->with([
        'has space',
        'héllo',
        'a.b',
        'foo!',
        'tab	sep',
    ]);

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[AlphaDash]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must only contain letters, numbers, dashes and underscores']]);
        }
    });
});
