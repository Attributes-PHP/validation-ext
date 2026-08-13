<?php

namespace Attributes\Validation\Tests\Integration\Validate\TypeHint;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;

use function Attributes\Validation\validate;

describe('type-hint validation (loose mode)', function () {
    describe('scalar types', function () {
        it('accepts valid string values', function () {
            $model = new class extends BaseModel
            {
                public string $value;
            };

            $values = [
                'Hello world!',
                '/[a-z]+/g',
                'test@test.com',
                'https://www.example.com/',
                123,
                123.5,
                -12,
                -0.05,
                true,
                false,
                '',
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeString();
            }
        });

        it('rejects invalid string values', function () {
            $model = new class extends BaseModel
            {
                public string $value;
            };

            $invalidValues = [
                [123],
                ['a' => 1, 'b' => 2],
                (object) [123],
                new class {},
                new \DateTime,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid integer values', function () {
            $model = new class extends BaseModel
            {
                public int $value;
            };

            $values = [
                '123',
                123,
                '2e3',
                2000,
                -12,
                0,
                '0',
                '000',
                true,
                false,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeInt();
            }
        });

        it('rejects invalid integer values', function () {
            $model = new class extends BaseModel
            {
                public int $value;
            };

            $invalidValues = [
                'hello world',
                [[5, 2, 1]],
                new class {},
                new \DateTime,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid float values', function () {
            $model = new class extends BaseModel
            {
                public float $value;
            };

            $values = [
                '123',
                123,
                123.5,
                -12,
                -0.05,
                true,
                false,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeFloat();
            }
        });

        it('rejects invalid float values', function () {
            $model = new class extends BaseModel
            {
                public float $value;
            };

            $invalidValues = [
                'hello world',
                [[5, 2, 1]],
                new class {},
                new \DateTime,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid bool values', function () {
            $model = new class extends BaseModel
            {
                public bool $value;
            };

            $values = [
                'true',
                'false',
                'True',
                'False',
                true,
                false,
                0,
                1,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeBool();
            }
        });

        it('rejects invalid bool values', function () {
            $model = new class extends BaseModel
            {
                public bool $value;
            };

            $invalidValues = [
                'hello',
                'yes',
                12345,
                [[1, 2, 3]],
                new class {},
                new \DateTime,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);
    });

    describe('DateTime types', function () {
        it('accepts valid DateTime values', function () {
            $model = new class extends BaseModel
            {
                public \DateTime $value;
            };

            $values = [
                '2025-03-06T08:57:06+00:00',
                '2050-12-06T00:00:03+00:00',
                '2013-06-23',
                new \DateTime,
                new \DateTime('2013-6-23'),
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeInstanceOf(\DateTime::class);
            }
        });

        it('rejects invalid DateTime values', function () {
            $model = new class extends BaseModel
            {
                public \DateTime $value;
            };

            $invalidValues = [
                '6/23/2013',
                '23.06. 2013',
                '2013/6/23',
                [[123]],
                123,
                true,
                false,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid DateTimeInterface values', function () {
            $model = new class extends BaseModel
            {
                public \DateTimeInterface $value;
            };

            $values = [
                '2025-03-06T08:57:06+00:00',
                new \DateTime,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeInstanceOf(\DateTimeInterface::class);
            }
        });

        it('rejects invalid DateTimeInterface values', function () {
            $model = new class extends BaseModel
            {
                public \DateTimeInterface $value;
            };

            $invalidValues = [
                '6/23/2013',
                [[123]],
                123,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);
    });

    describe('DateTime string conversion', function () {
        it('converts valid date strings to DateTime objects', function () {
            $model = new class extends BaseModel
            {
                public \DateTime $value;
            };

            $result = validate(['value' => '2025-03-06T08:57:06+00:00'], $model);
            expect($result->value)->toBeInstanceOf(\DateTime::class);
            expect($result->value->format('Y-m-d\TH:i:sP'))->toBe('2025-03-06T08:57:06+00:00');
        });

        it('converts valid date strings to DateTimeInterface', function () {
            $model = new class extends BaseModel
            {
                public \DateTimeInterface $value;
            };

            $result = validate(['value' => '2025-03-06T08:57:06+00:00'], $model);
            expect($result->value)->toBeInstanceOf(\DateTimeInterface::class);
            expect($result->value->format('Y-m-d\TH:i:sP'))->toBe('2025-03-06T08:57:06+00:00');
        });

        it('preserves DateTime objects as-is', function () {
            $date = new \DateTime('2025-03-06T08:57:06+00:00');
            $model = new class extends BaseModel
            {
                public \DateTime $value;
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
                public \DateTime $createdAt;
            };

            $result = validate([
                'name' => 'John Doe',
                'age' => '30',
                'score' => '95.5',
                'active' => 1,
                'createdAt' => '2025-01-01',
            ], $model);

            expect($result->name)->toBe('John Doe');
            expect($result->age)->toBe(30);
            expect($result->score)->toBe(95.5);
            expect($result->active)->toBe(true);
            expect($result->createdAt)->toBeInstanceOf(\DateTime::class);
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
