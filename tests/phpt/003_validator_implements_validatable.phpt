--TEST--
Test that Validator implements Validatable interface
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

$reflection = new ReflectionClass('Attributes\Validation\Validator');
var_dump($reflection->implementsInterface('Attributes\Validation\Validatable'));

?>
--EXPECT--
bool(true)
