<?php

namespace Attributes\Validation\Tests;

use Attributes\Validation\BaseModel;
use Attributes\Validation\ModelConfigs;
use Attributes\Validation\Exceptions\ValidationException;

use function Attributes\Validation\validate;

// ============= Basic Type Validation Tests =============

it('validates and coerces integer properties', function () {
    $model = new class extends BaseModel {
        public int $age;
    };

    $result = validate(['age' => '25'], $model);
    
    expect($result->age)->toBeInt()->toBe(25);
});

it('validates and coerces float properties', function () {
    $model = new class extends BaseModel {
        public float $price;
    };

    $result = validate(['price' => '19.99'], $model);
    
    expect($result->price)->toBeFloat()->toBe(19.99);
});

it('validates and coerces string properties', function () {
    $model = new class extends BaseModel {
        public string $name;
    };

    $result = validate(['name' => 123], $model);
    
    expect($result->name)->toBeString()->toBe('123');
});

it('validates and coerces boolean properties', function () {
    $model = new class extends BaseModel {
        public bool $active;
    };

    $result = validate(['active' => '1'], $model);
    
    expect($result->active)->toBeBool()->toBeTrue();
    
    $result2 = validate(['active' => '0'], $model);
    expect($result2->active)->toBeBool()->toBeFalse();
});

// ============= Nullable Type Tests =============

it('validates nullable properties with null value', function () {
    $model = new class extends BaseModel {
        public ?string $name;
    };

    $result = validate(['name' => null], $model);
    
    expect($result->name)->toBeNull();
});

it('validates nullable properties with actual value', function () {
    $model = new class extends BaseModel {
        public ?string $name;
    };

    $result = validate(['name' => 'John'], $model);
    
    expect($result->name)->toBeString()->toBe('John');
});

it('fails validation when nullable property is missing and not nullable', function () {
    $model = new class extends BaseModel {
        public string $name;
    };

    try {
        validate([], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toHaveKey('name');
    }
});

// ============= Strict Mode Tests =============

it('fails validation in strict mode when types do not match', function () {
    $model = new class extends BaseModel {
        #[ModelConfigs(strict: true)]
        public int $age;
    };

    try {
        validate(['age' => '25'], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toHaveKey('age');
    }
});

it('passes validation in strict mode when types match', function () {
    $model = new class extends BaseModel {
        #[ModelConfigs(strict: true)]
        public int $age;
    };

    $result = validate(['age' => 25], $model);
    
    expect($result->age)->toBeInt()->toBe(25);
});

// ============= Multiple Properties Tests =============

it('validates multiple properties correctly', function () {
    $model = new class extends BaseModel {
        public int $age;
        public string $name;
        public float $price;
        public bool $active;
    };

    $result = validate([
        'age' => '30',
        'name' => 123,
        'price' => '29.99',
        'active' => '1'
    ], $model);
    
    expect($result->age)->toBeInt()->toBe(30);
    expect($result->name)->toBeString()->toBe('123');
    expect($result->price)->toBeFloat()->toBe(29.99);
    expect($result->active)->toBeBool()->toBeTrue();
});

// ============= Missing Required Fields Tests =============

it('fails when required field is missing', function () {
    $model = new class extends BaseModel {
        public string $name;
        public int $age;
    };

    try {
        validate(['name' => 'John'], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toHaveKey('age');
    }
});

it('fails when multiple required fields are missing', function () {
    $model = new class extends BaseModel {
        public string $name;
        public int $age;
        public float $price;
    };

    try {
        validate([], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        $errors = $e->getErrors();
        expect($errors)->toHaveKey('name');
        expect($errors)->toHaveKey('age');
        expect($errors)->toHaveKey('price');
    }
});

// ============= Stop at First Error Tests =============

it('stops at first error when configured', function () {
    $model = new class extends BaseModel {
        #[ModelConfigs(stopAtFirstError: true)]
        public string $name;
        public int $age;
    };

    try {
        validate([], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        $errors = $e->getErrors();
        // Should only have one error due to stopAtFirstError
        expect(count($errors))->toBe(1);
    }
});

it('collects all errors when not configured to stop', function () {
    $model = new class extends BaseModel {
        #[ModelConfigs(stopAtFirstError: false)]
        public string $name;
        public int $age;
    };

    try {
        validate([], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        $errors = $e->getErrors();
        // Should have both errors
        expect(count($errors))->toBe(2);
    }
});

// ============= Inheritance Tests =============

class ParentModel extends BaseModel
{
    public string $parentField;
}

it('validates inherited properties', function () {
    $model = new class extends ParentModel {
        public int $childField;
    };

    $result = validate([
        'parentField' => 'parent',
        'childField' => '42'
    ], $model);
    
    expect($result->parentField)->toBeString()->toBe('parent');
    expect($result->childField)->toBeInt()->toBe(42);
});

// ============= Alias Tests =============

use Attributes\Validation\Fields\Alias;

it('validates with alias attribute', function () {
    $model = new class extends BaseModel {
        #[Alias('user_name')]
        public string $name;
    };

    $result = validate(['user_name' => 'John'], $model);
    
    expect($result->name)->toBeString()->toBe('John');
});

it('fails when using property name instead of alias', function () {
    $model = new class extends BaseModel {
        #[Alias('user_name')]
        public string $name;
    };

    try {
        validate(['name' => 'John'], $model);
        expect(false)->toBeTrue('Expected ValidationException to be thrown');
    } catch (ValidationException $e) {
        expect($e->getErrors())->toHaveKey('user_name');
    }
});

// ============= ModelConfigs Tests =============

it('validates with aliasGenerator in ModelConfigs', function () {
    $model = new class extends BaseModel {
        #[ModelConfigs(aliasGenerator: 'snake')]
        public string $firstName;
    };

    $result = validate(['first_name' => 'John'], $model);
    
    expect($result->firstName)->toBeString()->toBe('John');
});
