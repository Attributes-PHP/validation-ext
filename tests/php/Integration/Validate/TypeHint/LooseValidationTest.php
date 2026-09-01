<?php

namespace Attributes\Validation\Tests\Integration\Validate\TypeHint;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use DateTime;
use DateTimeInterface;

use function Attributes\Validation\validate;

describe('type-hint validation (loose mode)', function () {
    describe('scalar types', function () {
        it('accepts valid string values', function ($value) {
            $model = new class extends BaseModel
            {
                public string $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeString();
        })->with('valid string loose');

        it('rejects invalid string values', function ($value) {
            $model = new class extends BaseModel
            {
                public string $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid string loose')->throws(ValidationException::class);

        it('accepts valid integer values', function ($value) {
            $model = new class extends BaseModel
            {
                public int $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeInt();
        })->with('valid integer loose');

        it('rejects invalid integer values', function ($value) {
            $model = new class extends BaseModel
            {
                public int $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid integer loose')->throws(ValidationException::class);

        it('accepts valid float values', function ($value) {
            $model = new class extends BaseModel
            {
                public float $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeFloat();
        })->with('valid float loose');

        it('rejects invalid float values', function ($value) {
            $model = new class extends BaseModel
            {
                public float $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid float loose')->throws(ValidationException::class);

        it('accepts valid bool values', function ($value) {
            $model = new class extends BaseModel
            {
                public bool $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeBool();
        })->with('valid bool loose');

        it('rejects invalid bool values', function ($value) {
            $model = new class extends BaseModel
            {
                public bool $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid bool loose')->throws(ValidationException::class);
    });

    describe('DateTime types', function () {
        it('accepts valid DateTime values', function ($value) {
            $model = new class extends BaseModel
            {
                public DateTime $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeInstanceOf(DateTime::class);
        })->with('valid datetime loose');

        it('rejects invalid DateTime values', function ($value) {
            $model = new class extends BaseModel
            {
                public DateTime $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid datetime loose')->throws(ValidationException::class);

        it('accepts valid DateTimeInterface values', function ($value) {
            $model = new class extends BaseModel
            {
                public DateTimeInterface $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeInstanceOf(DateTimeInterface::class);
        })->with('valid datetime loose');

        it('rejects invalid DateTimeInterface values', function ($value) {
            $model = new class extends BaseModel
            {
                public DateTimeInterface $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid datetime loose')->throws(ValidationException::class);
    });

    describe('DateTime string conversion', function () {
        it('converts valid date strings to DateTime objects', function () {
            $model = new class extends BaseModel
            {
                public DateTime $value;
            };

            $result = validate(['value' => '2025-03-06T08:57:06+00:00'], $model);
            expect($result->value)->toBeInstanceOf(DateTime::class);
            expect($result->value->format('Y-m-d\TH:i:sP'))->toBe('2025-03-06T08:57:06+00:00');
        });

        it('converts valid date strings to DateTimeInterface', function () {
            $model = new class extends BaseModel
            {
                public DateTimeInterface $value;
            };

            $result = validate(['value' => '2025-03-06T08:57:06+00:00'], $model);
            expect($result->value)->toBeInstanceOf(DateTimeInterface::class);
            expect($result->value->format('Y-m-d\TH:i:sP'))->toBe('2025-03-06T08:57:06+00:00');
        });

        it('preserves DateTime objects as-is', function () {
            $date = new DateTime('2025-03-06T08:57:06+00:00');
            $model = new class extends BaseModel
            {
                public DateTime $value;
            };

            $result = validate(['value' => $date], $model);
            expect($result->value)->toBe($date);
        });
    });

    describe('mixed properties', function () {
        it('validates multiple properties with different type hints', function () {
            $model = new class extends BaseModel
            {
                public string $name;

                public int $age;

                public float $score;

                public bool $active;

                public DateTime $createdAt;
            };

            $result = validate([
                'name' => 'John Doe',
                'age' => '30',
                'score' => '95.5',
                'active' => 1,
                'createdAt' => '2025-01-01T08:57:06Z',
            ], $model);

            expect($result->name)->toBe('John Doe');
            expect($result->age)->toBe(30);
            expect($result->score)->toBe(95.5);
            expect($result->active)->toBe(true);
            expect($result->createdAt)->toBeInstanceOf(DateTime::class);
        });

        it('fails when any property has invalid type', function () {
            $model = new class extends BaseModel
            {
                public string $name;

                public int $age;
            };

            validate([
                'name' => 'John Doe',
                'age' => 'not a number',
            ], $model);
        })->throws(ValidationException::class);
    });
});
