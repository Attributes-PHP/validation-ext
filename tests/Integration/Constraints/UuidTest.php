<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Uuid;

use function Attributes\Validation\validate;

describe('Uuid constraint', function () {
    it('accepts valid UUID strings', function (string $value) {
        $model = new class extends BaseModel {
            #[Uuid]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        '123e4567-e89b-12d3-a456-426614174000',
        '123E4567-E89B-12D3-A456-426614174000', // uppercase also accepted
        '00000000-0000-0000-0000-000000000000', // nil UUID
        'ffffffff-ffff-ffff-ffff-ffffffffffff',
    ]);

    it('reports an error for invalid UUID strings', function (string $value) {
        $model = new class extends BaseModel {
            #[Uuid]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid UUID']]);
        }
    })->with([
        '123e4567e89b12d3a456426614174000', // missing dashes
        '123e4567-e89b-12d3-a456-42661417400', // 35 characters
        '123e4567-e89b-12d3-a456-4266141740000', // 37 characters
        '123e4567-e89b1-2d3a-456-426614174000', // misplaced dashes
        'g23e4567-e89b-12d3-a456-426614174000', // g is not a hex digit
        '123e4567+e89b-12d3-a456-426614174000', // wrong separator
        '123e4567-e89b_12d3-a456-426614174000', // wrong separator
        'not-a-uuid',
        '',
    ]);

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[Uuid]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be a valid UUID']]);
        }
    });
});
