<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\Fields\Constraints\DateTime;
use DateTimeZone;
use ReflectionClass;
use stdClass;
use TypeError;
use ValueError;


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
});
