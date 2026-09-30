<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   #[Attribute]
   class Datetime implements Field {
      public function __construct(public string $format = 'X-m-d\\TH:i:sP', public ?DateTimeZone $timezone = null) {}
   }
}