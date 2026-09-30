<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Validate\TypeHint;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\Dict;
use Attributes\Validation\Fields\ErrorMessage;
use Attributes\Validation\Fields\Intersection;
use Attributes\Validation\Fields\Sequence;
use Attributes\Validation\Fields\Union;
use Attributes\Validation\ModelConfigs;
use Attributes\Validation\Type;
use DateTime;
use DateTimeInterface;
use ValueError;

use function Attributes\Validation\validate;

use const Attributes\Validation\bool;
use const Attributes\Validation\float;
use const Attributes\Validation\int;
use const Attributes\Validation\object;
use const Attributes\Validation\sequence;
use const Attributes\Validation\string;

class ArrayTypeHintAddress extends BaseModel
{
    public string $street;
}

class ArrayTypeHintSelfNode extends BaseModel
{
    #[Sequence(ArrayTypeHintSelfNode::class)]
    public array $children;

    public string $name;
}

interface ArrayTypeHintCountable
{
    public function count(): int;
}

interface ArrayTypeHintIterable
{
    public function iterate(): string;
}

class ArrayTypeHintIterableCountable implements ArrayTypeHintCountable, ArrayTypeHintIterable
{
    public function count(): int
    {
        return 0;
    }

    public function iterate(): string
    {
        return 'ArrayTypeHintIterableCountable->iterate(...)';
    }
}

describe('type-hint array validation via attributes (loose mode)', function () {
    it('accepts an array of strings', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
                public array $tags;
            };

        $result = validate(['tags' => ['a', 'b', 'c']], $model);
        expect($result->tags)->toBe(['a', 'b', 'c']);
    });

    it('accepts a Type enum case as the element type', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
                public array $tags;
            };

        $result = validate(['tags' => ['a', 'b']], $model);
        expect($result->tags)->toBe(['a', 'b']);
    });

    it('accepts the namespace type constants as element types', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(of: string)]
                public array $tags;

                #[Union(bool, int, sequence)]
                public array $mixed;
            };

        $result = validate(['tags' => ['a', 'b'], 'mixed' => [true, 7, [1]]], $model);
        expect($result->tags)->toBe(['a', 'b']);
        expect($result->mixed)->toBe([true, 7, [1]]);
    });

    it('aligns Type case values with the Zend engine type masks', function () {
        // The backing values are the engine MAY_BE_* masks, so C-level
        // comparisons need no translation table
        expect([
            Type::bool->value,
            Type::int->value,
            Type::float->value,
            Type::string->value,
            Type::sequence->value,
            Type::object->value,
        ])->toBe([12, 16, 32, 64, 128, 256]);

        // The engine has no dict mask: dict carries a private bit the
        // validator folds into the array type
        expect(Type::dict->value)->toBe(268435456);
    });

    it('accepts a bitmask union of types', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool | int | string)]
                public array $values;
            };

        $result = validate(['values' => [true, 30, 'a']], $model);
        expect($result->values)->toBe([true, 30, 'a']);

        try {
            validate(['values' => [[], (object) []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'values.0' => ['Must be boolean, integer or string'],
                'values.1' => ['Must be boolean, integer or string'],
            ]);
        }
    });

    it('treats a bitmask union the same as an argument list for typed values', function () {
        $list = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, int, string)]
                public array $values;
            };
        $bitmask = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool | int | string)]
                public array $values;
            };

        $rawData = ['values' => [true, 30, 'a']];
        expect(validate($rawData, $list)->values)->toBe(validate($rawData, $bitmask)->values);
    });

    it('loses the declared coercion order when using a bitmask', function () {
        $list = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(int, bool)] // int arm first: '1' coerces to int
                public array $values;
            };
        $bitmask = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(int | bool)] // fixed order: bool first
                public array $values;
            };

        expect(validate(['values' => ['1']], $list)->values)->toBe([1]);
        expect(validate(['values' => ['1']], $bitmask)->values)->toBe([true]);
    });

    it('accepts a bitmask as the Sequence element type', function () {
        $bitmask = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(of: string | int | float)]
                public array $values;
            };
        $union = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(of: new Union(string, int, float))]
                public array $values;
            };

        $rawData = ['values' => ['a', 30, 1.5]];
        expect(validate($rawData, $bitmask)->values)->toBe(['a', 30, 1.5]);
        expect(validate($rawData, $union)->values)->toBe(['a', 30, 1.5]);

        try {
            validate(['values' => [[1 => []]]], $bitmask);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            // The bitmask renders its types in bit order, since the
            // declaration order is lost
            expect($e->getErrors())->toBe([
                'values.0' => ['Must be integer, float or string'],
            ]);
        }
    });

    it('rejects bitmask key types outside integer and string', function () {
        $model = new class extends BaseModel {
            #[Dict(key: bool | int, of: string)]
            public array $keyed;
        };

        validate(['keyed' => []], $model);
    })->throws(ValueError::class);

    it('accepts an empty array for a shaped property', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
                public array $tags;
            };

        $result = validate(['tags' => []], $model);
        expect($result->tags)->toBeEmpty();
    });

    it('accepts a nullable shaped array when value is null', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
                public ?array $tags;
            };

        $result = validate(['tags' => null], $model);
        expect($result->tags)->toBeNull();
    });

    it('converts scalar elements to the declared basic type in loose mode', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
                public array $tags;
            };

        $result = validate(['tags' => ['a', 30, 1.5]], $model);
        expect($result->tags)->toBe(['a', '30', '1.5']);
    });

    it('coerces numeric strings to integer for an integer element type', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::int)]
                public array $scores;
            };

        $result = validate(['scores' => ['30', 50]], $model);
        expect($result->scores)->toBe([30, 50]);
    });

    it('coerces scalar elements to bool for a bool element type', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::bool)]
                public array $flags;
            };

        $result = validate(['flags' => ['true', 0, 1]], $model);
        expect($result->flags)->toBe([true, false, true]);
    });

    it('rejects a non-scalar element for a string spec', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
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
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(string, int)]
                public array $mixed;
            };

        $result = validate(['mixed' => ['a', 30, 'b', 7]], $model);
        expect($result->mixed)->toBe(['a', 30, 'b', 7]);
    });

    it('rejects an element matching no union arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, int, string)]
                public array $mixed;
            };

        try {
            validate(['mixed' => ['a', []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'mixed.1' => ['Must be boolean, integer or string'],
            ]);
        }
    });

    it('prefers an exact match over an earlier coercible arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(int, string)]
                public array $mixed;
            };

        $result = validate(['mixed' => ['30', 50]], $model);
        expect($result->mixed)->toBe(['30', 50]);
    });

    it('follows the union order when coercion is needed', function () {
        $boolFirst = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, int)]
                public array $values;
            };
        $intFirst = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(int, bool)]
                public array $values;
            };

        $result = validate(['values' => ['1', '0']], $boolFirst);
        expect($result->values)->toBe([true, false]);

        $result = validate(['values' => ['1', '0']], $intFirst);
        expect($result->values)->toBe([1, 0]);
    });

    it('enforces the key type of the Dict attribute', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Dict(key: int, of: string)]
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
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Dict(key: string, of: int)]
                public array $keyed;
            };

        $result = validate(['keyed' => ['a' => 1, 'b' => 2]], $model);
        expect($result->keyed)->toBe(['a' => 1, 'b' => 2]);
    });

    it('accepts several key types via a Union key spec', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Dict(key: new Union(int, string), of: string)]
                public array $keyed;
            };

        $result = validate(['keyed' => ['a' => 'b', 3 => 'c']], $model);
        expect($result->keyed)->toBe(['a' => 'b', 3 => 'c']);
    });

    it('rejects dict values not matching the value spec', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Dict(key: string, of: int)]
                public array $keyed;
            };

        try {
            validate(['keyed' => ['a' => 1, 'b' => 'nope']], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'keyed.b' => ['Must be integer'],
            ]);
        }
    });

    it('does not check keys for the list form', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(Type::string)]
                public array $tags;
            };

        $result = validate(['tags' => ['any' => 'a']], $model);
        expect($result->tags)->toBe(['any' => 'a']);
    });

    it('accepts nested arrays via a sequence arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(sequence, string)]
                public array $values;
            };

        $result = validate(['values' => [[1, 2], 'a']], $model);
        expect($result->values)->toBe([[1, 2], 'a']);
    });

    it('validates nested array elements through composed attributes', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, new Sequence(Type::int))]
                public array $matrix;
            };

        $result = validate(['matrix' => [true, ['30', 7]]], $model);
        expect($result->matrix)->toBe([true, [30, 7]]);

        try {
            validate(['matrix' => [true, [1, 'nope']]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'matrix.1.1' => ['Must be integer'],
            ]);
        }
    });

    it('rejects an object element for a scalar union with the union expected string', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, int, string)]
                public array $values;
            };

        try {
            validate(['values' => ['a', [], (object) []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'values.1' => ['Must be boolean, integer or string'],
                'values.2' => ['Must be boolean, integer or string'],
            ]);
        }
    });

    it('accepts objects for an object arm', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(string, object)]
                public array $values;
            };

        $result = validate(['values' => ['a', (object) []]], $model);
        expect($result->values[0])->toBeString();
        expect($result->values[1])->toBeObject();
    });

    it('resolves a Dict attribute arm by class name', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Union(bool, Dict::class)]
                #[Dict(key: int, of: string)]
                public array $values;
            };

        $result = validate(['values' => [true, [1 => 'a', 2 => 'b'], false]], $model);
        expect($result->values)->toBe([true, [1 => 'a', 2 => 'b'], false]);

        try {
            validate(['values' => [[1 => []]]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'values.0.1' => ['Must be string'],
            ]);
        }
    });

    it('hydrates nested BaseModel elements from raw arrays', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(ArrayTypeHintAddress::class)]
                public array $addresses;
            };

        $result = validate(['addresses' => [['street' => 'Main St']]], $model);
        expect($result->addresses[0])->toBeInstanceOf(ArrayTypeHintAddress::class);
        expect($result->addresses[0]->street)->toBe('Main St');
    });

    it('reports nested model errors with dot notation paths', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(ArrayTypeHintAddress::class)]
                public array $addresses;
            };

        try {
            validate(['addresses' => [['street' => 'Main St'], []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'addresses.1.street' => ['Field is required'],
            ]);
        }
    });

    it('coerces string elements to DateTime objects', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Sequence(DateTime::class)]
                public array $birthdays;
            };

        $result = validate(['birthdays' => ['1994-01-01T09:00:00+00:00']], $model);
        expect($result->birthdays[0])->toBeInstanceOf(DateTimeInterface::class);
    });

    it('accepts self-referencing model recursion', function () {
        $result = validate([
            'name' => 'root',
            'children' => [
                ['name' => 'child', 'children' => []],
            ],
        ], new ArrayTypeHintSelfNode);

        expect($result->children[0])->toBeInstanceOf(ArrayTypeHintSelfNode::class);
        expect($result->children[0]->name)->toBe('child');
    });

    it('accepts elements matching every class of an Intersection', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[Intersection(
                    ArrayTypeHintCountable::class,
                    ArrayTypeHintIterable::class,
                )]
                public array $items;
            };

        $result = validate(['items' => [new ArrayTypeHintIterableCountable]], $model);
        expect($result->items[0])->toBeInstanceOf(ArrayTypeHintCountable::class);

        try {
            validate(['items' => [(object) []]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'items.0' => [
                    'Must be Attributes\Validation\Tests\Integration\Validate\TypeHint\ArrayTypeHintCountable and'
                        .' Attributes\Validation\Tests\Integration\Validate\TypeHint\ArrayTypeHintIterable',
                ],
            ]);
        }
    });
});

describe('type-hint array validation via attributes (strict mode)', function () {
    it('rejects scalar coercion of array elements', function () {
        $model = new
            #[\Attributes\Validation\ModelConfigs(strict: true)]
            class extends BaseModel {
                #[Sequence(Type::int)]
                public array $scores;
            };

        try {
            validate(['scores' => ['30', 50]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'scores.0' => ['Must be integer'],
            ]);
        }
    });

    it('accepts elements already of the declared type', function () {
        $model = new
            #[\Attributes\Validation\ModelConfigs(strict: true)]
            class extends BaseModel {
                #[Sequence(Type::int)]
                public array $scores;
            };

        $result = validate(['scores' => [30, 50]], $model);
        expect($result->scores)->toBe([30, 50]);
    });

    it('still hydrates nested models from raw arrays', function () {
        $model = new
            #[\Attributes\Validation\ModelConfigs(strict: true)]
            class extends BaseModel {
                #[Sequence(ArrayTypeHintAddress::class)]
                public array $addresses;
            };

        $result = validate(['addresses' => [['street' => 'Main St']]], $model);
        expect($result->addresses[0])->toBeInstanceOf(ArrayTypeHintAddress::class);
    });

    it('still coerces datetime strings for DateTime element types', function () {
        $model = new
            #[\Attributes\Validation\ModelConfigs(strict: true)]
            class extends BaseModel {
                #[Sequence(DateTime::class)]
                public array $birthdays;
            };

        $result = validate(['birthdays' => ['1994-01-01T09:00:00+00:00']], $model);
        expect($result->birthdays[0])->toBeInstanceOf(DateTimeInterface::class);
    });
});

describe('type-hint array attribute error messages', function () {
    it('applies custom ErrorMessage templates to element errors', function () {
        $model = new class extends BaseModel {
            #[ErrorMessage(type: 'Ups wrong type {expected}')]
            #[Union(bool, int, string)]
            public array $values;
        };

        try {
            validate(['values' => [[]]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect($e->getErrors())->toBe([
                'values.0' => ['Ups wrong type boolean, integer or string'],
            ]);
        }
    });

    it('propagates a spec error instead of masking it with an empty ValidationException', function () {
        $model = new class extends BaseModel {
            #[Union(Type::bool, new ArrayTypeHintMissingAttribute)]
            public array $values;
        };

        try {
            validate(['values' => [true]], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect(false)->toBeTrue(); // the spec error must not surface as an empty ValidationException
        } catch (\Error $e) {
            expect($e->getMessage())->toContain('ArrayTypeHintMissingAttribute');
        }
    });

    it('throws a ValueError when a class arm cannot be resolved', function () {
        $model = new class extends BaseModel {
            #[Sequence(ArrayTypeHintNowhere::class)]
            public array $items;
        };

        try {
            validate(['items' => []], $model);
            expect(false)->toBeTrue();
        } catch (ValueError $e) {
            expect($e->getMessage())->toContain('ArrayTypeHintNowhere');
        }
    });

    it('throws a ValueError when a Dict arm references a missing Dict attribute', function () {
        $model = new class extends BaseModel {
            #[Union(bool, Dict::class)]
            public array $values;
        };

        validate(['values' => [true]], $model);
    })->throws(ValueError::class);

    it('throws a ValueError when a Dict value type is an unknown Type value', function () {
        $model = new class extends BaseModel {
            #[Dict(key: Type::int, of: 512)]
            public array $keyed;
        };

        validate(['keyed' => []], $model);
    })->throws(ValueError::class);

    it('throws a ValueError when an arm type is unsupported', function () {
        $model = new class extends BaseModel {
            #[Union(3.14)]
            public array $values;
        };

        validate(['values' => []], $model);
    })->throws(ValueError::class);

    it('throws a ValueError when an Intersection arm is not a class name', function () {
        $model = new class extends BaseModel {
            #[Intersection(Type::int, stdClass::class)]
            public array $items;
        };

        validate(['items' => []], $model);
    })->throws(ValueError::class);

    it('throws a ValueError when a Dict key type is not an integer or string', function () {
        $model = new class extends BaseModel {
            #[Dict(key: new Dict(key: Type::int, of: Type::int), of: Type::int)]
            public array $keyed;
        };

        validate(['keyed' => []], $model);
    })->throws(ValueError::class);
});
