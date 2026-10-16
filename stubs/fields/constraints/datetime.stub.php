<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
   #[Attribute]
   final class DateTime implements Field {
      public function __construct(public string $format = 'X-m-d\\TH:i:sP', public \DateTimeZone|string|null $timezone = null) {}
   }
}