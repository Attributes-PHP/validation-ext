--TEST--
Test that validate method exists in Validator class
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

$validator = new Attributes\Validation\Validator();
var_dump(method_exists($validator, 'validate'));

?>
--EXPECT--
bool(true)
