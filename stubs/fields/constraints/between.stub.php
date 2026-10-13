<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    class Between implements Field {
        public function __construct(public int|float $min, public int|float $max) {}
    }
}
