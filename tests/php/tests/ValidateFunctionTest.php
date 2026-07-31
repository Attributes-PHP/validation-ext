<?php

namespace Attributes\Validation\Tests;

it('has validate function in Attributes\Validation namespace', function () {
    expect(function_exists('Attributes\Validation\validate'))->toBeTrue();
});
