<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
    #[Attribute]
    class Intersection implements Field {
        public array $of;

        public function __construct(string ...$of) {}
    }
}
