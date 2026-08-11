<?php

namespace Attributes\Validation\Tests\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\ModelConfigs;

use function Attributes\Validation\validate;

describe('validate function model configs handling', function () {
    it('uses default configs when no ModelConfigs attribute is present', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        $result = validate(['name' => 'John'], $model);
        expect($result->name)->toBe('John');
    });

    it('stopAtFirstError=true', function () {
        $model = new #[ModelConfigs(stopAtFirstError: true)] class extends BaseModel
        {
            public string $name;

            public string $email;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect(count($errors))->toBe(1);
        }
    });

    it('stopAtFirstError=false', function () {
        $model = new class extends BaseModel
        {
            public string $name;

            public string $email;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect(count($errors))->toBe(2);
            expect(array_key_exists('name', $errors))->toBeTrue();
            expect(array_key_exists('email', $errors))->toBeTrue();
        }
    });
});
