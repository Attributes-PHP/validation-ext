<?php

namespace Attributes\Validation\Tests;

use Attributes\Validation\BaseModel;
use Attributes\Validation\ModelConfigs;

use function Attributes\Validation\validate;

it('calls no hooks when not defined', function () {
    $hooks = validate(['number' => '1.23'], new NoHooks);
    expect($hooks->calledBeforeValidation)->toBeFalse();
    expect($hooks->calledAfterValidation)->toBeFalse();
});

it('calls beforeValidation hook correctly', function () {
    $withBeforeHook = new class extends NoHooks
    {
        public function beforeValidation(array $rawData, ModelConfigs $configs): array
        {
            $this->calledBeforeValidation = true;

            return $rawData;
        }
    };

    validate(['number' => '1.23'], $withBeforeHook);
    expect($withBeforeHook->calledBeforeValidation)->toBeTrue();
    expect($withBeforeHook->calledAfterValidation)->toBeFalse();
});

it('calls afterValidation hook correctly', function () {
    $withAfterHook = new class(['number' => '1.23']) extends NoHooks
    {
        public function afterValidation(array $rawData, ModelConfigs $configs): void
        {
            $this->calledAfterValidation = true;
        }
    };

    validate(['number' => '1.23'], $withAfterHook);

    expect($withAfterHook->calledBeforeValidation)->toBeFalse();
    expect($withAfterHook->calledAfterValidation)->toBeTrue();
});

it('calls both beforeValidation and afterValidation hooks correctly', function () {
    $withBothHooks = new class(['number' => '1.23']) extends NoHooks
    {
        public function beforeValidation(array $rawData, ModelConfigs $configs): array
        {
            $this->calledBeforeValidation = true;

            return $rawData;
        }

        public function afterValidation(array $rawData, ModelConfigs $configs): void
        {
            $this->calledAfterValidation = true;
        }
    };

    $hello = validate(['number' => '1.23'], $withBothHooks);
    expect($withBothHooks->calledBeforeValidation)->toBeTrue();
    expect($withBothHooks->calledAfterValidation)->toBeTrue();
});

class NoHooks extends BaseModel
{
    public bool $calledBeforeValidation = false;

    public bool $calledAfterValidation = false;

    public string $number;
}
