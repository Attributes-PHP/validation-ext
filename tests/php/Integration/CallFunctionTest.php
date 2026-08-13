<?php

namespace Attributes\Validation\Tests\Integration;

it('has call function in Attributes\Validation namespace', function () {
    expect(function_exists('Attributes\Validation\call'))->toBeTrue();
});
