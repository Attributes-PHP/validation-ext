--TEST--
Test that validateCallable method exists in Validator class
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

$validator = new Attributes\Validation\Validator();
var_dump(method_exists($validator, 'validateCallable'));

?>
--EXPECT--
bool(true)
