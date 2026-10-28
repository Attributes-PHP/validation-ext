<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Fields;

use Attributes\Validation\Fields\ConfigAwareConstraint;
use Attributes\Validation\Fields\Constraint;
use Attributes\Validation\ModelConfigs;
use ReflectionClass;
use ReflectionMethod;
use TypeError;

describe('ConfigAwareConstraint interface', function () {
    it('is registered and extends Constraint', function () {
        expect(interface_exists(ConfigAwareConstraint::class))->toBeTrue();

        $reflection = new ReflectionClass(ConfigAwareConstraint::class);
        expect($reflection->isInterface())->toBeTrue();
        expect(is_subclass_of(ConfigAwareConstraint::class, Constraint::class))->toBeTrue();
    });

    it('declares setModelConfigs(ModelConfigs): void', function () {
        $method = new ReflectionMethod(ConfigAwareConstraint::class, 'setModelConfigs');

        expect($method->isPublic())->toBeTrue();
        expect($method->getReturnType()->__toString())->toBe('void');
        expect($method->getNumberOfParameters())->toBe(1);
        expect($method->getNumberOfRequiredParameters())->toBe(1);

        $parameter = $method->getParameters()[0];
        expect($parameter->getName())->toBe('configs');
        expect($parameter->getType()->__toString())->toBe(ModelConfigs::class);
    });

    it('forces implementing classes to define all contract methods', function () {
        $class = new class implements ConfigAwareConstraint {
            public function setModelConfigs(ModelConfigs $configs): void {}

            public function validate(mixed $value): bool
            {
                return true;
            }

            public function message(): string
            {
                return 'no error';
            }
        };

        expect($class)->toBeInstanceOf(ConfigAwareConstraint::class);
        expect($class)->toBeInstanceOf(Constraint::class);

        $class->setModelConfigs(new ModelConfigs);
        expect($class->validate(null))->toBeTrue();
    });

    it('rejects a non-ModelConfigs argument to setModelConfigs', function () {
        $class = new class implements ConfigAwareConstraint {
            public function setModelConfigs(ModelConfigs $configs): void {}

            public function validate(mixed $value): bool
            {
                return true;
            }

            public function message(): string
            {
                return 'no error';
            }
        };

        expect(fn () => $class->setModelConfigs('not configs'))->toThrow(TypeError::class);
    });
});
