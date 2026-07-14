--TEST--
Test that BaseException exists and can be thrown
--INI--
extension=modules/attributes_validation.so
--FILE--
<?php

use Attributes\Validation\BaseException;

try {
  throw new BaseException('Testing');
} catch (BaseException $e) {
  var_dump($e);
}

?>
--EXPECT--
object(Attributes\Validation\BaseException)#1 (7) {
  ["message":protected]=>
  string(7) "Testing"
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(78) "/Users/andregil/Desktop/others/validation-ext/tests/phpt/007_baseexception.php"
  ["line":protected]=>
  int(6)
  ["trace":"Exception":private]=>
  array(0) {
  }
  ["previous":"Exception":private]=>
  NULL
}
