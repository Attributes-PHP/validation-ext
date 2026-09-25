<?php

declare(strict_types=1);

require_once __DIR__.'/../helpers.php';

// Union types - strict mode - invalid values
$invalidStringIntStrict = [
    12.5,
    -10.5,
    true,
    false,
    [[123]],
    (object) [123],
    new DateTime,
    new class {},
    null,
];

dataset('invalid string|int strict', $invalidStringIntStrict);

dataset('invalid string|int loose', $invalidStringIntStrict);

$invalidIntFloatStrict = [
    '123',
    'hello world',
    [[5, 2, 1]],
    new class {},
    (object) [[1, 2, 3]],
    new DateTime,
    true,
    false,
    null,
    '2e3',
    '3.14',
];

dataset('invalid int|float strict', $invalidIntFloatStrict);

dataset('invalid int|float loose', [
    ...$invalidIntFloatStrict,
    'not a number',
]);

$invalidArrayObjectStrict = [
    'simple string',
    123,
    123.5,
    true,
    false,
    null,
    '2025-03-06',
];

dataset('invalid array|object strict', $invalidArrayObjectStrict);

dataset('invalid array|object loose', $invalidArrayObjectStrict);

$invalidStringArrayStrict = [
    123,
    123.5,
    -12,
    new class {},
    new DateTime,
    null,
    true,
    false,
];

dataset('invalid string|array strict', $invalidStringArrayStrict);

dataset('invalid string|array loose', $invalidStringArrayStrict);

$invalidBoolNullStrict = [
    0,
    1,
    2,
    -1,
    12.5,
    '0',
    '1',
    'yes',
    'no',
    [],
    [1, 2, 3],
    (object) [1, 2],
    new class {},
    new DateTime,
];

dataset('invalid bool|null strict', $invalidBoolNullStrict);

dataset('invalid bool|null loose', [
    ...$invalidBoolNullStrict,
    'true',
    'false',
]);
