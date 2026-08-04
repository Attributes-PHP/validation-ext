#include "unity.h"
#include "test_structs.h"
#include "test_helpers.h"
#include "validate_function.h"
#include "model_configs.h"
#include "Zend/zend_types.h"

// Mock structures and functions for testing
// We need to mock Zend engine structures for testing

void setUp(void) {
    // This is run before EACH test
}

void tearDown(void) {
    // This is run after EACH test
}

// ============= add_field_error Tests =============

void test_add_field_error_adds_error_to_array(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test_field", sizeof("test_field") - 1, 0);
    
    add_field_error(&errors, field_name, "test error", sizeof("test error") - 1);
    
    // Check that error was added
    TEST_ASSERT_EQUAL(1, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    
    // Check that the error message is correct
    zval *error_msg = zend_hash_str_find(Z_ARRVAL_P(&errors), "test_field", sizeof("test_field") - 1);
    TEST_ASSERT_NOT_NULL(error_msg);
    TEST_ASSERT_EQUAL_STRING("test error", Z_STRVAL_P(error_msg));
    
    zend_string_release(field_name);
    zval_ptr_dtor(&errors);
}

void test_add_field_error_with_different_field_names(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field1 = zend_string_init("field1", sizeof("field1") - 1, 0);
    zend_string *field2 = zend_string_init("field2", sizeof("field2") - 1, 0);
    
    add_field_error(&errors, field1, "error1", sizeof("error1") - 1);
    add_field_error(&errors, field2, "error2", sizeof("error2") - 1);
    
    TEST_ASSERT_EQUAL(2, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    
    zend_string_release(field1);
    zend_string_release(field2);
    zval_ptr_dtor(&errors);
}

// ============= validate_type Tests =============

// Helper to create a zval of a specific type
static zval* create_test_zval(zend_uchar type, void *value) {
    zval *z = emalloc(sizeof(zval));
    
    switch (type) {
        case IS_LONG:
            ZVAL_LONG(z, *(zend_long*)value);
            break;
        case IS_DOUBLE:
            ZVAL_DOUBLE(z, *(double*)value);
            break;
        case IS_STRING:
            ZVAL_STRING(z, zend_string_init(*(char**)value, strlen(*(char**)value), 0));
            break;
        case IS_BOOL:
            ZVAL_BOOL(z, *(zend_bool*)value);
            break;
        case IS_NULL:
            ZVAL_NULL(z);
            break;
        default:
            ZVAL_NULL(z);
    }
    
    return z;
}

void test_validate_type_strict_mode_rejects_mismatch(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    zend_long long_val = 42;
    zval *value = create_test_zval(IS_LONG, &long_val);
    
    // Try to validate as string in strict mode
    bool result = validate_type(value, IS_STRING, &errors, field_name, true);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(1, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    
    zend_string_release(field_name);
    zval_ptr_dtor(value);
    zval_ptr_dtor(&errors);
}

void test_validate_type_strict_mode_accepts_match(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    zend_long long_val = 42;
    zval *value = create_test_zval(IS_LONG, &long_val);
    
    // Validate as long in strict mode
    bool result = validate_type(value, IS_LONG, &errors, field_name, true);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    
    zend_string_release(field_name);
    zval_ptr_dtor(value);
    zval_ptr_dtor(&errors);
}

void test_validate_type_non_strict_coerces_string_to_long(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    char *str_val = "42";
    zval *value = create_test_zval(IS_STRING, &str_val);
    
    // Validate as long in non-strict mode (should coerce)
    bool result = validate_type(value, IS_LONG, &errors, field_name, false);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    TEST_ASSERT_EQUAL(IS_LONG, Z_TYPE_P(value));
    TEST_ASSERT_EQUAL(42, Z_LVAL_P(value));
    
    zend_string_release(field_name);
    zval_ptr_dtor(value);
    zval_ptr_dtor(&errors);
}

void test_validate_type_non_strict_coerces_long_to_string(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    zend_long long_val = 42;
    zval *value = create_test_zval(IS_LONG, &long_val);
    
    // Validate as string in non-strict mode (should coerce)
    bool result = validate_type(value, IS_STRING, &errors, field_name, false);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    TEST_ASSERT_EQUAL(IS_STRING, Z_TYPE_P(value));
    TEST_ASSERT_EQUAL_STRING("42", Z_STRVAL_P(value));
    
    zend_string_release(field_name);
    zval_ptr_dtor(value);
    zval_ptr_dtor(&errors);
}

void test_validate_type_non_strict_coerces_string_to_double(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    char *str_val = "3.14";
    zval *value = create_test_zval(IS_STRING, &str_val);
    
    // Validate as double in non-strict mode (should coerce)
    bool result = validate_type(value, IS_DOUBLE, &errors, field_name, false);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    TEST_ASSERT_EQUAL(IS_DOUBLE, Z_TYPE_P(value));
    TEST_ASSERT_EQUAL_DOUBLE(3.14, Z_DVAL_P(value), 0.001);
    
    zend_string_release(field_name);
    zval_ptr_dtor(value);
    zval_ptr_dtor(&errors);
}

void test_validate_type_non_strict_coerces_long_to_double(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    zend_long long_val = 42;
    zval *value = create_test_zval(IS_LONG, &long_val);
    
    // Validate as double in non-strict mode (should coerce)
    bool result = validate_type(value, IS_DOUBLE, &errors, field_name, false);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    TEST_ASSERT_EQUAL(IS_DOUBLE, Z_TYPE_P(value));
    TEST_ASSERT_EQUAL_DOUBLE(42.0, Z_DVAL_P(value), 0.001);
    
    zend_string_release(field_name);
    zval_ptr_dtor(value);
    zval_ptr_dtor(&errors);
}

void test_validate_type_non_strict_coerces_various_to_bool(void) {
    zval errors;
    array_init(&errors);
    
    zend_string *field_name = zend_string_init("test", sizeof("test") - 1, 0);
    
    // Test with string "1"
    char *str_val = "1";
    zval *value = create_test_zval(IS_STRING, &str_val);
    bool result = validate_type(value, IS_BOOL, &errors, field_name, false);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(Z_BVAL_P(value));
    zval_ptr_dtor(value);
    
    // Test with long 0
    zend_long long_val = 0;
    value = create_test_zval(IS_LONG, &long_val);
    result = validate_type(value, IS_BOOL, &errors, field_name, false);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(Z_BVAL_P(value));
    zval_ptr_dtor(value);
    
    // Test with double 1.0
    double double_val = 1.0;
    value = create_test_zval(IS_DOUBLE, &double_val);
    result = validate_type(value, IS_BOOL, &errors, field_name, false);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(Z_BVAL_P(value));
    zval_ptr_dtor(value);
    
    TEST_ASSERT_EQUAL(0, zend_hash_num_elements(Z_ARRVAL_P(&errors)));
    
    zend_string_release(field_name);
    zval_ptr_dtor(&errors);
}

// ============= Helper function to check if type name matches =============

static bool type_name_matches(const char *expected, const char *actual) {
    // Simple comparison for basic types
    return strcmp(expected, actual) == 0;
}
