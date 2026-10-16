<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   interface Constraint extends Field {
      public function validate(mixed $value): bool;
      public function message(): string;
   }
}
