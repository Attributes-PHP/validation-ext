<?php

namespace Attributes\Validation\Tests\Integration\Validate;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;

use function Attributes\Validation\validate;

// Define test classes for nested model validation
enum TestRole: string
{
    case Admin = 'admin';
    case User = 'user';
}

enum TeamRole: string
{
    case Coach = 'coach';
    case Player = 'player';
}

class TestAuthModel extends BaseModel
{
    public string $token;
}

class TestTeamAuthModel extends BaseModel
{
    public TeamRole $role;
}

class TestAuthModelWithEnum extends BaseModel
{
    public TestRole $role;
}

class TeamAuthModel extends BaseModel
{
    public TeamRole $role;
}

describe('validate function error handling', function () {
    it('throws ValidationException for missing required fields', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        validate([], $model);
    })->throws(ValidationException::class);

    it('throws ValidationException with error details', function () {
        $model = new class extends BaseModel
        {
            public string $name;

            public string $email;
        };

        try {
            validate(['name' => 'John'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e)->toBeInstanceOf(ValidationException::class);
            expect($e->getErrors())->toBeArray();
            expect(count($e->getErrors()))->toBeGreaterThan(0);
        }
    });

    it('includes error message for required field', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();
            expect($errors['name'][0] ?? null)->toContain('field is required');
        }
    });

    it('generates proper message for enum validation', function () {
        $model = new class extends BaseModel
        {
            public TestRole $role;
        };

        try {
            validate(['role' => 'invalid'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getMessage())->toBe('Invalid data');
            expect($e->getErrors()['role'][0])->toBe('The selected role is invalid.');
        }
    });

    it('generates proper message for nested model errors', function () {
        $model = new class extends BaseModel
        {
            public TestAuthModel $auth;
        };

        try {
            validate(['auth' => []], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toHaveKey('auth');
            expect($e->getErrors()['auth'][0])->toBe('The auth must be an Attributes\Validation\Tests\Integration\Validate\TestAuthModel.');
        }
    });

    it('uses proper articles in error messages', function () {
        $model = new class extends BaseModel
        {
            public int|float $value;
        };

        try {
            validate(['value' => 'invalid'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors()['value'][0])->toBe('The value must be an integer or a float.');
        }
    });

    it('generates proper message for required fields', function () {
        $model = new class extends BaseModel
        {
            public string $name;
        };

        try {
            validate([], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors()['name'][0])->toBe('The name field is required.');
        }
    });

    it('matches expected error format from documentation', function () {
        $team = new class extends BaseModel
        {
            public int|float $team_id;

            public TeamRole $role;
        };

        $rawData = [
            'team_id' => 'non-numeric',
            'role' => 'invalid_role',
        ];

        try {
            validate($rawData, $team);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            $errors = $e->getErrors();

            expect($errors)->toHaveKey('team_id');
            expect($errors)->toHaveKey('role');

            expect($errors['team_id'][0])->toBe('The team_id must be an integer or a float.');
            expect($errors['role'][0])->toBe('The selected role is invalid.');

            expect($e->getMessage())->toBe('Invalid data');
        }
    });

    it('handles many validation errors efficiently', function () {
        $model = new class extends BaseModel
        {
            public int $field1;

            public int $field2;

            public int $field3;

            public int $field4;

            public int $field5;

            public int $field6;

            public int $field7;

            public int $field8;

            public int $field9;

            public int $field10;
        };

        $start = microtime(true);

        try {
            validate([
                'field1' => 'invalid',
                'field2' => 'invalid',
                'field3' => 'invalid',
                'field4' => 'invalid',
                'field5' => 'invalid',
                'field6' => 'invalid',
                'field7' => 'invalid',
                'field8' => 'invalid',
                'field9' => 'invalid',
                'field10' => 'invalid',
            ], $model);
        } catch (ValidationException $e) {
            $end = microtime(true);

            expect($end - $start)->toBeLessThan(0.1);

            expect(count($e->getErrors()))->toBe(10);
        }
    });
});
