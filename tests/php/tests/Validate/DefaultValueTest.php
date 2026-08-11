<?php

namespace Attributes\Validation\Tests\Validate;

use Attributes\Validation\BaseModel;

use function Attributes\Validation\validate;

describe('validate function default value handling', function () {
    it('skips validation for properties with default values when field is missing', function () {
        $model = new class extends BaseModel
        {
            public string $name = 'default';
        };

        $result = validate([], $model);
        expect($result->name)->toBe('default');
    });

    it('uses provided value over default value', function () {
        $model = new class extends BaseModel
        {
            public string $name = 'default';
        };

        $result = validate(['name' => 'provided'], $model);
        expect($result->name)->toBe('provided');
    });

    it('handles multiple properties with default values', function () {
        $model = new class extends BaseModel
        {
            public string $name = 'default_name';

            public int $age = 25;
        };

        $result = validate([], $model);
        expect($result->name)->toBe('default_name');
        expect($result->age)->toBe(25);
    });

    it('mixes required and default value properties correctly', function () {
        $model = new class extends BaseModel
        {
            public string $name;

            public string $city = 'Unknown';
        };

        $result = validate(['name' => 'John'], $model);
        expect($result->name)->toBe('John');
        expect($result->city)->toBe('Unknown');
    });

    it('ignores default values when field is present', function () {
        $model = new class extends BaseModel
        {
            public string $name = 'default';

            public string $email = 'default@example.com';
        };

        $result = validate(['name' => 'John', 'email' => 'john@example.com'], $model);
        expect($result->name)->toBe('John');
        expect($result->email)->toBe('john@example.com');
    });
});
