--TEST--
Test that ArrayOf is a valid PHP Attribute for class properties
--EXTENSIONS--
attributes_validation
--FILE--
<?php

use Attributes\Validation\Types\ArrayOf;

class TestClass {
    #[ArrayOf('string')]
    public array $data;
}

$reflection = new ReflectionClass('TestClass');
$property = $reflection->getProperty('data');
$allAttributes = $property->getAttributes();
$instance = $allAttributes[0]->newInstance();
var_dump($instance instanceof ArrayOf);
var_dump($instance);

?>
--EXPECT--
bool(true)
object(Attributes\Validation\Types\ArrayOf)#4 (1) {
  ["type"]=>
  array(1) {
    [0]=>
    string(6) "string"
  }
}
