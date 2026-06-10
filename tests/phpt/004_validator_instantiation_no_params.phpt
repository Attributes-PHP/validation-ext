--TEST--
Test that Validator can be instantiated without parameters
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

try {
    $validator = new Attributes\Validation\Validator();
    var_dump(true);
} catch (Exception $e) {
    var_dump(false);
} catch (Error $e) {
    var_dump(false);
}

?>
--EXPECT--
bool(true)
