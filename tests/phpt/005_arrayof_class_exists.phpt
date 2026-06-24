--TEST--
Test that ArrayOf class exists in Attributes\Validation namespace
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

var_dump(class_exists('Attributes\Validation\ArrayOf'));

?>
--EXPECT--
bool(true)
