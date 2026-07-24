--TEST--
Test that validate function exists in Attributes\Validation namespace
--EXTENSIONS--
attributes_validation
--FILE--
<?php

var_dump(function_exists('Attributes\Validation\validate'));

?>
--EXPECT--
bool(true)
