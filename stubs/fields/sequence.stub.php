<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
    #[Attribute]
    class Sequence implements Field {
        public function __construct(public int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict $of) {}
    }
}
