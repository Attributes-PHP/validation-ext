<?php

declare(strict_types=1);

function all_case_combinations(string $str): array
{
    $result = [];
    $n = strlen($str);
    $total = 1 << $n;

    for ($i = 0; $i < $total; $i++) {
        $combination = '';
        for ($j = 0; $j < $n; $j++) {
            $combination .= $i & (1 << $j) ? strtoupper($str[$j]) : strtolower($str[$j]);
        }
        $result[] = $combination;
    }

    return $result;
}
