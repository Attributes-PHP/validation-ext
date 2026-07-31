<?php

define('TESTS_RUNNING', true);

if (! extension_loaded('attributes_validation')) {
    exit('attributes_validation extension is not loaded');
}

uses(__DIR__.'/tests')->group('validation');
