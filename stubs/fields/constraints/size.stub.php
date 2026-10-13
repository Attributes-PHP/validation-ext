<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields\Constraints {
    #[Attribute]
    class Size implements Field {
        public function __construct(public ?int $min = null, public ?int $max = null) {}
    }
}
