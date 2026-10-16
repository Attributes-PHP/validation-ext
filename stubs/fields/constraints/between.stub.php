<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Between implements Field {
        public function __construct(public int|float $min, public int|float $max) {}
    }
}
