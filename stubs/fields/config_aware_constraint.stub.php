<?php
/** @generate-class-entries */

namespace Attributes\Validation\Fields {
   interface ConfigAwareConstraint extends Constraint {
      public function setModelConfigs(\Attributes\Validation\ModelConfigs $configs): void;
   }
}
