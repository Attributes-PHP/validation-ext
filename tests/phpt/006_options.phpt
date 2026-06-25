--TEST--
Test that Options is instantiable and holds all needed options for validation
--INI--
extension=modules/validation_ext.so
--FILE--
<?php

use Attributes\Validation\Options;

$options = new Options(isStrict: true, isToStopAtFirstError: true);
var_dump($options->isStrict());
var_dump($options->isToStopAtFirstError());

?>
--EXPECT--
bool(true)
bool(true)
