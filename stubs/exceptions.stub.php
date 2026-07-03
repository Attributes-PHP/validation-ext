<?php
/** @generate-class-entries */

namespace Attributes\Validation\Exceptions {
   class BaseException extends \Exception {}

   class ValidationException extends BaseException {
      private array $allErrors;

      public function getAllErrors(): array {}
   }
}