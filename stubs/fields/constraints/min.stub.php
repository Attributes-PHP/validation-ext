<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    class Min implements Field {
        public function __construct(public int|float $value) {}
    }
}
