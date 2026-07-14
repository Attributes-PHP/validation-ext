--TEST--
Test that call function exists in Attributes\Validation namespace
--INI--
extension=modules/attributes_validation.so
--FILE--
<?php

var_dump(function_exists('Attributes\Validation\call'));

?>
--EXPECT--
bool(true)
