<?php
/** @generate-class-entries */

namespace Attributes\Validation {
   #[Attribute(Attribute::TARGET_PROPERTY|Attribute::TARGET_PARAMETER)]
   class ArrayOf {
      private array $type;
      public function __construct(string ...$type) {}
   }
}