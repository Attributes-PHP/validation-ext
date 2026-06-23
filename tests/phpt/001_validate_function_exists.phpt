--TEST--
Test that validate function exists in Attributes\Validation namespace
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

var_dump(function_exists('Attributes\Validation\validate'));

?>
--EXPECT--
bool(true)
