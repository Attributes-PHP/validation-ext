--TEST--
Test that BaseModel has all required methods
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

use Attributes\Validation\BaseModel;

class CustomModel extends BaseModel {}

$model = new CustomModel;
var_dump(method_exists($model, 'beforeValidation'));
var_dump(method_exists($model, 'afterValidation'));

?>
--EXPECT--
bool(true)
bool(true)
