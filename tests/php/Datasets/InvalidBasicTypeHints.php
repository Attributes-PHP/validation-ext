<?php

dataset('invalid string loose', [
    [[123]],
    [['a' => 1, 'b' => 2]],
    (object) [123],
    (object) ['a' => 1, 'b' => 2],
    new class {},
    new DateTime,
    null,
]);

dataset('invalid integer loose', [
    'hello world',
    [[5, 2, 1]],
    new class {},
    (object) [[1, 2, 3], [1, 2, 3]],
    new DateTime,
    null,
    '3245e9898989898989898989',
]);

dataset('invalid float loose', [
    'hello world',
    [[5, 2, 1]],
    new class {},
    (object) [[1, 2, 3], [1, 2, 3]],
    new DateTime,
    true,
    false,
    null,
]);

dataset('invalid bool loose', [
    'hello',
    'yes',
    'no',
    12345,
    -982,
    -10e10,
    10e10,
    [[1, 2, 3]],
    new class {},
    (object) [[1, 2, 3], [1, 2, 3]],
    new DateTime,
    '6/23/2013',
    null,
]);

dataset('invalid datetime loose', [
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
    '2050-12-06T00:00:99+00:00',
    [[123]],
    (object) [[2345]],
    123,
    -124,
    18.2,
    true,
    false,
    null,
]);
