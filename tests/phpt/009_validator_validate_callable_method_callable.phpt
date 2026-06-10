--TEST--
Test that validateCallable method is callable
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

$validator = new Attributes\Validation\Validator();
var_dump(is_callable([$validator, 'validateCallable']));

?>
--EXPECT--
bool(true)
