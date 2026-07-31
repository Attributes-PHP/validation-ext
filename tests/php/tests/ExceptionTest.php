<?php

namespace Attributes\Validation\Tests;

use Attributes\Validation\Exceptions\BaseException;
use Attributes\Validation\Exceptions\ValidationException;

it('BaseException exists and can be thrown', function () {
    $exception = null;

    try {
        throw new BaseException('Testing');
    } catch (BaseException $e) {
        $exception = $e;
    }

    expect($exception)->not->toBeNull();
    expect($exception->getMessage())->toBe('Testing');
});

it('ValidationException exists, can be thrown and has all necessary methods and properties', function () {
    $exception = null;

    try {
        throw new ValidationException(['number' => 'Invalid number']);
    } catch (BaseException $e) {
        $exception = $e;
    }

    expect($exception)->not->toBeNull();
    expect($exception)->toBeInstanceOf(ValidationException::class);
    expect($exception->getErrors())->toBe(['number' => 'Invalid number']);
});
