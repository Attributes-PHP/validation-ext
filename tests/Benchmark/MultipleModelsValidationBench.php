<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark;

use Attributes\Validation\Tests\Benchmark\Models\BenchCartItem;
use Attributes\Validation\Tests\Benchmark\Models\BenchCategory;
use Attributes\Validation\Tests\Benchmark\Models\BenchInvoice;
use Attributes\Validation\Tests\Benchmark\Models\BenchProduct;
use Attributes\Validation\Tests\Benchmark\Models\BenchProfile;
use Attributes\Validation\Tests\Benchmark\Models\BenchReview;
use Attributes\Validation\Tests\Benchmark\Models\BenchSession;
use Attributes\Validation\Tests\Benchmark\Models\BenchSignup;
use Attributes\Validation\Tests\Benchmark\Models\BenchTagList;
use PhpBench\Attributes as Bench;

use function Attributes\Validation\validate;

#[Bench\OutputTimeUnit('microseconds')]
class MultipleModelsValidationBench
{
    private array $data;

    private array $models;

    public function __construct()
    {
        $this->data = [
            BenchProfile::class => [
                'username' => 'andre',
                'email' => 'andre@example.com',
                'loginCount' => 42,
            ],
            BenchSignup::class => [
                'user_name' => 'andre',
                'email' => 'andre@example.com',
                'age' => '30',
            ],
            BenchProduct::class => [
                'sku' => 'SKU-1',
                'title' => 'Validation extension',
                'price' => 99.9,
                'stock' => 10,
            ],
            BenchSession::class => [
                'sessionId' => 'abc123',
                'userId' => 'user-1',
                'rememberMe' => true,
            ],
            BenchCartItem::class => [
                'sku' => 'SKU-1',
                'quantity' => 2,
                'unitPrice' => 49.95,
            ],
            BenchInvoice::class => [
                'number' => 'INV-2026-001',
                'customer' => 'Acme Inc',
                'total' => 1250.50,
            ],
            BenchReview::class => [
                'author' => 'andre',
                'comment' => 'Fast validation',
                'rating' => 5,
            ],
            BenchCategory::class => [
                'slug' => 'php-extensions',
                'name' => 'PHP Extensions',
            ],
            BenchTagList::class => [
                'name' => 'validation',
                'tags' => ['php', 'c', 'extension'],
            ],
        ];

        $this->models = [];
        foreach (array_keys($this->data) as $class) {
            $this->models[] = new $class;
        }
    }

    #[Bench\Subject, Bench\Groups(['multiple-models'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNineDistinctModels(): void
    {
        foreach ($this->models as $model) {
            validate($this->data[$model::class], $model);
        }
    }

    #[Bench\Subject, Bench\Groups(['multiple-models'])]
    #[Bench\Revs(100), Bench\Iterations(5)]
    public function benchValidateSameModelTenTimes(): void
    {
        $model = $this->models[0];
        $data = $this->data[$model::class];

        for ($i = 0; $i < 10; $i++) {
            validate($data, $model);
        }
    }
}
