<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration;

use ArrayAccess;
use ArrayObject;
use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\ModelConfigs;
use Attributes\Validation\Type;
use Error;
use stdClass;
use TypeError;

use function Attributes\Validation\call;
use function Attributes\Validation\validate;

class CallFunctionUser extends BaseModel
{
    public int $age;

    public string $name;
}

#[ModelConfigs(strict: false)]
class CallFunctionLooseUser extends BaseModel
{
    public int $age;
}

class CallFunctionStrictUser extends BaseModel
{
    public int $age;
}

class CallFunctionAliasUser extends BaseModel
{
    #[Alias('user_name')]
    public string $name;
}

class CallFunctionTagsUser extends BaseModel
{
    #[Sequence(Type::string)]
    public array $tags;
}

class CallFunctionHooksUser extends BaseModel
{
    public string $name;

    protected function beforeValidation(array $rawData, ModelConfigs $configs): array
    {
        $rawData['name'] = 'hooked';

        return $rawData;
    }

    protected function afterValidation(array $rawData, ModelConfigs $configs): void
    {
        $this->name = $this->name.'-validated';
    }
}

class CallFunctionLazyContainer implements ArrayAccess
{
    public array $constructed = [];

    public function offsetExists(mixed $offset): bool
    {
        return in_array($offset, ['logger', 'db'], true);
    }

    public function offsetGet(mixed $offset): mixed
    {
        $this->constructed[] = $offset;

        $service = new stdClass;
        $service->name = "service-{$offset}";

        return $service;
    }

    public function offsetSet(mixed $offset, mixed $value): void {}

    public function offsetUnset(mixed $offset): void {}
}

function call_function_takes_model(CallFunctionUser $user): string
{
    return $user->name;
}

function call_function_returns_model(CallFunctionUser $user): CallFunctionUser
{
    return $user;
}

function call_function_takes_loose_user(CallFunctionLooseUser $user): int
{
    return $user->age;
}

function call_function_takes_strict_user(CallFunctionStrictUser $user): int
{
    return $user->age;
}

function call_function_takes_alias_user(CallFunctionAliasUser $user): string
{
    return $user->name;
}

function call_function_takes_tags_user(CallFunctionTagsUser $user): array
{
    return $user->tags;
}

function call_function_takes_hooks_user(CallFunctionHooksUser $user): string
{
    return $user->name;
}

function call_function_takes_model_and_dependency(CallFunctionUser $user, stdClass $service): string
{
    return $user->name.' from '.$service->name;
}

function call_function_takes_two_dependencies(stdClass $first, stdClass $second): string
{
    return $first->name.'+'.$second->name;
}

function call_function_takes_positional(string $first, int $second): string
{
    return $first.$second;
}

function call_function_takes_optional(string $first, string $second = 'default'): string
{
    return $first.$second;
}

function call_function_takes_variadic(string ...$values): array
{
    return $values;
}

function call_function_takes_service(stdClass $logger): string
{
    return $logger->name;
}

describe('call function model hydration', function () {
    it('hydrates a model parameter from the params array', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_model', [
            'name' => 'Andre',
            'age' => 30,
        ]);
        expect($result)->toBe('Andre');
    });

    it('accepts a closure as the callable', function () {
        $result = call(function (CallFunctionUser $user): string {
            return $user->name;
        }, ['name' => 'Andre', 'age' => 30]);
        expect($result)->toBe('Andre');
    });

    it('returns the callable return value', function () {
        $rawData = ['name' => 'Andre', 'age' => 30];
        expect(call('Attributes\Validation\Tests\Integration\call_function_returns_model', $rawData))->toEqual(validate(
            $rawData,
            new CallFunctionUser,
        ));
    });

    it('coerces scalars in loose mode like validate', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_loose_user', ['age' => '30']);
        expect($result)->toBe(30);
    });

    it('is strict by default like validate', function () {
        call('Attributes\Validation\Tests\Integration\call_function_takes_strict_user', ['age' => '30']);
    })->throws(ValidationException::class);

    it('reports field errors through ValidationException like validate', function () {
        try {
            call('Attributes\Validation\Tests\Integration\call_function_takes_strict_user', ['age' => '30']);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'age' => ['Must be integer'],
            ]);
        }
    });

    it('resolves the Alias attribute like validate', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_alias_user', [
            'user_name' => 'Andre',
        ]);
        expect($result)->toBe('Andre');
    });

    it('validates array type hints like validate', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_tags_user', ['tags' => ['a', 'b']]);
        expect($result)->toBe(['a', 'b']);

        try {
            call('Attributes\Validation\Tests\Integration\call_function_takes_tags_user', ['tags' => ['a', 1]]);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'tags.1' => ['Must be string'],
            ]);
        }
    });

    it('runs the model hooks like validate', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_hooks_user', ['name' => 'ignored']);
        expect($result)->toBe('hooked-validated');
    });
});

describe('call function dependencies', function () {
    it('matches dependencies by parameter name', function () {
        $first = new stdClass;
        $first->name = 'one';
        $second = new stdClass;
        $second->name = 'two';

        $result = call(
            'Attributes\Validation\Tests\Integration\call_function_takes_two_dependencies',
            [],
            [
                'second' => $second,
                'first' => $first,
            ],
        );
        expect($result)->toBe('one+two');
    });

    it('falls back to positional dependencies in order', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_positional', [], ['x', 7]);
        expect($result)->toBe('x7');
    });

    it('feeds internal functions positionally', function () {
        expect(call('str_repeat', [], ['x', 3]))->toBe('xxx');
    });

    it('hydrates models and injects dependencies in the same call', function () {
        $service = new stdClass;
        $service->name = 'svc';

        $result = call(
            'Attributes\Validation\Tests\Integration\call_function_takes_model_and_dependency',
            ['name' => 'Andre', 'age' => 30],
            [
                'service' => $service,
            ],
        );
        expect($result)->toBe('Andre from svc');
    });

    it('applies the declared default when a dependency is missing', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_optional', [], ['first' => 'x']);
        expect($result)->toBe('xdefault');
    });

    it('throws an Error when a required dependency is missing', function () {
        call(
            'Attributes\Validation\Tests\Integration\call_function_takes_two_dependencies',
            [],
            ['first' => new stdClass],
        );
    })->throws(
        Error::class,
        'No value provided for parameter $second of Attributes\Validation\Tests\Integration\call_function_takes_two_dependencies().',
    );

    it('passes the leftover positional dependencies to a variadic target', function () {
        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_variadic', [], ['a', 'b', 'c']);
        expect($result)->toBe(['a', 'b', 'c']);
    });
});

describe('call function ArrayAccess dependencies', function () {
    it('matches dependencies by parameter name on an ArrayObject', function () {
        $result = call(
            'Attributes\Validation\Tests\Integration\call_function_takes_two_dependencies',
            [],
            new ArrayObject([
                'second' => service('two'),
                'first' => service('one'),
            ]),
        );
        expect($result)->toBe('one+two');
    });

    it('falls back to positional entries of an ArrayObject', function () {
        $result = call(
            'Attributes\Validation\Tests\Integration\call_function_takes_positional',
            [],
            new ArrayObject(['x', 7]),
        );
        expect($result)->toBe('x7');
    });

    it('passes the leftover positional entries of an ArrayObject to a variadic target', function () {
        $result = call(
            'Attributes\Validation\Tests\Integration\call_function_takes_variadic',
            [],
            new ArrayObject(['a', 'b', 'c']),
        );
        expect($result)->toBe(['a', 'b', 'c']);
    });

    it('constructs only the dependencies the call consumes', function () {
        $container = new CallFunctionLazyContainer;

        $result = call('Attributes\Validation\Tests\Integration\call_function_takes_service', [], $container);
        expect($result)->toBe('service-logger');
        expect($container->constructed)->toBe(['logger']);
    });

    it('propagates a container exception', function () {
        $container = new class implements ArrayAccess {
            public function offsetExists(mixed $offset): bool
            {
                return true;
            }

            public function offsetGet(mixed $offset): mixed
            {
                throw new Error('container failure');
            }

            public function offsetSet(mixed $offset, mixed $value): void {}

            public function offsetUnset(mixed $offset): void {}
        };

        call('Attributes\Validation\Tests\Integration\call_function_takes_service', [], $container);
    })->throws(Error::class, 'container failure');

    it('throws a TypeError when the dependencies are not an array or ArrayAccess', function ($dependencies) {
        call('Attributes\Validation\Tests\Integration\call_function_takes_positional', [], $dependencies);
    })->with([
        42,
        new stdClass,
        'deps',
    ])->throws(TypeError::class);
});

function service(string $name): stdClass
{
    $service = new stdClass;
    $service->name = $name;

    return $service;
}
