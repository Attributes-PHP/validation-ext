<?php

declare(strict_types=1);

require_once __DIR__.'/../helpers.php';

$invalidStringLoose = [
    [[123]],
    [['a' => 1, 'b' => 2]],
    (object) [123],
    (object) ['a' => 1, 'b' => 2],
    new class {},
    new DateTime,
    null,
];

dataset('invalid string strict', [
    1,
    2.94,
    -30,
    -3.3,
    false,
    true,
    [[1, 2, 3]],
    (object) [[3, 3]],
    ...$invalidStringLoose,
]);

dataset('invalid string loose', $invalidStringLoose);

$invalidIntegerLoose = [
    'hello world',
    [[5, 2, 1]],
    new class {},
    (object) [[1, 2, 3], [1, 2, 3]],
    new DateTime,
    null,
    '3245e9898989898989898989',
];

dataset('invalid integer strict', [
    '123',
    ...$invalidIntegerLoose,
    true,
    false,
    '2e3',
    '-4e4',
    '000',
    3245e9898989898989898989,
]);

dataset('invalid integer loose', $invalidIntegerLoose);

$invalidFloatLoose = [
    'hello world',
    [[5, 2, 1]],
    new class {},
    (object) [[1, 2, 3], [1, 2, 3]],
    new DateTime,
    null,
];

dataset('invalid float strict', [
    '123',
    '24.98',
    ...$invalidFloatLoose,
    true,
    false,
    '2e3',
    '-4e4',
    '000',
    '3245e9898989898989898989',
]);

dataset('invalid float loose', $invalidFloatLoose);

$invalidBoolLoose = [
    ...all_case_combinations('noo'),
    ...all_case_combinations('yess'),
    ...all_case_combinations('ye'),
    ...all_case_combinations('truee'),
    ...all_case_combinations('falsee'),
    'fals$',
    'hello',
    12_345,
    -982,
    -10e10,
    10e10,
    [[1, 2, 3]],
    new class {},
    (object) [[1, 2, 3], [1, 2, 3]],
    new DateTime,
    '6/23/2013',
    null,
];

dataset('invalid bool strict', [
    ...all_case_combinations('no'),
    ...all_case_combinations('yes'),
    ...all_case_combinations('y'),
    ...all_case_combinations('n'),
    ...all_case_combinations('t'),
    ...all_case_combinations('f'),
    ...all_case_combinations('true'),
    ...all_case_combinations('false'),
    0,
    1,
    ...$invalidBoolLoose,
]);

dataset('invalid bool loose', $invalidBoolLoose);

$invalidDatetimeLoose = [
    '6/23/2013',
    '23.06. 2013',
    '2013/6/23',
    '2013-06-23',
    '20130623T13:22-0500',
    '2011-10-05T14:48:00.000Z',
    '2025-99-99 15:46:55',
    '2025-999-01',
    '2013-6-999',
    '6/23/-2000',
    'Invalid date',
    '2050-12-06T00:00:999+00:00',
    [[123]],
    (object) [[2345]],
    123,
    -124,
    18.2,
    true,
    false,
    null,
];

dataset('invalid datetime strict', $invalidDatetimeLoose);

dataset('invalid datetime loose', $invalidDatetimeLoose);
