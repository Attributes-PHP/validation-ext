<?php
/** @generate-class-entries */

namespace Attributes\Validation {
   #[Attribute(Attribute::TARGET_CLASS)]
   abstract class ModelConfigs {
      public function __construct(
         public readonly ?string $aliasGenerator = null,
         public readonly bool $strToLower = false,
         public readonly bool $strToUpper = false,
         public readonly bool $stripWhitespace = false,
         public readonly string $extra = "ignore",
         public readonly bool $strict = false,
         public readonly bool $stopAtFirstError = false,
      ) {}
   }
}