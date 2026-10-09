<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark;

use Attributes\Validation\Tests\Benchmark\Models\BenchAttributeUser;
use Attributes\Validation\Tests\Benchmark\Models\BenchEmptyUser;
use Attributes\Validation\Tests\Benchmark\Models\BenchLooseUser;
use Attributes\Validation\Tests\Benchmark\Models\BenchOrder;
use Attributes\Validation\Tests\Benchmark\Models\BenchSimpleUser;
use PhpBench\Attributes as Bench;

use function Attributes\Validation\validate;

#[Bench\OutputTimeUnit('microseconds')]
class SingleModelValidationBench
{
    private array $simpleData;

    private array $looseData;

    private array $attributeData;

    private array $nestedData;

    private BenchSimpleUser $simpleUser;

    private BenchEmptyUser $emptyUser;

    private BenchLooseUser $looseUser;

    private BenchAttributeUser $attributeUser;

    private BenchOrder $order;

    public function __construct()
    {
        $this->simpleData = [
            'age' => 30,
            'name' => 'Andre',
            'email' => 'andre@example.com',
            'active' => true,
            'balance' => 1250.50,
        ];

        $this->looseData = [
            'age' => '30',
            'name' => 123,
            'email' => 'andre@example.com',
            'active' => 1,
            'balance' => '1250.50',
        ];

        $this->attributeData = [
            'user_name' => 'Andre',
            'age' => 30,
            'tags' => ['php', 'extension', 'validation'],
        ];

        $this->nestedData = [
            'id' => 42,
            'addresses' => [
                ['street' => 'Main St', 'city' => 'Porto'],
                ['street' => 'Second St', 'city' => 'Lisbon'],
                ['street' => 'Third St', 'city' => 'Braga'],
                ['street' => 'Fourth St', 'city' => 'Faro'],
                ['street' => 'Fifth St', 'city' => 'Aveiro'],
                ['street' => 'Sixth St', 'city' => 'Coimbra'],
                ['street' => 'Seventh St', 'city' => 'Setubal'],
                ['street' => 'Eighth St', 'city' => 'Guimaraes'],
                ['street' => 'Ninth St', 'city' => 'Viseu'],
                ['street' => 'Tenth St', 'city' => 'Evora'],
            ],
        ];

        $this->simpleUser = new BenchSimpleUser;
        $this->emptyUser = new BenchEmptyUser;
        $this->looseUser = new BenchLooseUser;
        $this->attributeUser = new BenchAttributeUser;
        $this->order = new BenchOrder;
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateEmptyModel(): void
    {
        validate([], $this->emptyUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateSimpleModel(): void
    {
        validate($this->simpleData, $this->simpleUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateLooseModel(): void
    {
        validate($this->looseData, $this->looseUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(200), Bench\Iterations(5)]
    public function benchValidateAttributeModel(): void
    {
        validate($this->attributeData, $this->attributeUser);
    }

    #[Bench\Subject, Bench\Groups(['single-model'])]
    #[Bench\Revs(50), Bench\Iterations(5)]
    public function benchValidateNestedModel(): void
    {
        validate($this->nestedData, $this->order);
    }
}
