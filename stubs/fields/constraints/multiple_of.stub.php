<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class MultipleOf implements Field {
        public function __construct(public int|float $value) {}
    }
}
