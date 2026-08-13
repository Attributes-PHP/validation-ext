<?php

namespace Attributes\Validation\Tests\Integration\Validate\TypeHint;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\ModelConfigs;
use DateTime;
use DateTimeInterface;

use function Attributes\Validation\validate;

describe('type-hint validation (strict mode)', function () {
    describe('scalar types', function () {
        it('accepts valid string values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public string $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeString();
        })->with('valid string strict');

        it('rejects invalid string values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public string $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid string strict')->throws(ValidationException::class);

        it('accepts valid integer values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public int $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeInt();
        })->with('valid integer strict');

        it('rejects invalid integer values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public int $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid integer strict')->throws(ValidationException::class);

        it('accepts valid float values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public float $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeFloat();
        })->with('valid float strict');

        it('rejects invalid float values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public float $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid float strict')->throws(ValidationException::class);

        it('accepts valid bool values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public bool $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeBool();
        })->with('valid bool strict');

        it('rejects invalid bool values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public bool $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid bool strict')->throws(ValidationException::class);
    });

    describe('DateTime types', function () {
        it('accepts valid DateTime values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public DateTime $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeInstanceOf(DateTime::class);
        })->with('valid datetime strict');

        it('rejects invalid DateTime values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public DateTime $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid datetime strict')->throws(ValidationException::class);

        it('accepts valid DateTimeInterface values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public DateTimeInterface $value;
            };

            $result = validate(['value' => $value], $model);
            expect($result->value)->toBeInstanceOf(DateTimeInterface::class);
        })->with('valid datetimeinterface strict');

        it('rejects invalid DateTimeInterface values in strict mode', function ($value) {
            $model = new #[ModelConfigs(strict: true)] class extends BaseModel
            {
                public DateTimeInterface $value;
            };

            validate(['value' => $value], $model);
        })->with('invalid datetimeinterface strict')->throws(ValidationException::class);
    });
});
