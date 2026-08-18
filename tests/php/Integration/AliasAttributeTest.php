<?php

namespace Attributes\Validation\Tests\Integration;

use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\Field;
use ReflectionClass;
use ReflectionFunction;

it('Alias is a valid PHP Attribute for class properties', function () {
    $reflection = new ReflectionClass(TestClassWithAliasProperty::class);
    $property = $reflection->getProperty('name');
    $allAttributes = $property->getAttributes();
    $instance = $allAttributes[0]->newInstance();

    expect($instance)->toBeInstanceOf(Alias::class);
    expect($instance)->toBeInstanceOf(Field::class);
    expect($instance->name)->toBe('user_name');
});

it('Alias is a valid PHP Attribute for function parameters', function () {
    $reflection = new ReflectionFunction(test_function_with_alias(...));
    $allParameters = $reflection->getParameters();
    $allAttributes = $allParameters[0]->getAttributes();
    $instance = $allAttributes[0]->newInstance();

    expect($instance)->toBeInstanceOf(Alias::class);
    expect($instance)->toBeInstanceOf(Field::class);
    expect($instance->name)->toBe('email_address');
});

class TestClassWithAliasProperty
{
    #[Alias('user_name')]
    public string $name;
}

function test_function_with_alias(#[Alias('email_address')] string $email)
{
    return $email;
}
