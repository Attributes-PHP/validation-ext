<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Benchmark\Models;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Fields\ErrorMessage;

class BenchErrorForm extends BaseModel
{
    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public string $username;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public int $age;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public float $score;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public bool $active;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public int|string $note;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public \DateTime $when;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public string|null $email;

    #[ErrorMessage(type: 'Wrong {field}: expected {expected}, got {value}')]
    public array $tags;
}
