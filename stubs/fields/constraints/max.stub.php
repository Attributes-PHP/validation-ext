<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Max implements Field {
        public function __construct(public int|float $value) {}
    }
}
