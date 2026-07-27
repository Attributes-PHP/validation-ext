--TEST--
Test that ValidationException exists, can be thrown and has all necessary methods and properties
--EXTENSIONS--
attributes_validation
--FILE--
<?php

use Attributes\Validation\Exceptions\ValidationException;
use Attributes\Validation\Exceptions\BaseException;

try {
  throw new ValidationException(['number' => 'Invalid number']);
} catch (BaseException $e) {
  var_dump($e);
  var_dump($e->getErrors());
}

?>
--EXPECT--
object(Attributes\Validation\Exceptions\ValidationException)#1 (8) {
  ["message":protected]=>
  string(12) "Invalid data"
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(84) "/Users/andregil/Desktop/others/validation-ext/tests/phpt/008_validationexception.php"
  ["line":protected]=>
  int(7)
  ["trace":"Exception":private]=>
  array(0) {
  }
  ["previous":"Exception":private]=>
  NULL
  ["allErrors":"Attributes\Validation\Exceptions\ValidationException":private]=>
  array(1) {
    ["number"]=>
    string(14) "Invalid number"
  }
}
array(1) {
  ["number"]=>
  string(14) "Invalid number"
}
