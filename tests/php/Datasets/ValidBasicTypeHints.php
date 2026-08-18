<?php

dataset('valid string loose', [
    'Hello world!',
    '/[a-z]+/g',
    'test@test.com',
    'https://www.example.com/',
    123,
    123.5,
    -12,
    -0.05,
    true,
    false,
    '',
]);

dataset('valid integer loose', [
    '123',
    123,
    '2e3',
    2000,
    -12,
    0,
    '0',
    '000',
    true,
    false,
]);

dataset('valid float loose', [
    '123',
    123,
    123.5,
    -12,
    -0.05,
    -10e5,
    '2e3',
    '-4e4',
    '000',
    true,
    false,
    0,
    1,
]);

dataset('valid bool loose', [
    'true',
    'false',
    'True',
    'False',
    'TrUe',
    'FaLsE',
    'TRUE',
    'FALSE',
    true,
    false,
    0,
    1,
]);

dataset('valid datetime loose', [
    '2025-03-06T08:57:06+00:00',
    '2050-12-06T00:00:03+00:00',
    '2013-06-23',
    new DateTime,
    new DateTime('2013-6-23'),
    new DateTime('2025-03-06T08:57:06+00:00'),
]);

dataset('valid datetimeinterface loose', [
    '2025-03-06T08:57:06+00:00',
    '2050-12-06T00:00:03+00:00',
    '2013-06-23',
    new DateTime,
    new DateTime('2013-6-23'),
    new DateTime('2025-03-06T08:57:06+00:00'),
]);
