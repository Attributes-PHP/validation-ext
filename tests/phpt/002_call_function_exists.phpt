--TEST--
Test that call function exists in Attributes\Validation namespace
--EXTENSIONS--
attributes_validation
--FILE--
<?php

var_dump(function_exists('Attributes\Validation\call'));

?>
--EXPECT--
bool(true)
