<?php
/** @generate-class-entries */

namespace Attributes\Validation\Exceptions {
    class BaseException extends \Exception {}

    class ValidationException extends BaseException {
       public function __construct(array $allErrors) {}
       public function getErrors(): array {}
    }
}