<?php

declare(strict_types=1);

namespace Attributes\Validation\Tests\Integration\Constraints;

use Attributes\Validation\BaseModel;
use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Fields\ConfigAware;
use Attributes\Validation\ModelConfigs;
use RuntimeException;

use function Attributes\Validation\validate;

#[Attribute(Attribute::TARGET_PROPERTY)]
final class TracksCalls implements ConfigAware
{
    /** @var list<string> */
    public static array $calls = [];
    public static ?ModelConfigs $configs = null;

    public function setModelConfigs(ModelConfigs $configs): void
    {
        self::$calls[] = 'setModelConfigs';
        self::$configs = $configs;
    }

    public function validate(mixed $value): bool
    {
        self::$calls[] = 'validate';

        return $value === 'ok';
    }

    public function message(): string
    {
        self::$calls[] = 'message';

        return '{field} rejected by '.(self::$configs?->isStrict() ? 'strict' : 'loose').' validation';
    }
}

#[Attribute(Attribute::TARGET_PROPERTY)]
final class RejectingConfigs implements ConfigAware
{
    public function setModelConfigs(ModelConfigs $configs): void
    {
        throw new RuntimeException('configs rejected');
    }

    public function validate(mixed $value): bool
    {
        return true;
    }

    public function message(): string
    {
        return 'unreachable';
    }
}

describe('ConfigAware constraints', function () {
    beforeEach(function () {
        TracksCalls::$calls = [];
        TracksCalls::$configs = null;
    });

    it('calls setModelConfigs before validate() on the success path', function () {
        $model = new class extends BaseModel {
            #[TracksCalls]
            public string $value;
        };

        $result = validate(['value' => 'ok'], $model);

        expect($result->value)->toBe('ok');
        expect(TracksCalls::$calls)->toBe(['setModelConfigs', 'validate']);
        expect(TracksCalls::$configs)->toBeInstanceOf(ModelConfigs::class);
        expect(TracksCalls::$configs->isStrict())->toBeTrue();
    });

    it('calls setModelConfigs before validate() and message() on the failure path', function () {
        $model = new
            #[ModelConfigs(strict: false)]
            class extends BaseModel {
                #[TracksCalls]
                public string $value;
            };

        try {
            validate(['value' => 'nope'], $model);
            expect(false)->toBeTrue();
        } catch (ValidationException $e) {
            expect(TracksCalls::$calls)->toBe(['setModelConfigs', 'validate', 'message']);
            expect(TracksCalls::$configs)->toBeInstanceOf(ModelConfigs::class);
            expect(TracksCalls::$configs->isStrict())->toBeFalse();
            expect($e->getErrors())->toBe(['value' => ['value rejected by loose validation']]);
        }
    });

    it('propagates exceptions thrown by setModelConfigs', function () {
        $model = new class extends BaseModel {
            #[RejectingConfigs]
            public string $value;
        };

        expect(fn () => validate(['value' => 'ok'], $model))->toThrow(RuntimeException::class, 'configs rejected');
    });
});
