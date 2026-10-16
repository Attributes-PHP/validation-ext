<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class EndsWith implements Field {
        public array $values;

        public function __construct(string|int|float ...$values) {}
    }
}
