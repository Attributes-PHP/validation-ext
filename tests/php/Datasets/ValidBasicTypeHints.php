<?php

require_once __DIR__.'/../helpers.php';

$validStringStrict = [
    'Hello world!',
    '/[a-z]+/g',
    'test@test.com',
    'https://www.example.com/',
    '',
];

dataset('valid string strict', $validStringStrict);

dataset('valid string loose', [
    ...$validStringStrict,
    123,
    123.5,
    -12,
    -0.05,
    true,
    false,
]);

$validIntegerStrict = [
    123,
    -12,
    2000,
    0,
    PHP_INT_MAX,
    PHP_INT_MIN,
];

dataset('valid integer strict', $validIntegerStrict);

dataset('valid integer loose', [
    ...$validIntegerStrict,
    '123',
    '2e3',
    '0',
    '000',
    true,
    false,
]);

$validFloatStrict = [
    123.5,
    -12.82,
    -0.05,
    -10e5,
    2e3,
    PHP_INT_MAX,
    PHP_INT_MIN,
    3245e9898989898989898989, // INF is valid by default
];

dataset('valid float strict', $validFloatStrict);

dataset('valid float loose', [
    ...$validFloatStrict,
    '123',
    123,
    '-4e4',
    '000',
    true,
    false,
    0,
    1,
]);

$validBoolStrict = [
    true,
    false,
];

dataset('valid bool strict', $validBoolStrict);

dataset('valid bool loose', [
    ...$validBoolStrict,
    ...allCaseCombinations('yes'),
    ...allCaseCombinations('no'),
    ...allCaseCombinations('y'),
    ...allCaseCombinations('n'),
    ...allCaseCombinations('t'),
    ...allCaseCombinations('f'),
    ...allCaseCombinations('true'),
    ...allCaseCombinations('false'),
    0,
    1,
]);

$validDatetimeStrict = [
    '2025-03-06T08:57:06+00:00',
    '2050-12-06T00:00:03+00:00',
    '2013-06-23',
    new DateTime,
    new DateTime('2013-6-23'),
    new DateTime('2025-03-06T08:57:06+00:00'),
];

dataset('valid datetime strict', $validDatetimeStrict);

dataset('valid datetime loose', $validDatetimeStrict);
