<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Validate\TypeHint;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\ModelConfigs;
use Attributes\Validation\Tests\Models\ArrayTypeHintLazyAddress;
use DateTime;
use DateTimeInterface;
use ValueError;

use function Attributes\Validation\validate;

class ArrayTypeHintAddress extends BaseModel
{
    public string $street;
}

class ArrayTypeHintBigAddress extends ArrayTypeHintAddress
{
    public string $zip;
}

class ArrayTypeHintUserWithAddresses extends BaseModel
{
    /** @var array<ArrayTypeHintAddress> */
    public array $addresses;
}

class ArrayTypeHintLazyUser extends BaseModel
{
    /**
     * Fully qualified on purpose: `use` imports are compile-time, file-level
     * aliases that PHP discards after compilation, so they cannot be applied
     * to doc comments at runtime. Cross-namespace docstring references must
     * spell out the whole name.
     *
     * @var array<\Attributes\Validation\Tests\Models\ArrayTypeHintLazyAddress>
     */
    public array $addresses;
}

class ArrayTypeHintSelfNode extends BaseModel
{
    /** @var array<self> */
    public array $children;

    public string $name;
}

class ArrayTypeHintUserWithMissingDocClass extends BaseModel
{
    /** @var array<ArrayTypeHintNowhereAddress> */
    public array $addresses;
}

class ArrayTypeHintUserWithMissingUnionClass extends BaseModel
{
    /** @var array<string|ArrayTypeHintNowhereAddress> */
    public array $tags;
}

class ArrayTypeHintUserWithMissingNestedDocClass extends BaseModel
{
    /** @var array<array<ArrayTypeHintNowhereAddress>> */
    public array $matrix;
}

class ArrayTypeHintUserWithMissingQualifiedDocClass extends BaseModel
{
    /** @var array<\Attributes\Validation\Tests\Integration\Validate\TypeHint\ArrayTypeHintNowhereAddress> */
    public array $items;
}

class ArrayTypeHintUser extends BaseModel
{
    /** @var array<string> */
    public array $tags;

    /** @var array<string|int> */
    public array $mixed;

    /** @var array<int, string> */
    public array $keyed;

    /** @var array<DateTime> */
    public array $birthdays;

    /** @var array<ArrayTypeHintAddress> */
    public array $addresses;

    /** @var array<array<int>> */
    public array $matrix;
}

describe('type-hint array validation via docstrings (loose mode)', function () {
    it('accepts an array of strings', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public array $tags;
        };

        $result = validate(['tags' => ['a', 'b', 'c']], $model);
        expect($result->tags)->toBe(['a', 'b', 'c']);
    });

    it('accepts an empty array for a shaped property', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public array $tags;
        };

        $result = validate(['tags' => []], $model);
        expect($result->tags)->toBeEmpty();
    });

    it('accepts a nullable shaped array when value is null', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public ?array $tags;
        };

        $result = validate(['tags' => null], $model);
        expect($result->tags)->toBeNull();
    });

    it('converts scalar elements to the declared basic type in loose mode', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public array $tags;
        };

        $result = validate(['tags' => ['a', 30, 1.5]], $model);
        expect($result->tags)->toBe(['a', '30', '1.5']);
    });

    it('rejects a non-scalar element for a string spec', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public array $tags;
        };

        try {
            validate(['tags' => ['a', []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'tags.1' => ['Must be string'],
            ]);
        }
    });

    it('accepts an array of strings or integers', function () {
        $model = new class extends BaseModel {
            /** @var array<string|int> */
            public array $mixed;
        };

        $result = validate(['mixed' => ['a', 30, 'b', 7]], $model);
        expect($result->mixed)->toBe(['a', 30, 'b', 7]);
    });

    it('coerces numeric strings to integer for an integer element spec', function () {
        $model = new class extends BaseModel {
            /** @var array<int> */
            public array $scores;
        };

        $result = validate(['scores' => ['30', 50]], $model);
        expect($result->scores)->toBe([30, 50]);
    });

    it('rejects an element matching no union arm', function () {
        $model = new class extends BaseModel {
            /** @var array<string|int> */
            public array $mixed;
        };

        try {
            validate(['mixed' => ['a', []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'mixed.1' => ['Must be string or integer'],
            ]);
        }
    });

    it('enforces the key type of the dict form', function () {
        $model = new class extends BaseModel {
            /** @var array<int, string> */
            public array $keyed;
        };

        try {
            validate(['keyed' => ['name' => 'a']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'keyed.name' => ['Must be integer'],
            ]);
        }
    });

    it('accepts string keys for a string key spec', function () {
        $model = new class extends BaseModel {
            /** @var array<string, int> */
            public array $keyed;
        };

        $result = validate(['keyed' => ['a' => 1, 'b' => 2]], $model);
        expect($result->keyed)->toBe(['a' => 1, 'b' => 2]);
    });

    it('does not check keys for the list form', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public array $tags;
        };

        $result = validate(['tags' => ['any' => 'a']], $model);
        expect($result->tags)->toBe(['any' => 'a']);
    });

    it('accepts the legacy T[] form', function () {
        $model = new class extends BaseModel {
            /** @var string[] */
            public array $tags;
        };

        $result = validate(['tags' => ['a', 'b']], $model);
        expect($result->tags)->toBe(['a', 'b']);
    });

    it('accepts the list<T> form', function () {
        $model = new class extends BaseModel {
            /** @var list<int> */
            public array $scores;
        };

        $result = validate(['scores' => [1, 2, 3]], $model);
        expect($result->scores)->toBe([1, 2, 3]);
    });

    it('coerces string elements to DateTime objects', function () {
        $model = new class extends BaseModel {
            /** @var array<DateTime> */
            public array $birthdays;
        };

        $result = validate(['birthdays' => ['1994-01-01T09:00:00+00:00']], $model);
        expect($result->birthdays)->toHaveCount(1);
        expect($result->birthdays[0])->toBeInstanceOf(DateTimeInterface::class);
    });

    it('hydrates array elements into nested models', function () {
        $model = new ArrayTypeHintUserWithAddresses;

        $result = validate(['addresses' => [['street' => 'Main St']]], $model);
        expect($result->addresses)->toHaveCount(1);
        expect($result->addresses[0])->toBeInstanceOf(ArrayTypeHintAddress::class);
        expect($result->addresses[0]->street)->toBe('Main St');
    });

    it('accepts already-instantiated model elements', function () {
        $address = new ArrayTypeHintAddress;
        $address->street = 'Main St';

        $result = validate(['addresses' => [$address]], new ArrayTypeHintUserWithAddresses);
        expect($result->addresses)->toHaveCount(1);
        expect($result->addresses[0])->toBe($address);
    });

    it('accepts a mix of instances and raw arrays', function () {
        $address = new ArrayTypeHintAddress;
        $address->street = 'First St';

        $result = validate(['addresses' => [$address, ['street' => 'Second St']]], new ArrayTypeHintUserWithAddresses);
        expect($result->addresses)->toHaveCount(2);
        expect($result->addresses[0])->toBe($address);
        expect($result->addresses[1])->toBeInstanceOf(ArrayTypeHintAddress::class);
        expect($result->addresses[1]->street)->toBe('Second St');
    });

    it('accepts subclass instances of the docstring model', function () {
        $address = new ArrayTypeHintBigAddress;
        $address->street = 'Main St';
        $address->zip = '00000';

        $result = validate(['addresses' => [$address]], new ArrayTypeHintUserWithAddresses);
        expect($result->addresses[0])->toBe($address);
    });

    it('resolves docstring model classes through autoloading', function () {
        expect(class_exists(ArrayTypeHintLazyAddress::class, false))->toBeFalse();

        $result = validate(['addresses' => [['street' => 'Main St']]], new ArrayTypeHintLazyUser);
        expect($result->addresses)->toHaveCount(1);
        expect($result->addresses[0])->toBeInstanceOf(ArrayTypeHintLazyAddress::class);
        expect($result->addresses[0]->street)->toBe('Main St');
    });

    it('supports self references in the docstring', function () {
        $result = validate([
            'name' => 'root',
            'children' => [
                ['name' => 'child', 'children' => []],
            ],
        ], new ArrayTypeHintSelfNode);

        expect($result->children)->toHaveCount(1);
        expect($result->children[0])->toBeInstanceOf(ArrayTypeHintSelfNode::class);
        expect($result->children[0]->name)->toBe('child');
    });

    it('reports nested model element errors under the element path', function () {
        $model = new ArrayTypeHintUserWithAddresses;

        try {
            validate(['addresses' => [['street' => 'Main St'], ['street' => []]]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'addresses.1.street' => ['Must be string'],
            ]);
        }
    });

    it('accepts nested array shapes', function () {
        $model = new class extends BaseModel {
            /** @var array<array<int>> */
            public array $matrix;
        };

        $result = validate(['matrix' => [[1, 2], [3, '4']]], $model);
        expect($result->matrix)->toBe([[1, 2], [3, 4]]);
    });

    it('rejects a non-array element of a nested array shape', function () {
        $model = new class extends BaseModel {
            /** @var array<array<int>> */
            public array $matrix;
        };

        try {
            validate(['matrix' => [[1, 2], 'nope']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'matrix.1' => ['Must be array<integer>'],
            ]);
        }
    });

    it('accepts nullable union arms', function () {
        $model = new class extends BaseModel {
            /** @var array<string|null> */
            public array $tags;
        };

        $result = validate(['tags' => ['a', null]], $model);
        expect($result->tags)->toBe(['a', null]);
    });

    it('keeps plain array behavior without a docstring shape', function () {
        $model = new class extends BaseModel {
            public array $tags;
        };

        $result = validate(['tags' => ['a', 30, 1.5, null]], $model);
        expect($result->tags)->toBe(['a', 30, 1.5, null]);
    });

    it('keeps plain array behavior for a shapeless @var array docstring', function () {
        $model = new class extends BaseModel {
            /** @var array */
            public array $tags;
        };

        $result = validate(['tags' => ['a', 30, 1.5, null]], $model);
        expect($result->tags)->toBe(['a', 30, 1.5, null]);
    });

    it('collects errors for multiple invalid elements', function () {
        $model = new class extends BaseModel {
            /** @var array<string> */
            public array $tags;
        };

        try {
            validate(['tags' => [[], 'a', []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'tags.0' => ['Must be string'],
                'tags.2' => ['Must be string'],
            ]);
        }
    });
});

describe('type-hint array validation via docstrings (strict mode)', function () {
    it('rejects a coercible numeric string for an integer element spec', function () {
        $model = new
            #[ModelConfigs(strict: true)]
            class extends BaseModel {
                /** @var array<int> */
                public array $scores;
            };

        try {
            validate(['scores' => ['30']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'scores.0' => ['Must be integer'],
            ]);
        }
    });

    it('rejects a scalar of a different type for a string element spec', function () {
        $model = new
            #[ModelConfigs(strict: true)]
            class extends BaseModel {
                /** @var array<string> */
                public array $tags;
            };

        try {
            validate(['tags' => ['a', 30]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'tags.1' => ['Must be string'],
            ]);
        }
    });

    it('accepts exact types in strict mode', function () {
        $model = new
            #[ModelConfigs(strict: true)]
            class extends BaseModel {
                /** @var array<int> */
                public array $scores;
            };

        $result = validate(['scores' => [30, 50]], $model);
        expect($result->scores)->toBe([30, 50]);
    });
});

describe('type-hint array validation on declared model classes', function () {
    it('validates all shaped properties of a full model', function () {
        $model = new ArrayTypeHintUser;

        $result = validate([
            'tags' => ['a'],
            'mixed' => ['a', 1],
            'keyed' => ['a'],
            'birthdays' => ['1994-01-01T09:00:00+00:00'],
            'addresses' => [['street' => 'Main St']],
            'matrix' => [[1]],
        ], $model);

        expect($result->tags)->toBe(['a']);
        expect($result->mixed)->toBe(['a', 1]);
        expect($result->birthdays[0])->toBeInstanceOf(DateTimeInterface::class);
        expect($result->addresses[0]->street)->toBe('Main St');
        expect($result->matrix)->toBe([[1]]);
    });
});

describe('missing docstring classes', function () {
    it('throws a ValueError when a docstring class cannot be found', function () {
        validate(['addresses' => []], new ArrayTypeHintUserWithMissingDocClass);
    })->throws(ValueError::class);

    it('throws before any element is validated, even for an empty array', function () {
        try {
            validate(['addresses' => []], new ArrayTypeHintUserWithMissingDocClass);
            expect(false)->toBeTrue();
        } catch (ValueError $e) {
            expect($e->getMessage())->toContain('ArrayTypeHintNowhereAddress');
            expect($e->getMessage())->toContain(
                'Attributes\Validation\Tests\Integration\Validate\TypeHint\ArrayTypeHintNowhereAddress',
            );
            expect($e->getMessage())->toContain('addresses');
            expect($e->getMessage())->toContain('ArrayTypeHintUserWithMissingDocClass');
        }
    });

    it('throws even when another union arm would accept the elements', function () {
        validate(['tags' => ['a', 'b']], new ArrayTypeHintUserWithMissingUnionClass);
    })->throws(ValueError::class);

    it('throws for a class referenced by a nested shape', function () {
        validate(['matrix' => []], new ArrayTypeHintUserWithMissingNestedDocClass);
    })->throws(ValueError::class);

    it('throws for a fully qualified name that cannot be found', function () {
        try {
            validate(['items' => []], new ArrayTypeHintUserWithMissingQualifiedDocClass);
            expect(false)->toBeTrue();
        } catch (ValueError $e) {
            expect($e->getMessage())->toContain(
                'Attributes\Validation\Tests\Integration\Validate\TypeHint\ArrayTypeHintNowhereAddress',
            );
        }
    });
});
