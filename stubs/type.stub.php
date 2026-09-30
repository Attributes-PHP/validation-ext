<?php
/** @generate-class-entries */

namespace Attributes\Validation {
    /*
     * The backing values are the Zend engine MAY_BE_* type masks, so C code
     * can compare them against engine masks directly. dict has no engine
     * mask (the engine only knows array) and carries a private bit the
     * validator folds into MAY_BE_ARRAY.
     */
    enum Type: int {
        case bool = 12;      // MAY_BE_BOOL
        case int = 16;       // MAY_BE_LONG
        case float = 32;     // MAY_BE_DOUBLE
        case string = 64;    // MAY_BE_STRING
        case sequence = 128; // MAY_BE_ARRAY
        case dict = 268435456; // private dict bit
        case object = 256;   // MAY_BE_OBJECT
    }
}
