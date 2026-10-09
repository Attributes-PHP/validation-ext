<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark;

use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Tests\Benchmark\Models\BenchDateTimeEvent;
use Attributes\Validation\Tests\Benchmark\Models\BenchErrorForm;
use Attributes\Validation\Tests\Benchmark\Models\BenchNestedShape;
use Attributes\Validation\Tests\Benchmark\Models\BenchOrder;
use Attributes\Validation\Tests\Benchmark\Models\BenchShallowModel;
use Attributes\Validation\Tests\Benchmark\Models\BenchWideModel;
use PhpBench\Attributes as Bench;

use function Attributes\Validation\validate;

/**
 * Data-layout scenarios for the review findings: each subject isolates
 * one hot path so a candidate optimization (#5 hot/cold split, #6 spec
 * flattening, per-element path strings, datetime call setup, error
 * template compilation) can be judged against a baseline number.
 *
 * Baselines to record before changing anything: run
 * `vendor/bin/phpbench run tests/Benchmark/DataLayoutBench.php --group=<group>`.
 */
#[Bench\OutputTimeUnit('microseconds')]
class DataLayoutBench
{
    private array $wideData;

    private array $matrixData;

    private array $groupsData;

    private array $ordersData;

    private array $dateTimeData;

    private array $errorData;

    private array $extraKeysData;

    public function __construct()
    {
        $this->wideData = [];
        for ($i = 1; $i <= 32; $i++) {
            $name = 'field' . str_pad((string) $i, 2, '0', STR_PAD_LEFT);
            $this->wideData[$name] = match (($i - 1) % 8) {
                0, 1 => 'value-' . $i,
                2, 3 => $i,
                4, 5 => $i + 0.5,
                6, 7 => $i % 2 === 1,
            };
        }

        $this->matrixData = [
            'matrix' => [],
            'groups' => ['seed' => [1]],
        ];
        for ($i = 0; $i < 20; $i++) {
            $this->matrixData['matrix'][] = range($i * 50, $i * 50 + 49);
        }

        $this->groupsData = [
            'matrix' => [[1]],
            'groups' => [],
        ];
        for ($i = 0; $i < 20; $i++) {
            $this->groupsData['groups']['group-' . $i] = range(0, 49);
        }

        $this->ordersData = ['id' => 1, 'addresses' => []];
        for ($i = 0; $i < 20; $i++) {
            $this->ordersData['addresses'][] = [
                'street' => 'Street ' . $i,
                'city' => 'City ' . $i,
            ];
        }

        $stamp = '2026-10-09T12:30:00+00:00';
        $this->dateTimeData = [
            'createdAt' => $stamp,
            'updatedAt' => $stamp,
            'deletedAt' => $stamp,
            'archivedAt' => $stamp,
        ];

        $this->errorData = [
            'username' => 42,
            'age' => 'not an int',
            'score' => true,
            'active' => 3.14,
            'note' => false,
            'when' => 'not a datetime',
            'email' => 99,
            'tags' => 'oops',
        ];

        $this->extraKeysData = [
            'name' => 'andre',
            'count' => 7,
            'ratio' => 1.5,
        ];
        for ($i = 0; $i < 97; $i++) {
            $this->extraKeysData['unknown' . $i] = $i;
        }

        // Warm the compiled plans, field-name and spec caches: the
        // scenarios measure steady-state per-call cost, not the
        // first-call compilation
        $this->benchValidate32FieldModel();
        $this->benchValidateNestedSequences();
        $this->benchValidateDictOfSequences();
        $this->benchValidateSequenceOfNestedModels();
        $this->benchValidateFourDatetimeCoercions();
        $this->benchValidateEightErrorsPerCall();
        $this->benchValidateThreeFieldModelWithHundredKeys();
    }

    #[Bench\Subject, Bench\Groups(['wide-model'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidate32FieldModel(): void
    {
        validate($this->wideData, new BenchWideModel);
    }

    #[Bench\Subject, Bench\Groups(['nested-shape'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateNestedSequences(): void
    {
        validate($this->matrixData, new BenchNestedShape);
    }

    #[Bench\Subject, Bench\Groups(['nested-shape'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateDictOfSequences(): void
    {
        validate($this->groupsData, new BenchNestedShape);
    }

    #[Bench\Subject, Bench\Groups(['nested-models'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateSequenceOfNestedModels(): void
    {
        validate($this->ordersData, new BenchOrder);
    }

    #[Bench\Subject, Bench\Groups(['datetime-coercion'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateFourDatetimeCoercions(): void
    {
        validate($this->dateTimeData, new BenchDateTimeEvent);
    }

    #[Bench\Subject, Bench\Groups(['error-heavy'])]
    #[Bench\Revs(20), Bench\Iterations(5)]
    public function benchValidateEightErrorsPerCall(): void
    {
        try {
            validate($this->errorData, new BenchErrorForm);
        } catch (ValidationException) {
            // The expected outcome: eight errors collected, templates
            // and {expected} strings built per field per call
        }
    }

    #[Bench\Subject, Bench\Groups(['extra-keys'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateThreeFieldModelWithHundredKeys(): void
    {
        validate($this->extraKeysData, new BenchShallowModel);
    }
}
