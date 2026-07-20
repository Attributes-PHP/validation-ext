<?php
/** @generate-class-entries */

namespace Attributes\Validation\Types {
   #[Attribute(Attribute::TARGET_PROPERTY|Attribute::TARGET_PARAMETER)]
   class ArrayOf {
      public function __construct(private string ...$type) {}
   }
}