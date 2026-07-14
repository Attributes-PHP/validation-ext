--TEST--
Test that validate function exists in Attributes\Validation namespace
--INI--
extension=modules/attributes_validation.so
--FILE--
<?php

use function Attributes\Validation\validate;
use Attributes\Validation\BaseModel;
use Attributes\Validation\ModelConfigs;

#[ModelConfigs(extra: 'forbid')]
class NoHooks extends BaseModel
{
    public float $number;
}

class WithBeforeHook extends NoHooks {
    public function beforeValidation(array $rawData, ModelConfigs $configs) {
        var_dump($this::class . '->beforeValidation(...) - no return value');
    }
}

class WithAfterHook extends NoHooks {
    public function afterValidation(array $rawData, ModelConfigs $configs) {
        var_dump($this::class . '->afterValidation(...)');
    }
}

class WithBothHooks extends NoHooks {
    public function beforeValidation(array $rawData, ModelConfigs $configs) {
        var_dump($this::class . '->beforeValidation(...)');
        var_dump($rawData);
        return ['number' => 'invalid-value'];
    }

    public function afterValidation(array $rawData, ModelConfigs $configs) {
        var_dump($this::class . '->afterValidation(...)');
        var_dump($rawData);
    }
}

$rawData = ['number' => '1.23'];
validate($rawData, new NoHooks);
validate($rawData, new WithBeforeHook);
validate($rawData, new WithAfterHook);
validate($rawData, new WithBothHooks);

?>
--EXPECT--
string(55) "WithBeforeHook->beforeValidation(...) - no return value"
string(35) "WithAfterHook->afterValidation(...)"
string(36) "WithBothHooks->beforeValidation(...)"
array(1) {
  ["number"]=>
  string(4) "1.23"
}
string(35) "WithBothHooks->afterValidation(...)"
array(1) {
  ["number"]=>
  string(13) "invalid-value"
}
