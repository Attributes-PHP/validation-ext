<?php
/** @generate-class-entries */

namespace Attributes\Validation {
   abstract class BaseModel {
      protected function beforeValidation(array $rawData, ModelConfigs $configs): array {}
      protected function afterValidation(array $rawData, ModelConfigs $configs): void {}
   }
}
