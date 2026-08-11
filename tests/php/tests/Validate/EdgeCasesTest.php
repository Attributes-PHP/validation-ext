<?php

namespace Attributes\Validation\Tests\Validate;

use Attributes\Validation\BaseModel;

use function Attributes\Validation\validate;

describe('validate function edge cases', function () {
    it('handles empty raw data array', function () {
        $model = new class extends BaseModel
        {
            public string $name = 'default';
        };

        $result = validate([], $model);
        expect($result->name)->toBe('default');
    });

    it('handles model with no properties', function () {
        $model = new class extends BaseModel {};

        $result = validate([], $model);
        expect($result)->toBeInstanceOf(BaseModel::class);
    });

    it('handles raw data with extra fields not in model', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        $result = validate(['name' => 'John', 'extra' => 'ignored'], $model);
        expect($result->name)->toBe('John');
    });
});
