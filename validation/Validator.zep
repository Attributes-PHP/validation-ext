namespace Validation;

use Validation\Model\Base;

class Validator
{
    private isStrict;
    private stopFirstError;

    public function __construct(boolean isStrict = false, boolean stopFirstError = false) {
        let this->isStrict = isStrict;
        let this->stopFirstError = stopFirstError;
    }

    public function validate(array rawData) -> Base
    {
        return new Base;
    }
}
