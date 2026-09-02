<?php
/** @generate-class-entries */

namespace Attributes\Validation {
   // Attribute::TARGET_CLASS
   #[Attribute]
   abstract class ModelConfigs {
      private ?string $aliasGenerator = null;
      private bool $strToLower = false;
      private bool $strToUpper = false;
      private bool $stripWhitespace = false;
      private string $extra = "ignore";
      private bool $strict = false;
      private bool $stopAtFirstError = false;
      private static ?callable $defaultErrorMessages = null;

      public function getAliasGenerator(): ?string {}
      public function isStrToLower(): bool {}
      public function isStrToUpper(): bool {}
      public function isStripWhitespace(): bool {}
      public function getExtra(): string {}
      public function isStrict(): bool {}
      public function isStopAtFirstError(): bool {}

      public static function setDefaultErrorMessages(callable $setErrorMessages): void {}
   }
}