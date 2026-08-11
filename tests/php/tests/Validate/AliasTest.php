<?php

namespace Attributes\Validation\Tests\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Alias;

use function Attributes\Validation\validate;

describe('validate function alias attribute handling', function () {
    it('uses Alias attribute for property name mapping', function () {
        $model = new class extends BaseModel
        {
            #[Alias('user_name')]
            public string $name;
        };

        $result = validate(['user_name' => 'John'], $model);
        expect($result->name)->toBe('John');
    });

    it('throws error when using property name instead of alias', function () {
        $model = new class extends BaseModel
        {
            #[Alias('user_name')]
            public string $name;
        };

        validate(['name' => 'John'], $model);
    })->throws(ValidationException::class);

    it('handles multiple aliased properties', function () {
        $model = new class extends BaseModel
        {
            #[Alias('user_name')]
            public string $name;

            #[Alias('user_email')]
            public string $email;
        };

        $result = validate(['user_name' => 'John', 'user_email' => 'john@example.com'], $model);
        expect($result->name)->toBe('John');
        expect($result->email)->toBe('john@example.com');
    });

    it('mixes aliased and non-aliased properties', function () {
        $model = new class extends BaseModel
        {
            #[Alias('user_name')]
            public string $name;

            public string $email;
        };

        $result = validate(['user_name' => 'John', 'email' => 'john@example.com'], $model);
        expect($result->name)->toBe('John');
        expect($result->email)->toBe('john@example.com');
    });
});
