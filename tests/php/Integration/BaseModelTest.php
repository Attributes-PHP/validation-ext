<?php

namespace Attributes\Validation\Tests\Integration;

use Attributes\Validation\BaseModel;

it('BaseModel has all required methods', function () {
    $model = new class extends BaseModel {};

    expect(method_exists($model, 'beforeValidation'))->toBeTrue();
    expect(method_exists($model, 'afterValidation'))->toBeTrue();
});
