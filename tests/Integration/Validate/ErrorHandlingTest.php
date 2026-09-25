<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Tests\Models\Enums\EnumBasicOneValue;
use Attributes\Validation\Tests\Models\Enums\EnumBasicThreeValues;
use Attributes\Validation\Tests\Models\Enums\EnumBasicTwentyValues;
use Attributes\Validation\Tests\Models\Enums\EnumBasicTwoValues;
use Attributes\Validation\Tests\Models\Enums\EnumIntOneValue;
use Attributes\Validation\Tests\Models\Enums\EnumIntThreeValues;
use Attributes\Validation\Tests\Models\Enums\EnumIntTwentyValues;
use Attributes\Validation\Tests\Models\Enums\EnumIntTwoValues;
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

class SecondBasicModel extends BaseModel
{
    public string $secondField;
}

class ThirdBasicModel extends BaseModel
{
    public string $thirdField;
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

    it('generates proper message for basic enums', function (BaseModel $model, string $expectedErrorMessage) {
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
                public EnumBasicTwentyValues $field;
            },
            "Should be 'One', 'Two', 'Three', 'Four', 'Five', 'Six', 'Seven', 'Eight', 'Nine', 'Ten', 'Eleven', 'Twelve', 'Thirteen', 'Fourteen', 'Fifteen', 'Sixteen', 'Seventeen', 'Eighteen', 'Nineteen' or 'Twenty'",
        ],
        'three options' => [
            new class extends BaseModel {
                public EnumBasicThreeValues $field;
            },
            "Should be 'One', 'Two' or 'Three'",
        ],
        'two options' => [
            new class extends BaseModel {
                public EnumBasicTwoValues $field;
            },
            "Should be 'One' or 'Two'",
        ],
        'single option' => [
            new class extends BaseModel {
                public EnumBasicOneValue $field;
            },
            "Should be 'One'",
        ],
    ]);

    it('generates proper message for string-backed enums', function (BaseModel $model, string $expectedErrorMessage) {
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

    it('generates proper message for int-backed enums', function (BaseModel $model, string $expectedErrorMessage) {
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
                public EnumIntTwentyValues $field;
            },
            'Should be 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 or 20',
        ],
        'three options' => [
            new class extends BaseModel {
                public EnumIntThreeValues $field;
            },
            'Should be 1, 2 or 3',
        ],
        'two options' => [
            new class extends BaseModel {
                public EnumIntTwoValues $field;
            },
            'Should be 1 or 2',
        ],
        'single option' => [
            new class extends BaseModel {
                public EnumIntOneValue $field;
            },
            'Should be 1',
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

    it('generates proper message for basic unions', function (BaseModel $model, string $expectedErrorMessage) {
        try {
            validate(['field' => (object) []], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            $errors = $e->getErrors();
            expect($errors)->toBe(['field' => [$expectedErrorMessage]]);
        }
    })->with([
        'two-type-hints' => [
            'model' => new class extends BaseModel {
                public bool|int $field;
            },
            'expectedErrorMessage' => 'Must be boolean or integer',
        ],
        'three-type-hints' => [
            'model' => new class extends BaseModel {
                public bool|int|float $field;
            },
            'expectedErrorMessage' => 'Must be boolean, integer or float',
        ],
        'five-type-hints' => [
            'model' => new class extends BaseModel {
                public bool|int|float|string|array $field;
            },
            'expectedErrorMessage' => 'Must be boolean, integer, float, string or array',
        ],
    ]);

    it('generates proper message for class-only unions', function (BaseModel $model, array $expectedErrors) {
        try {
            validate(['field' => ['invalid' => 'bro']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            $errors = $e->getErrors();
            expect($errors)->toBe($expectedErrors);
        }
    })->with([
        'two-type-hints' => [
            'model' => new class extends BaseModel {
                public BasicModel|SecondBasicModel $field;
            },
            'expectedErrors' => [
                'field.field' => ['Field is required'],
                'field.secondField' => ['Field is required'],
                'field' => [
                    'Must be an Attributes\Validation\Tests\Integration\Validate\BasicModel or an Attributes\Validation\Tests\Integration\Validate\SecondBasicModel',
                ],
            ],
        ],
        'three-type-hints' => [
            'model' => new class extends BaseModel {
                public BasicModel|SecondBasicModel|ThirdBasicModel $field;
            },
            'expectedErrors' => [
                'field.field' => ['Field is required'],
                'field.secondField' => ['Field is required'],
                'field.thirdField' => ['Field is required'],
                'field' => [
                    'Must be an Attributes\Validation\Tests\Integration\Validate\BasicModel or an Attributes\Validation\Tests\Integration\Validate\SecondBasicModel or an Attributes\Validation\Tests\Integration\Validate\ThirdBasicModel',
                ],
            ],
        ],
    ]);

    it('generates proper message for basic and class unions', function (BaseModel $model, array $expectedErrors) {
        try {
            validate(['field' => ['invalid' => 'bro']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            $errors = $e->getErrors();
            var_dump($errors['field'][0]);
            expect($errors)->toBe($expectedErrors);
        }
    })->with([
        'two-type-hints' => [
            'model' => new class extends BaseModel {
                public BasicModel|bool $field;
            },
            'expectedErrors' => [
                'field.field' => ['Field is required'],
                'field' => ['Must be boolean or Attributes\Validation\Tests\Integration\Validate\BasicModel'],
            ],
        ],
        'five-type-hints' => [
            'model' => new class extends BaseModel {
                public bool|int|float|BasicModel|SecondBasicModel $field;
            },
            'expectedErrors' => [
                'field.field' => ['Field is required'],
                'field.secondField' => ['Field is required'],
                'field' => ['Must be boolean, integer, float, Attributes\Validation\Tests\Integration\Validate\BasicModel or Attributes\Validation\Tests\Integration\Validate\SecondBasicModel'],
            ],
        ],
    ]);
});
