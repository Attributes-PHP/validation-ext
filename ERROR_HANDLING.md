# Future Enhancements

1. **Custom Error Messages via Attributes:**

In properties:
   ```php
   #[ErrorMessage("required", "Custom {field} error message")]
   public string $field;
   ```

Or directly in BaseModel to apply to all required rules:
 ```php
    #[ErrorMessage("required", "Custom {field} error message")]
    class MyModel extends BaseModel {
        public string $field;
    }
   ```


2. **Include Valid Values in Enum Errors:**
   ```
   "The selected role must be one of: coach, player."
   ```

3. **Error Codes:**
   Add error codes to distinguish between different types of validation failures.

4. **Localization Support:**
   Allow custom error message handlers for i18n.

5. **Strict Mode Error Messages:**
   Differentiate between strict mode and loose mode errors.

## References

- Zend Engine Type System: https://github.com/php/php-src/blob/master/Zend/zend_types.h
- PHP Extension Writing Guide: https://www.php.net/manual/en/internals2.php
- PHP Internals Book: https://www.phpinternalsbook.com/
