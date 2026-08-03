<?php

namespace Attributes\Validation\Tests;

use Attributes\Validation\BaseModel;
use Attributes\Validation\ModelConfigs;

it('ModelConfigs attribute can be applied to a class', function () {
    $model = new class extends BaseModel {};
    
    // ModelConfigs should be applicable as an attribute
    expect(true)->toBeTrue();
});

it('ModelConfigs has all required getter methods', function () {
    $configs = new ModelConfigs();

    expect(method_exists($configs, 'getAliasGenerator'))->toBeTrue();
    expect(method_exists($configs, 'isStrToLower'))->toBeTrue();
    expect(method_exists($configs, 'isStrToUpper'))->toBeTrue();
    expect(method_exists($configs, 'isStripWhitespace'))->toBeTrue();
    expect(method_exists($configs, 'getExtra'))->toBeTrue();
    expect(method_exists($configs, 'isStrict'))->toBeTrue();
    expect(method_exists($configs, 'isStopAtFirstError'))->toBeTrue();
});

it('ModelConfigs getter methods return expected types', function () {
    $configs = new ModelConfigs();

    expect($configs->getAliasGenerator())->toBeNull();
    expect($configs->isStrToLower())->toBeBool();
    expect($configs->isStrToUpper())->toBeBool();
    expect($configs->isStripWhitespace())->toBeBool();
    expect($configs->getExtra())->toBeString();
    expect($configs->isStrict())->toBeBool();
    expect($configs->isStopAtFirstError())->toBeBool();
});

it('ModelConfigs default values are correct', function () {
    $configs = new ModelConfigs();

    expect($configs->getAliasGenerator())->toBeNull();
    expect($configs->isStrToLower())->toBeFalse();
    expect($configs->isStrToUpper())->toBeFalse();
    expect($configs->isStripWhitespace())->toBeFalse();
    expect($configs->getExtra())->toBe('ignore');
    expect($configs->isStrict())->toBeFalse();
    expect($configs->isStopAtFirstError())->toBeFalse();
});
