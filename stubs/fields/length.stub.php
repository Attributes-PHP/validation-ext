<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   #[Attribute(Attribute::TARGET_PROPERTY|Attribute::TARGET_PARAMETER)]
   class Length implements Field {
      public function __construct(public ?int $min = null, public ?int $max = null) {}
   }
}
