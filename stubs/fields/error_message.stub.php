<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   #[Attribute]
   class ErrorMessage implements Field {
      public function __construct(public string $required, public string $type) {}
   }
}