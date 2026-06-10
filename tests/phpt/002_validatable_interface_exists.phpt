--TEST--
Test that Validatable interface exists in Attributes\Validation namespace
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

var_dump(interface_exists('Attributes\Validation\Validatable'));

?>
--EXPECT--
bool(true)
