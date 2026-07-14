--TEST--
Test that ArrayOf is a valid PHP Attribute for function parameters
--INI--
extension=modules/attributes_validation.so
--FILE--
<?php

use Attributes\Validation\ArrayOf;

function test_function(#[ArrayOf('float', 'int')] array $data) {
    return $data;
}

$reflection = new ReflectionFunction('test_function');
$allParameters = $reflection->getParameters();
$allAttributes = $allParameters[0]->getAttributes();
$instance = $allAttributes[0]->newInstance();
var_dump($instance instanceof ArrayOf);
var_dump($instance);

?>
--EXPECT--
bool(true)
object(Attributes\Validation\ArrayOf)#4 (1) {
  ["type"]=>
  array(2) {
    [0]=>
    string(5) "float"
    [1]=>
    string(3) "int"
  }
}
