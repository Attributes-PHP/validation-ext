<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Tests\Models\Enums\EnumStrOneValue;
use Attributes\Validation\Tests\Models\Enums\EnumStrThreeValues;
use Attributes\Validation\Tests\Models\Enums\EnumStrTwentyValues;
use Attributes\Validation\Tests\Models\Enums\EnumStrTwoValues;

use function Attributes\Validation\validate;

// Define test classes for nested model validation

class BasicModel extends BaseModel
{
    public string $field;
}

// Multi-level nested model

class OneLevelNestedModel extends BaseModel
{
    public BasicModel $one;
}

class TwoLevelNestedModel extends BaseModel
{
    public OneLevelNestedModel $two;
}

class ThreeLevelNestedModel extends BaseModel
{
    public TwoLevelNestedModel $three;
}

describe('validate function error handling', function () {
    it('throws ValidationException for missing required fields', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        validate([], $model);
    })->throws(ValidationException::class);

    it('throws ValidationException with error details', function () {
        $model = new class extends BaseModel {
            public string $name;

            public string $email;
        };

        try {
            validate(['name' => 'John'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect($errors)->toBeArray()
                ->toHaveCount(1);
        }
    });

    it('includes error message for required field', function () {
        $model = new class extends BaseModel {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            $errors = $e->getErrors();
            expect($errors)->toBe([
                'name' => ['Field is required'],
            ]);
        }
    });

    it('includes error messages for multiple fields', function () {
        $model = new class extends BaseModel {
            public string $name;
            public string $email;
            public int $age;
            public bool $isAdmin;
            public float $timestamp;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            $errors = $e->getErrors();
            $allFields = ['name', 'email', 'age', 'isAdmin', 'timestamp'];
            expect($errors)->toHaveCount(count($allFields))
                ->toHaveKeys($allFields);

            foreach ($allFields as $name) {
                expect($errors)->toMatchArray([$name => ['Field is required']]);
            }
        }
    });

    it('generates proper message for string enums', function (BaseModel $model, string $expectedErrorMessage) {
        try {
            validate(['field' => 'invalid'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            expect($e->getErrors())->toBe([
                'field' => [$expectedErrorMessage],
            ]);
        }
    })->with([
        '20 options' => [
            new class extends BaseModel {
                public EnumStrTwentyValues $field;
            },
            "Should be 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten', 'eleven', 'twelve', 'thirteen', 'fourteen', 'fifteen', 'sixteen', 'seventeen', 'eighteen', 'nineteen' or 'twenty'",
        ],
        'three options' => [
            new class extends BaseModel {
                public EnumStrThreeValues $field;
            },
            "Should be 'one', 'two' or 'three'",
        ],
        'two options' => [
            new class extends BaseModel {
                public EnumStrTwoValues $field;
            },
            "Should be 'one' or 'two'",
        ],
        'single option' => [
            new class extends BaseModel {
                public EnumStrOneValue $field;
            },
            "Should be 'one'",
        ],
    ]);

    it('generates proper message for nested models', function (BaseModel $model, array $input, array $expectedErrors) {
        try {
            validate($input, $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            expect($e->getErrors())->toBe($expectedErrors);
        }
    })->with([
        'one-level' => [
            'model' => new OneLevelNestedModel,
            'input' => ['one' => []],
            'expectedErrors' => ['one.field' => ['Field is required']],
        ],
        'two-level' => [
            'model' => new TwoLevelNestedModel,
            'input' => ['two' => ['one' => []]],
            'expectedErrors' => ['two.one.field' => ['Field is required']],
        ],
        'three-level' => [
            'model' => new ThreeLevelNestedModel,
            'input' => ['three' => ['two' => ['one' => []]]],
            'expectedErrors' => ['three.two.one.field' => ['Field is required']],
        ],
    ]);
});
