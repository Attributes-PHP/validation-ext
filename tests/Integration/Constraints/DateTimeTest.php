<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\DateTime;
use DateTimeZone;
use ReflectionClass;
use stdClass;
use TypeError;
use ValueError;

use function Attributes\Validation\validate;

describe('DateTime constraint constructor argument validation', function () {
    it('applies the stub defaults when no arguments are given', function () {
        $constraint = new DateTime;

        expect($constraint->format)->toBe('X-m-d\TH:i:sP')->and($constraint->timezone)->toBeNull();
    });

    it('accepts a string $format and stores it on the promoted property', function (string $format) {
        expect((new DateTime($format))->format)->toBe($format);
    })->with([
        'Y-m-d',
        'X-m-d\TH:i:sP',
        '',
    ]);

    it('accepts every supported $timezone type', function (DateTimeZone|string|null $timezone) {
        $constraint = new DateTime(timezone: $timezone);

        expect($constraint->timezone)->toBe($timezone);
    })->with([
        'UTC',
        null,
        new DateTimeZone('Europe/London'),
    ]);

    it('accepts both arguments together', function () {
        $timezone = new DateTimeZone('America/Sao_Paulo');
        $constraint = new DateTime('d/m/Y', $timezone);

        expect($constraint->format)->toBe('d/m/Y')->and($constraint->timezone)->toBe($timezone);
    });

    it('throws a TypeError when $format is not a string', function (mixed $format, string $message) {
        expect(fn () => new DateTime($format))->toThrow(TypeError::class, $message);
    })->with([
        'integer' => [123, 'Argument #1 ($format) must be of type string, int given'],
        'float' => [3.14, 'Argument #1 ($format) must be of type string, float given'],
        'null' => [null, 'Argument #1 ($format) must be of type string, null given'],
        'array' => [['Y-m-d'], 'Argument #1 ($format) must be of type string, array given'],
    ]);

    it('throws a TypeError when $timezone is not DateTimeZone, string or null', function (
        mixed $timezone,
        string $message,
    ) {
        expect(fn () => new DateTime(timezone: $timezone))->toThrow(TypeError::class, $message);
    })->with([
        'integer' => [123, 'Argument #2 ($timezone) must be of type DateTimeZone|string|null, int given'],
        'float' => [3.14, 'Argument #2 ($timezone) must be of type DateTimeZone|string|null, float given'],
        'array' => [['UTC'], 'Argument #2 ($timezone) must be of type DateTimeZone|string|null, array given'],
        'object' => [new stdClass, 'Argument #2 ($timezone) must be of type DateTimeZone|string|null, stdClass given'],
    ]);

    it('throws an ArgumentCountError when more than two arguments are given', function () {
        $construct = fn () => new DateTime('Y-m-d', 'UTC', 'extra');

        expect($construct)->toThrow(TypeError::class, 'expects at most 2 arguments, 3 given');
    });

    it('throws a ValueError when $timezone is an unconstructed DateTimeZone', function () {
        new DateTime(timezone: (new ReflectionClass(DateTimeZone::class))->newInstanceWithoutConstructor());
    })->throws(
        ValueError::class,
        'Argument #2 ($timezone) must be a constructed DateTimeZone object',
    );

    it('throws a ValueError when $timezone is an unknown timezone name', function (string $timezone) {
        expect(fn () => new DateTime(timezone: $timezone))->toThrow(
            ValueError::class,
            'Argument #2 ($timezone) must be a valid timezone identifier',
        );
    })->with([
        'Nope/Nope',
        'hello world',
        '',
    ]);
});

describe('DateTime validation', function () {
    it('accepts date strings parseable in the declared format', function () {
        $model = new class extends BaseModel {
            #[DateTime('Y-m-d')]
            public string $value;
        };

        $result = validate(['value' => '2026-10-16'], $model);
        expect($result->value)->toBe('2026-10-16');

        $model = new class extends BaseModel {
            #[DateTime('d/m/Y H:i')]
            public string $value;
        };

        $result = validate(['value' => '16/10/2026 09:00'], $model);
        expect($result->value)->toBe('16/10/2026 09:00');
    });

    it('reports an error for strings not parseable in the declared format', function (string $value) {
        $model = new class extends BaseModel {
            #[DateTime('Y-m-d')]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must be a datetime in the format 'Y-m-d'"]]);
        }
    })->with([
        '2026/10/16',
        'not a date',
        '16-10-2026',
        '',
    ]);

    it('applies the declared timezone', function () {
        $model = new class extends BaseModel {
            #[DateTime('Y-m-d H:i', timezone: 'Europe/London')]
            public string $value;
        };

        $result = validate(['value' => '2026-10-16 09:00'], $model);
        expect($result->value)->toBe('2026-10-16 09:00');
    });

    it('accepts a DateTimeZone instance as the timezone', function () {
        $model = new class extends BaseModel {
            #[DateTime('Y-m-d H:i', new DateTimeZone('America/Sao_Paulo'))]
            public string $value;
        };

        $result = validate(['value' => '2026-10-16 09:00'], $model);
        expect($result->value)->toBe('2026-10-16 09:00');
    });

    it('passes DateTimeInterface instances as-is', function () {
        $model = new class extends BaseModel {
            #[DateTime('Y-m-d')]
            public \DateTime $value;
        };

        $result = validate(['value' => new \DateTime('2026-10-16 09:00')], $model);
        expect($result->value)->toBeInstanceOf(\DateTime::class);
    });

    it('fails eagerly for an invalid timezone name in the attribute', function () {
        $model = new class extends BaseModel {
            #[DateTime(timezone: 'Nope/Nope')]
            public string $value;
        };

        expect(fn () => validate(['value' => '2026-10-16'], $model))->toThrow(
            ValueError::class,
            'Argument #2 ($timezone) must be a valid timezone identifier',
        );
    });
});
