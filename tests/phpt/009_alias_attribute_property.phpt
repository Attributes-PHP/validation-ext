--TEST--
Test that Alias is a valid PHP Attribute for class properties
--EXTENSIONS--
attributes_validation
--FILE--
<?php

use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\Field;

class TestClass {
    #[Alias('user_name')]
    public string $name;
}

$reflection = new ReflectionClass('TestClass');
$property = $reflection->getProperty('name');
$allAttributes = $property->getAttributes();
$instance = $allAttributes[0]->newInstance();
var_dump($instance instanceof Alias);
var_dump($instance instanceof Field);
var_dump($instance->name);

?>
--EXPECT--
bool(true)
bool(true)
string(9) "user_name"
