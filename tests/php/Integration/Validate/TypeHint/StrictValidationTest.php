<?php

namespace Attributes\Validation\Tests\Integration\Validate\TypeHint;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\ModelConfigs;

use function Attributes\Validation\validate;

describe('type-hint validation (strict mode)', function () {
    describe('scalar types', function () {
        it('accepts valid string values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public string $value;
            };

            $values = [
                'Hello world!',
                '/[a-z]+/g',
                'test@test.com',
                'https://www.example.com/',
                '',
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeString();
            }
        });

        it('rejects invalid string values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public string $value;
            };

            $invalidValues = [
                1,
                2.94,
                -30,
                -3.3,
                false,
                true,
                [[1, 2, 3]],
                (object) [[3, 3]],
                new \DateTime,
                new class {},
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid integer values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public int $value;
            };

            $values = [
                123,
                -12,
                2000,
                0,
                PHP_INT_MAX,
                PHP_INT_MIN,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeInt();
            }
        });

        it('rejects invalid integer values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public int $value;
            };

            $invalidValues = [
                '123',
                'hello world',
                [[5, 2, 1]],
                new class {},
                new \DateTime,
                true,
                false,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid float values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public float $value;
            };

            $values = [
                123.5,
                -12.82,
                -0.05,
                -10e5,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeFloat();
            }
        });

        it('rejects invalid float values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public float $value;
            };

            $invalidValues = [
                '123',
                '24.98',
                'hello world',
                [[5, 2, 1]],
                new class {},
                new \DateTime,
                true,
                false,
                null,
                123,
                -12,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid bool values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public bool $value;
            };

            $values = [
                true,
                false,
            ];

            foreach ($values as $value) {
                $result = validate(['value' => $value], $model);
                expect($result->value)->toBeBool();
            }
        });

        it('rejects invalid bool values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public bool $value;
            };

            $invalidValues = [
                'true',
                'false',
                'True',
                'False',
                0,
                1,
                'hello',
                12345,
                -982,
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
        it('accepts valid DateTime values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
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

        it('rejects invalid DateTime values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public \DateTime $value;
            };

            $invalidValues = [
                '6/23/2013',
                '23.06. 2013',
                '2013/6/23',
                [[123]],
                123,
                -124,
                18.2,
                true,
                false,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);

        it('accepts valid DateTimeInterface values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
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

        it('rejects invalid DateTimeInterface values in strict mode', function () {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public \DateTimeInterface $value;
            };

            $invalidValues = [
                '6/23/2013',
                [[123]],
                123,
                -124,
                18.2,
                true,
                false,
                null,
            ];

            foreach ($invalidValues as $value) {
                validate(['value' => $value], $model);
            }
        })->throws(ValidationException::class);
    });
});
