<?php

declare(strict_types=1);

require_once __DIR__.'/../helpers.php';

// Union types - strict mode
dataset('valid string|int strict', [
    'Hello world!',
    123,
    '456',
    0,
    -42,
    '',
]);

// Union types - loose mode
dataset('valid string|int loose', [
    'Hello world!',
    123,
    '456',
    0,
    -42,
    '',
    true,
    false,
]);

dataset('valid int|float strict', [
    123,
    123.5,
    -12,
    -0.05,
    0,
    1,
    PHP_INT_MAX,
    PHP_INT_MIN,
]);

dataset('valid int|float loose', [
    123,
    123.5,
    '456',
    '789.5',
    -12,
    -0.05,
    0,
    1,
    true,
    false,
]);

dataset('valid array|object strict', [
    [1, 2, 3],
    ['a' => 1, 'b' => 2],
    (object) [1, 2, 3],
    (object) ['a' => 1, 'b' => 2],
    [],
    new stdClass,
    new DateTime,
]);

dataset('valid array|object loose', [
    [1, 2, 3],
    ['a' => 1, 'b' => 2],
    (object) [1, 2, 3],
    (object) ['a' => 1, 'b' => 2],
    [],
    new stdClass,
    new DateTime,
    '123',
    123,
    true,
    false,
]);

dataset('valid string|array strict', [
    'hello',
    [1, 2, 3],
    ['key' => 'value'],
    '',
    [],
]);

dataset('valid string|array loose', [
    'hello',
    [1, 2, 3],
    ['key' => 'value'],
    '',
    [],
    123,
    true,
]);

dataset('valid bool|null strict', [
    true,
    false,
    null,
]);

dataset('valid bool|null loose', [
    true,
    false,
    null,
    0,
    1,
    '',
    '0',
    '1',
]);
