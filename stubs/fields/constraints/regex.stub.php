<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    final class Regex implements Field {
        public function __construct(public string $regex) {}
    }
}
