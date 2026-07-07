<?php
/** @generate-class-entries */

namespace Attributes\Validation {
   abstract class BaseModel {
      public function beforeValidation(array $rawData): array {}
      public function afterValidation(array $rawData): void {}
      public function getModelConfigs(): ModelConfigs {}
   }
}
