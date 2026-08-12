<?php

namespace Attributes\Validation\Tests\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\ModelConfigs;
use ValueError;

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

    it('aliasGenerator=pascal transforms property names to PascalCase', function () {
        $model = new #[ModelConfigs(aliasGenerator: 'pascal')] class extends BaseModel
        {
            public string $firstName;

            public string $lastName;

            public int $user2Id;
        };

        $result = validate(['FirstName' => 'John', 'LastName' => 'Doe', 'User2Id' => 123], $model);
        expect($result->firstName)->toBe('John');
        expect($result->lastName)->toBe('Doe');
        expect($result->user2Id)->toBe(123);
    });

    it('aliasGenerator=camel transforms property names to camelCase', function () {
        $model = new #[ModelConfigs(aliasGenerator: 'camel')] class extends BaseModel
        {
            public string $firstName;

            public string $lastName;

            public int $user2Id;
        };

        $result = validate(['firstName' => 'John', 'lastName' => 'Doe', 'user2Id' => 123], $model);
        expect($result->firstName)->toBe('John');
        expect($result->lastName)->toBe('Doe');
        expect($result->user2Id)->toBe(123);
    });

    it('aliasGenerator=snake transforms property names to snake_case', function () {
        $model = new #[ModelConfigs(aliasGenerator: 'snake')] class extends BaseModel
        {
            public string $firstName;

            public string $lastName;

            public int $user2Id;
        };

        $result = validate(['first_name' => 'John', 'last_name' => 'Doe', 'user2_id' => 123], $model);
        expect($result->firstName)->toBe('John');
        expect($result->lastName)->toBe('Doe');
        expect($result->user2Id)->toBe(123);
    });

    it('aliasGenerator=kebab transforms property names to kebab-case', function () {
        $model = new #[ModelConfigs(aliasGenerator: 'kebab')] class extends BaseModel
        {
            public string $firstName;

            public string $lastName;

            public int $user2Id;
        };

        $result = validate(['first-name' => 'John', 'last-name' => 'Doe', 'user2-id' => 123], $model);
        expect($result->firstName)->toBe('John');
        expect($result->lastName)->toBe('Doe');
        expect($result->user2Id)->toBe(123);
    });

    it('aliasGenerator throws error when using wrong property name', function () {
        $model = new #[ModelConfigs(aliasGenerator: 'snake')] class extends BaseModel
        {
            public string $firstName;
        };

        validate(['firstName' => 'John'], $model);
    })->throws(ValidationException::class);

    it('aliasGenerator throws error when passing invalid option', function () {
        $model = new #[ModelConfigs(aliasGenerator: 'invalid')] class extends BaseModel
        {
            public string $name;
        };

        validate(['name' => 'test'], $model);
    })->throws(ValueError::class, 'Attributes\Validation\ModelConfigs::__construct(): Argument #1 must be of one of the following options: pascal, camel, snake, kebab');
});
