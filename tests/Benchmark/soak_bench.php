<?php

declare(strict_types=1);

/**
 * Soak benchmarks: millions of iterations per scenario, asserting memory
 * stability (refcount leaks) and printing ns/op (at this scale the
 * resolution is ~1ns, enough to judge minor optimisations).
 *
 * Run: php tests/Benchmark/soak_bench.php [substring filter]
 * Exit code 1 when any case drifts past its memory threshold.
 */

require __DIR__.'/../../vendor/autoload.php';

use Attributes\Validation\Tests\Benchmark\Models\BenchBrokenDict;
use Attributes\Validation\Tests\Benchmark\Models\BenchDateTimeEvent;
use Attributes\Validation\Tests\Benchmark\Models\BenchErrorForm;
use Attributes\Validation\Tests\Benchmark\Models\BenchLooseUser;
use Attributes\Validation\Tests\Benchmark\Models\BenchNestedShape;
use Attributes\Validation\Tests\Benchmark\Models\BenchOrder;
use Attributes\Validation\Tests\Benchmark\Models\BenchShallowModel;
use Attributes\Validation\Tests\Benchmark\Models\BenchSignup;
use Attributes\Validation\Tests\Benchmark\Models\BenchTagList;
use Attributes\Validation\Tests\Benchmark\Models\BenchWideModel;

use function Attributes\Validation\validate;

// Memory drift tolerated per case: real leaks show up as megabytes at
// these iteration counts, while engine-internal caches (last exception,
// interned strings) drift a few kilobytes at most.
const DRIFT_LIMIT = 262144;

$failures = 0;
$filter = $argv[1] ?? '';

function run_case(string $name, int $iterations, Closure $body): void
{
    global $failures;

    $body(); // warm the compiled plan, field name and spec caches
    gc_collect_cycles();

    $memory = memory_get_usage();
    $start = hrtime(true);

    for ($i = 0; $i < $iterations; $i++) {
        $body();
    }

    $ns_per_op = (hrtime(true) - $start) / $iterations;
    gc_collect_cycles();
    $drift = memory_get_usage() - $memory;

    $verdict = $drift <= DRIFT_LIMIT ? 'PASS' : 'LEAK';
    printf("%-46s %9d iters  %9.1f ns/op  drift %8d B  %s\n", $name, $iterations, $ns_per_op, $drift, $verdict);

    if ($drift > DRIFT_LIMIT) {
        $failures++;
    }
}

$wideData = [];
for ($i = 1; $i <= 32; $i++) {
    $name = 'field'.str_pad((string) $i, 2, '0', STR_PAD_LEFT);
    $wideData[$name] = match (($i - 1) % 8) {
        0, 1 => 'value-'.$i,
        2, 3 => $i,
        4, 5 => $i + 0.5,
        6, 7 => ($i % 2) === 1,
    };
}

$extraKeysData = ['name' => 'andre', 'count' => 7, 'ratio' => 1.5];
for ($i = 0; $i < 97; $i++) {
    $extraKeysData['unknown'.$i] = $i;
}

$matrixData = [
    'matrix' => [],
    'groups' => ['seed' => [1]],
];
for ($i = 0; $i < 20; $i++) {
    $matrixData['matrix'][] = range($i * 50, ($i * 50) + 49);
}

$groupsData = [
    'matrix' => [[1]],
    'groups' => [],
];
for ($i = 0; $i < 20; $i++) {
    $groupsData['groups']['group-'.$i] = range(0, 49);
}

$tagListData = ['name' => 'validation', 'tags' => array_map(fn ($i) => "tag-$i", range(0, 999))];

$stamp = '2026-10-09T12:30:00+00:00';
$dateTimeOk = array_fill_keys(['createdAt', 'updatedAt', 'deletedAt', 'archivedAt'], $stamp);

$errorData = [
    'username' => 42,
    'age' => 'not an int',
    'score' => true,
    'active' => 3.14,
    'note' => false,
    'when' => 'not a datetime',
    'email' => 99,
    'tags' => 'oops',
];

$signupData = ['user_name' => 'andre', 'email' => 'andre@example.com', 'age' => '30'];

$cases = [
    'wide model, 32 typed fields' => [
        1_000_000,
        function () use ($wideData) {
            validate($wideData, new BenchWideModel);
        },
    ],

    'extra keys, 100 keys into 3 fields' => [
        1_000_000,
        function () use ($extraKeysData) {
            validate($extraKeysData, new BenchShallowModel);
        },
    ],

    'alias attribute + snake generator lookup' => [
        1_000_000,
        function () use ($signupData) {
            validate($signupData, new BenchSignup);
        },
    ],

    'datetime coercion, 4 per call' => [
        1_000_000,
        function () use ($stamp) {
            // Rebuilt each iteration: a successful coercion replaces the
            // string with a DateTime object, and a reused bucket would skip
            // coercion on the next call
            validate(array_fill_keys([
                'createdAt',
                'updatedAt',
                'deletedAt',
                'archivedAt',
            ], $stamp), new BenchDateTimeEvent);
        },
    ],

    'datetime coercion failure, dynamic strings' => [
        1_000_000,
        function () {
            // Dynamic (non-interned) strings: interned literals would mask
            // a refcount leak on the failure path
            $bad = substr('x2026-10-09T12:30:00+INVALID', 1);
            try {
                validate(array_fill_keys([
                    'createdAt',
                    'updatedAt',
                    'deletedAt',
                    'archivedAt',
                ], $bad), new BenchDateTimeEvent);
            } catch (Throwable) {
                // Expected: four type errors
            }
        },
    ],

    'error collection, 8 errors + exception' => [
        1_000_000,
        function () use ($errorData) {
            try {
                validate($errorData, new BenchErrorForm);
            } catch (Throwable) {
                // Expected: ValidationException
            }
        },
    ],

    'nested sequences, 20 x 50 ints' => [
        1_000_000,
        function () use ($matrixData) {
            validate($matrixData, new BenchNestedShape);
        },
    ],

    'dict of sequences, 20 x 50 ints' => [
        1_000_000,
        function () use ($groupsData) {
            validate($groupsData, new BenchNestedShape);
        },
    ],

    'scalar sequence, 1000 elements' => [
        1_000_000,
        function () use ($tagListData) {
            validate($tagListData, new BenchTagList);
        },
    ],

    'nested model hydration, 20 per call' => [
        1_000_000,
        function () {
            // Rebuilt each iteration: hydration replaces the address buckets
            // with model objects, and a reused array would skip hydration
            $addresses = [];
            for ($i = 0; $i < 20; $i++) {
                $addresses[] = ['street' => 'Street '.$i, 'city' => 'City '.$i];
            }
            validate(['id' => 1, 'addresses' => $addresses], new BenchOrder);
        },
    ],

    'loose coercion, int/float/bool/string' => [
        1_000_000,
        function () {
            // Rebuilt each iteration: loose-mode coercion rewrites the
            // bucket values in place
            validate([
                'age' => '30',
                'name' => 'andre',
                'email' => 'andre@example.com',
                'active' => 'true',
                'balance' => '1.5',
            ], new BenchLooseUser);
        },
    ],

    'failed spec build, uncached ValueError' => [
        1_000_000,
        function () {
            // The Dict attribute is missing `of`: the spec build fails and
            // stays uncached on purpose, exercising the free path
            try {
                validate(['items' => [1 => 2]], new BenchBrokenDict);
            } catch (Throwable) {
                // Expected: ValueError, must keep firing every call
            }
        },
    ],
];

foreach ($cases as $name => [$iterations, $body]) {
    if ($filter === '' || str_contains($name, $filter)) {
        run_case($name, $iterations, $body);
    }
}

exit($failures === 0 ? 0 : 1);
