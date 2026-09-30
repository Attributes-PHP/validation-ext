<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
    #[Attribute]
    class Dict implements Field {
        public function __construct(public int|\Attributes\Validation\Type|Union $key, public int|string|\Attributes\Validation\Type|Union|Intersection|Sequence|Dict $of) {}
    }
}
