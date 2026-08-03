<?php
/** @generate-class-entries */

namespace Attributes\Validation\Rules {
   interface Custom {
      /**
       * Validates the value and returns true if valid, false otherwise.
       * Can also throw ValidationException for more detailed error messages.
       */
      public function validate(mixed $value): bool;
   }
}
