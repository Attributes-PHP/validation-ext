# Future Enhancements

1. **Custom Error Messages via Attributes:**
   ```php
   #[ErrorMessage("Custom error message")]
   public string $field;
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
