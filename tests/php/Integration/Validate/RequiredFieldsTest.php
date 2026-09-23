<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;

use function Attributes\Validation\validate;

describe('validate function required field validation', function () {
    it('throws ValidationException when required field is missing', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        validate([], $model);
    })->throws(ValidationException::class);

    it('includes field name in error when required field is missing', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue(); // Should not reach here
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect($errors)->toHaveKey('name');
        }
    });

    it('accepts all required fields when present', function () {
        $model = new class extends BaseModel {
            public string $name;

            public string $email;
        };

        $result = validate(['name' => 'John', 'email' => 'john@example.com'], $model);
        expect($result->name)->toBe('John');
        expect($result->email)->toBe('john@example.com');
    });

    it('handles partial data with missing required fields', function () {
        $model = new class extends BaseModel {
            public string $name;

            public string $email;
        };

        try {
            validate(['name' => 'John'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect($errors)->toHaveKey('email')->not->toHaveKey('name');
        }
    });
});
