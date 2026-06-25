--TEST--
Test that ValidationException exists, can be thrown and has all necessary methods and properties
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

use Attributes\Validation\ValidationException;
use Attributes\Validation\BaseException;

try {
  throw new ValidationException(['number' => 'Invalid number']);
} catch (BaseException $e) {
  var_dump($e);
  var_dump($e->getErrors());
}

?>
--EXPECT--
object(Attributes\Validation\ValidationException)#1 (8) {
  ["message":protected]=>
  string(12) "Invalid data"
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(84) "/Users/andregil/Desktop/others/validation-ext/tests/phpt/007_validationexception.php"
  ["line":protected]=>
  int(7)
  ["trace":"Exception":private]=>
  array(0) {
  }
  ["previous":"Exception":private]=>
  NULL
  ["allErrors":"Attributes\Validation\ValidationException":private]=>
  array(1) {
    ["number"]=>
    string(14) "Invalid number"
  }
}
array(1) {
  ["number"]=>
  string(14) "Invalid number"
}
