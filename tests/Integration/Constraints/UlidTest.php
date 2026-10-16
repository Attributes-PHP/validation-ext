<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Ulid;

use function Attributes\Validation\validate;

describe('Ulid constraint', function () {
    it('accepts valid ULID strings', function (string $value) {
        $model = new class extends BaseModel {
            #[Ulid]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        '01ARZ3NDEKTSV4RRFFQ69G5FAV',
        '01BX5ZZKBKACTV9DYEBKMJHZEQ',
        '7ZZZZZZZZZZZZZZZZZZZZZZZZZ',
        '01arz3ndektsv4rrffq69g5fav', // lowercase also accepted
    ]);

    it('reports an error for invalid ULID strings', function (string $value) {
        $model = new class extends BaseModel {
            #[Ulid]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid ULID']]);
        }
    })->with([
        '01ARZ3NDEKTSV4RRFFQ69G5FA', // 25 characters
        '01ARZ3NDEKTSV4RRFFQ69G5FAVX', // 27 characters
        '01ARZ3NDEKTSV4RRFFQ69G5FAI', // contains I
        '01ARZ3NDEKTSV4RRFFQ69G5FAL', // contains L
        '01ARZ3NDEKTSV4RRFFQ69G5FAO', // contains O
        '01ARZ3NDEKTSV4RRFFQ69G5FAU', // contains U
        'not-a-ulid',
        '',
    ]);

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[Ulid]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid ULID']]);
        }
    });
});
