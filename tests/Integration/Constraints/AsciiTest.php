<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Constraints\Ascii;

use function Attributes\Validation\validate;

describe('Ascii constraint', function () {
    it('accepts values containing only 7-bit ASCII characters', function (string $value) {
        $model = new class extends BaseModel {
            #[Ascii]
            public string $value;
        };

        $result = validate(['value' => $value], $model);
        expect($result->value)->toBe($value);
    })->with([
        'plain text',
        'ABC-123_abc',
        '',
        "tabs\tand\nnewlines",
        "\x7F",
    ]);

    it('reports an error for values containing bytes outside ASCII', function (string $value) {
        $model = new class extends BaseModel {
            #[Ascii]
            public string $value;
        };

        try {
            validate(['value' => $value], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must only contain ASCII characters']]);
        }
    })->with([
        'café',
        '日本語',
        "\x80\x81",
        'em—dash',
    ]);

    it('reports an error for non-string values', function () {
        $model = new class extends BaseModel {
            #[Ascii]
            public $value;
        };

        try {
            validate(['value' => ['a']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must only contain ASCII characters']]);
        }
    });
});
