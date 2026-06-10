--TEST--
Test that validate method is callable
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

$validator = new Attributes\Validation\Validator();
var_dump(is_callable([$validator, 'validate']));

?>
--EXPECT--
bool(true)
