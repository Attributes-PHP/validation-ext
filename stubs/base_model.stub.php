<?php
/** @generate-class-entries */

namespace Attributes\Validation {
   abstract class BaseModel {
      public function beforeValidation(array $rawData, Options $options): array {}
      public function afterValidation(array $rawData, Options $options): void {}
   }
}
