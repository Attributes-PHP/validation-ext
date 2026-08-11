<?php

namespace Attributes\Validation\Tests\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;

use function Attributes\Validation\validate;

describe('validate function error handling', function () {
    it('throws ValidationException for missing required fields', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        validate([], $model);
    })->throws(ValidationException::class);

    it('throws ValidationException with error details', function () {
        $model = new class extends BaseModel
        {
            public string $name;

            public string $email;
        };

        try {
            validate(['name' => 'John'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e)->toBeInstanceOf(ValidationException::class);
            expect($e->getErrors())->toBeArray();
            expect(count($e->getErrors()))->toBeGreaterThan(0);
        }
    });

    it('includes error message for required field', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect($errors['name'] ?? null)->toContain('Required field');
        }
    });
});
