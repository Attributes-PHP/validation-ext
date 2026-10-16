<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class StartsWith implements Field {
        public array $values;

        public function __construct(string|int|float ...$values) {}
    }
}
