<?php

namespace Attributes\Validation\Tests\Integration\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\ModelConfigs;

use function Attributes\Validation\validate;

describe('validate function hook handling', function () {
    it('no hooks called', function () {
        $model = validate(['number' => '1.23'], new NoHooks);
        expect($model->calledBeforeValidation)->toBeFalse();
        expect($model->calledAfterValidation)->toBeFalse();
    });

    it('calls beforeValidation before validation', function () {
        $model = new class extends NoHooks
        {
            public function beforeValidation(array $rawData, ModelConfigs $configs): array
            {
                expect(isset($this->number))->toBeFalse();
                expect($this->calledBeforeValidation)->toBeFalse();
                expect($this->calledAfterValidation)->toBeFalse();

                $this->calledBeforeValidation = true;

                return $rawData;
            }
        };

        validate(['number' => '1.23'], $model);
        expect($model->number)->toBe('1.23');
        expect($model->calledBeforeValidation)->toBeTrue();
        expect($model->calledAfterValidation)->toBeFalse();
    });

    it('calls afterValidation after validation', function () {
        $model = new class(['number' => '1.23']) extends NoHooks
        {
            public function afterValidation(array $rawData, ModelConfigs $configs): void
            {
                expect($this->number)->toBe('1.23');
                expect($this->calledBeforeValidation)->toBeFalse();
                expect($this->calledAfterValidation)->toBeFalse();

                $this->calledAfterValidation = true;
            }
        };

        validate(['number' => '1.23'], $model);
        expect($model->number)->toBe('1.23');
        expect($model->calledBeforeValidation)->toBeFalse();
        expect($model->calledAfterValidation)->toBeTrue();
    });

    it('calls both beforeValidation and afterValidation', function () {
        $model = new class(['number' => '1.23']) extends NoHooks
        {
            public function beforeValidation(array $rawData, ModelConfigs $configs): array
            {
                expect(isset($this->number))->toBeFalse();
                expect($this->calledBeforeValidation)->toBeFalse();
                expect($this->calledAfterValidation)->toBeFalse();

                $this->calledBeforeValidation = true;

                return $rawData;
            }

            public function afterValidation(array $rawData, ModelConfigs $configs): void
            {
                expect($this->number)->toBe('1.23');
                expect($this->calledBeforeValidation)->toBeTrue();
                expect($this->calledAfterValidation)->toBeFalse();

                $this->calledAfterValidation = true;
            }
        };

        $hello = validate(['number' => '1.23'], $model);
        expect($model->calledBeforeValidation)->toBeTrue();
        expect($model->calledAfterValidation)->toBeTrue();
    });

    it('does not call afterValidation when validation fails', function () {
        $model = new class(['number' => '1.23']) extends NoHooks
        {
            public function afterValidation(array $rawData, ModelConfigs $configs): void
            {
                $this->calledAfterValidation = true;
            }
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($model->calledAfterValidation)->toBeFalse();
        }
    });

    it('passes raw data to beforeValidation and afterValidation', function () {
        $model = new class extends NoHooks
        {
            private array $expectedRawData = ['number' => '1.23'];

            public function beforeValidation(array $rawData, ModelConfigs $configs): array
            {
                expect($rawData)->toBe($this->expectedRawData);

                return $rawData;
            }

            public function afterValidation(array $rawData, ModelConfigs $configs): void
            {
                expect($rawData)->toBe($this->expectedRawData);
            }

            public function getExpectedRawData(): array
            {
                return $this->expectedRawData;
            }
        };

        validate($model->getExpectedRawData(), $model);
    });

    it('changes raw data in beforeValidation', function () {
        $model = new class extends NoHooks
        {
            private array $expectedRawData = ['number' => '1.23'];

            public function beforeValidation(array $rawData, ModelConfigs $configs): array
            {
                expect($rawData)->toBe($this->expectedRawData);

                return ['number' => '20.99', 'hello' => true];
            }

            public function afterValidation(array $rawData, ModelConfigs $configs): void
            {
                expect($rawData)->not->toBe($this->expectedRawData);
                expect($rawData)->toMatchArray([
                    'number' => '20.99',
                    'hello' => true,
                ]);
            }

            public function getExpectedRawData(): array
            {
                return $this->expectedRawData;
            }
        };

        $result = validate($model->getExpectedRawData(), $model);
        expect($result->number)->toBe('20.99');
    });
});

class NoHooks extends BaseModel
{
    public bool $calledBeforeValidation = false;

    public bool $calledAfterValidation = false;

    public string $number;
}
