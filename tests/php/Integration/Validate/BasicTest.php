<?php

namespace Attributes\Validation\Tests\Integration\Validate;

use Attributes\Validation\BaseModel;

use function Attributes\Validation\validate;

describe('validate function basic functionality', function () {
    it('has validate function in Attributes\Validation namespace', function () {
        expect(function_exists('Attributes\Validation\validate'))->toBeTrue();
    });

    it('accepts valid array and model parameters', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        $result = validate(['name' => 'test'], $model);
        expect($result)->toBeInstanceOf(BaseModel::class);
    });

    it('returns the model instance', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        $result = validate(['name' => 'test'], $model);
        expect($result)->toBe($model);
    });

    it('populates model properties from raw data', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        $result = validate(['name' => 'John'], $model);
        expect($result->name)->toBe('John');
    });

    it('handles multiple properties', function () {
        $model = new class extends BaseModel
        {
            public string $name;

            public int $age;
        };

        $result = validate(['name' => 'John', 'age' => 30], $model);
        expect($result->name)->toBe('John');
        expect($result->age)->toBe(30);
    });

    it('ignores private and protected properties', function () {
        $model = new class extends BaseModel
        {
            public string $name = '<default>';

            private int $private = 10;

            protected int $protected = 10;

            public function getPrivate(): int
            {
                return $this->private;
            }

            public function getProtected(): int
            {
                return $this->protected;
            }
        };

        $result = validate(['name' => 'John', 'private' => 30, 'protected' => 30], $model);
        expect($result->name)->toBe('John');
        expect($result->getPrivate())->toBe(10);
        expect($result->getProtected())->toBe(10);
    });

    it('ignores static properties', function () {
        $model = new class extends BaseModel
        {
            public static string $public = '<default>';

            private static int $private = 10;

            protected static int $protected = 10;

            public function getPrivate(): int
            {
                return $this::$private;
            }

            public function getProtected(): int
            {
                return $this::$protected;
            }
        };

        $result = validate(['public' => 'John', 'private' => 30, 'protected' => 30], $model);
        expect($result::$public)->toBe('<default>');
        expect($result->getPrivate())->toBe(10);
        expect($result->getProtected())->toBe(10);
    });
});
