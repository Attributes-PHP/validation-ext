<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   #[Attribute(Attribute::TARGET_PROPERTY|Attribute::TARGET_PARAMETER)]
   class ArrayOf implements Field {
      /**
       * @param class-string|string ...$types
       */
      public function __construct(public string ...$types) {}
   }
}
