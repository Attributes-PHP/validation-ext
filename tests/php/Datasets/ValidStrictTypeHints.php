<?php

dataset('valid string strict', [
    'Hello world!',
    '/[a-z]+/g',
    'test@test.com',
    'https://www.example.com/',
    '',
]);

dataset('valid integer strict', [
    123,
    -12,
    2000,
    0,
    PHP_INT_MAX,
    PHP_INT_MIN,
]);

dataset('valid float strict', [
    123.5,
    -12.82,
    -0.05,
    -10e5,
    2e3,
]);

dataset('valid bool strict', [
    true,
    false,
]);

dataset('valid datetime strict', [
    '2025-03-06T08:57:06+00:00',
    '2050-12-06T00:00:03+00:00',
    '2013-06-23',
    new DateTime,
    new DateTime('2013-6-23'),
    new DateTime('2025-03-06T08:57:06+00:00'),
]);
