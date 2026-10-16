<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Regex;
use TypeError;

use function Attributes\Validation\validate;

describe('Regex constructor argument validation', function () {
    it('promotes regex to a public property', function () {
        $constraint = new Regex('/^[a-z]+$/');

        expect($constraint->regex)->toBe('/^[a-z]+$/');
    });

    it('throws a TypeError for non-string patterns', function () {
        expect(fn () => new Regex(123))->toThrow(TypeError::class);
    });
});

describe('Regex validation', function () {
    it('accepts strings matching the pattern', function (string $value) {
        $model = new class extends BaseModel {
            #[Regex('/^[a-z]+$/')]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with(['abc', 'a', 'abcdefghijklmnopqrstuvwxyz']);

    it('reports an error for non-matching strings', function (string $value) {
        $model = new class extends BaseModel {
            #[Regex('/^[a-z]+$/')]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must match the pattern '/^[a-z]+$/'"]]);
        }
    })->with(['ABC', 'abc1', 'a b', '']);

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[Regex('/x/')]
            public $value;
        };

        try {
            validate(['value' => 123], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must match the pattern '/x/'"]]);
        }
    });

    it('reports an error for an invalid pattern, like preg_match', function () {
        $model = new class extends BaseModel {
            #[Regex('/(/')]
            public string $value;
        };

        // The PCRE compilation warning is part of the behavior: swallow it
        // so PHPUnit does not turn it into a test warning.
        set_error_handler(static fn (): bool => true);
        try {
            try {
                validate(['value' => 'x'], $model);
                expect(false)->toBeTrue();
            } catch (ValidationException $e) {
                expect($e->getErrors())->toBe(['value' => ["Must match the pattern '/(/'"]]);
            }
        } finally {
            restore_error_handler();
        }
    });
});
