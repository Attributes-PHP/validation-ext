<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\Constraint;
use Attributes\Validation\Fields\Constraints\Min;
use RuntimeException;

use function Attributes\Validation\validate;

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER)]
final class Even implements Constraint
{
    public function validate(mixed $value): bool
    {
        return is_int($value) && ($value % 2) === 0;
    }

    public function message(): string
    {
        return 'Must be an even number';
    }
}

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER)]
final class LessThan implements Constraint
{
    public function __construct(
        private readonly int|float $bound,
    ) {}

    public function validate(mixed $value): bool
    {
        return (is_int($value) || is_float($value)) && $value < $this->bound;
    }

    public function message(): string
    {
        return "Must be less than {$this->bound}";
    }
}

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER)]
final class ContainsWord implements Constraint
{
    public function validate(mixed $value): bool
    {
        return is_string($value) && str_contains($value, $this->word);
    }

    public function message(): string
    {
        return '{field} must contain the word '.$this->word;
    }

    public function __construct(
        private readonly string $word,
    ) {}
}

#[Attribute(Attribute::TARGET_PROPERTY | Attribute::TARGET_PARAMETER | Attribute::IS_REPEATABLE)]
final class RequireKeyword implements Constraint
{
    public function validate(mixed $value): bool
    {
        return is_string($value) && str_contains($value, $this->keyword);
    }

    public function message(): string
    {
        return "Must contain the keyword '{$this->keyword}'";
    }

    public function __construct(
        private readonly string $keyword,
    ) {}
}

#[Attribute(Attribute::TARGET_PROPERTY)]
final class Throwing implements Constraint
{
    public function validate(mixed $value): bool
    {
        throw new RuntimeException('userland constraint failure');
    }

    public function message(): string
    {
        return 'unreachable';
    }
}

describe('userland constraints', function () {
    it('applies a userland constraint to a model property', function () {
        $model = new class extends BaseModel {
            #[Even]
            public int $value;
        };

        $result = validate(['value' => 4], $model);
        expect($result->value)->toBe(4);

        try {
            validate(['value' => 3], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be an even number']]);
        }
    });

    it('supports userland constraint constructor arguments, named arguments included', function () {
        $model = new class extends BaseModel {
            #[LessThan(bound: 10)]
            public $value;
        };

        $result = validate(['value' => 5], $model);
        expect($result->value)->toBe(5);

        try {
            validate(['value' => 10], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['Must be less than 10']]);
        }
    });

    it('substitutes the {field} and {value} placeholders in the message', function () {
        $model = new class extends BaseModel {
            #[ContainsWord(word: 'required-word')]
            public string $value;
        };

        try {
            validate(['value' => 'other text'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ['value must contain the word required-word']]);
        }
    });

    it('collects errors with builtin constraints in declaration order', function () {
        $model = new class extends BaseModel {
            #[Min(3), Even]
            public int $value;
        };

        try {
            validate(['value' => 1], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => [
                'Must be at least 3',
                'Must be an even number',
            ]]);
        }
    });

    it('keeps failing values out of the model', function () {
        $model = new class extends BaseModel {
            #[Even]
            public $value;
        };
        $model->value = 'sentinel';

        try {
            validate(['value' => 3], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($model->value)->toBe('sentinel');
        }
    });

    it('reports the error under the resolved alias name', function () {
        $model = new class extends BaseModel {
            #[Alias('even_value')]
            #[Even]
            public int $value;
        };

        try {
            validate(['even_value' => 3], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['even_value' => ['Must be an even number']]);
        }
    });

    it('supports repeatable userland constraints', function () {
        $model = new class extends BaseModel {
            #[RequireKeyword('alpha'), RequireKeyword('beta')]
            public string $value;
        };

        $result = validate(['value' => 'alpha beta'], $model);
        expect($result->value)->toBe('alpha beta');

        try {
            validate(['value' => 'alpha only'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe(['value' => ["Must contain the keyword 'beta'"]]);
        }
    });

    it('propagates exceptions thrown inside validate()', function () {
        $model = new class extends BaseModel {
            #[Throwing]
            public int $value;
        };

        expect(fn () => validate(['value' => 1], $model))
            ->toThrow(RuntimeException::class, 'userland constraint failure');
    });

    it('ignores attribute classes that do not implement Constraint', function () {
        $model = new class extends BaseModel {
            #[\Some\Documentation\Attribute\That\Does\Not\Exist]
            #[Even]
            public int $value;
        };

        $result = validate(['value' => 2], $model);
        expect($result->value)->toBe(2);
    });

    it('declares the interface contract enforced by PHP itself', function () {
        $reflection = new \ReflectionClass(Constraint::class);

        expect($reflection->isInterface())->toBeTrue()
            ->and($reflection->getMethod('validate')->isAbstract())
            ->toBeTrue()
            ->and($reflection->getMethod('message')->isAbstract())
            ->toBeTrue()
            ->and($reflection->getMethod('validate')->getParameters()[0]->getType()->__toString())
            ->toBe('mixed')
            ->and($reflection->getMethod('validate')->getReturnType()->__toString())
            ->toBe('bool')
            ->and($reflection->getMethod('message')->getReturnType()->__toString())
            ->toBe('string')
            ->and($reflection->implementsInterface(\Attributes\Validation\Fields\Field::class))
            ->toBeTrue();
    });
});
