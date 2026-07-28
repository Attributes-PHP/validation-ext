--TEST--
Test that Alias is a valid PHP Attribute for function parameters
--EXTENSIONS--
attributes_validation
--FILE--
<?php

use Attributes\Validation\Fields\Alias;
use Attributes\Validation\Fields\Field;

function test_function(#[Alias('email_address')] string $email) {
    return $email;
}

$reflection = new ReflectionFunction('test_function');
$allParameters = $reflection->getParameters();
$allAttributes = $allParameters[0]->getAttributes();
$instance = $allAttributes[0]->newInstance();
var_dump($instance instanceof Alias);
var_dump($instance instanceof Field);
var_dump($instance->name);

?>
--EXPECT--
bool(true)
bool(true)
string(13) "email_address"
