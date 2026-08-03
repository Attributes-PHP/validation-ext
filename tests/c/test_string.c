#include "unity.h"
#include "test_structs.h"
#include "test_helpers.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_string.h"

void setUp(void) {
    // This is run before EACH test
    av_string_init_Stub(string_init_stub);
    av_string_release_Stub(string_release_stub);
    av_emalloc_Stub(emalloc_stub);
    av_efree_Stub(efree_stub);
}

void tearDown(void) {
    // This is run after EACH test
}

// ============= PascalCase Tests =============

void test_to_pascal_case_snake_case(void) {
    zend_string* input = av_string_init("first_name", sizeof("first_name") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("FirstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_kebab_case(void) {
    zend_string* input = av_string_init("first-name", sizeof("first-name") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("FirstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_space(void) {
    zend_string* input = av_string_init("first name", sizeof("first name") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("FirstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_camel_case(void) {
    zend_string* input = av_string_init("firstName", sizeof("firstName") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("FirstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_already_pascal(void) {
    zend_string* input = av_string_init("FirstName", sizeof("FirstName") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("FirstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_multiple_separators(void) {
    zend_string* input = av_string_init("first_name_last-name", sizeof("first_name_last-name") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("FirstNameLastName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_single_word(void) {
    zend_string* input = av_string_init("name", sizeof("name") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("Name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_empty(void) {
    zend_string* input = av_string_init("", sizeof("") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_pascal_case_numbers(void) {
    zend_string* input = av_string_init("user2_name", sizeof("user2_name") - 1, 0);
    zend_string* result = av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING("User2Name", result->val);
    av_string_release(input);
    av_string_release(result);
}

// ============= camelCase Tests =============

void test_to_camel_case_snake_case(void) {
    zend_string* input = av_string_init("first_name", sizeof("first_name") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("firstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_camel_case_kebab_case(void) {
    zend_string* input = av_string_init("first-name", sizeof("first-name") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("firstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_camel_case_pascal_case(void) {
    zend_string* input = av_string_init("FirstName", sizeof("FirstName") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("firstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_camel_case_already_camel(void) {
    zend_string* input = av_string_init("firstName", sizeof("firstName") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("firstName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_camel_case_single_word(void) {
    zend_string* input = av_string_init("Name", sizeof("Name") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_camel_case_multiple_separators(void) {
    zend_string* input = av_string_init("first_name_last-name", sizeof("first_name_last-name") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("firstNameLastName", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_camel_case_numbers(void) {
    zend_string* input = av_string_init("user2_name", sizeof("user2_name") - 1, 0);
    zend_string* result = av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING("user2Name", result->val);
    av_string_release(input);
    av_string_release(result);
}

// ============= snake_case Tests =============

void test_to_snake_case_pascal_case(void) {
    zend_string* input = av_string_init("FirstName", sizeof("FirstName") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("first_name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_camel_case(void) {
    zend_string* input = av_string_init("firstName", sizeof("firstName") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("first_name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_kebab_case(void) {
    zend_string* input = av_string_init("first-name", sizeof("first-name") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("first_name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_space(void) {
    zend_string* input = av_string_init("first name", sizeof("first name") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("first_name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_already_snake(void) {
    zend_string* input = av_string_init("first_name", sizeof("first_name") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("first_name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_single_word(void) {
    zend_string* input = av_string_init("Name", sizeof("Name") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_consecutive_uppercase(void) {
    zend_string* input = av_string_init("HTTPResponse", sizeof("HTTPResponse") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("h_t_t_p_response", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_snake_case_numbers(void) {
    zend_string* input = av_string_init("user2Name", sizeof("user2Name") - 1, 0);
    zend_string* result = av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING("user2_name", result->val);
    av_string_release(input);
    av_string_release(result);
}

// ============= kebab-case Tests =============

void test_to_kebab_case_pascal_case(void) {
    zend_string* input = av_string_init("FirstName", sizeof("FirstName") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("first-name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_kebab_case_camel_case(void) {
    zend_string* input = av_string_init("firstName", sizeof("firstName") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("first-name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_kebab_case_snake_case(void) {
    zend_string* input = av_string_init("first_name", sizeof("first_name") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("first-name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_kebab_case_space(void) {
    zend_string* input = av_string_init("first name", sizeof("first name") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("first-name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_kebab_case_already_kebab(void) {
    zend_string* input = av_string_init("first-name", sizeof("first-name") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("first-name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_kebab_case_single_word(void) {
    zend_string* input = av_string_init("Name", sizeof("Name") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("name", result->val);
    av_string_release(input);
    av_string_release(result);
}

void test_to_kebab_case_numbers(void) {
    zend_string* input = av_string_init("user2Name", sizeof("user2Name") - 1, 0);
    zend_string* result = av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING("user2-name", result->val);
    av_string_release(input);
    av_string_release(result);
}

// ============= Edge Cases =============

void test_no_input_modification(void) {
    zend_string* input = av_string_init("first_name", sizeof("first_name") - 1, 0);
    const char* original_val = strdup(input->val);
    size_t original_len = input->len;

    av_to_pascal_case(input);
    TEST_ASSERT_EQUAL_STRING(original_val, input->val);
    TEST_ASSERT_EQUAL(original_len, input->len);

    av_to_camel_case(input);
    TEST_ASSERT_EQUAL_STRING(original_val, input->val);
    TEST_ASSERT_EQUAL(original_len, input->len);

    av_to_snake_case(input);
    TEST_ASSERT_EQUAL_STRING(original_val, input->val);
    TEST_ASSERT_EQUAL(original_len, input->len);

    av_to_kebab_case(input);
    TEST_ASSERT_EQUAL_STRING(original_val, input->val);
    TEST_ASSERT_EQUAL(original_len, input->len);

    free((void*)original_val);
    av_string_release(input);
}

// ============= Static Helper Function Tests =============

void test_is_uppercase(void) {
    TEST_ASSERT_TRUE(is_uppercase('A'));
    TEST_ASSERT_TRUE(is_uppercase('Z'));
    TEST_ASSERT_TRUE(is_uppercase('M'));
    TEST_ASSERT_FALSE(is_uppercase('a'));
    TEST_ASSERT_FALSE(is_uppercase('z'));
    TEST_ASSERT_FALSE(is_uppercase('0'));
    TEST_ASSERT_FALSE(is_uppercase(' '));
    TEST_ASSERT_FALSE(is_uppercase('!'));
}

void test_is_lowercase(void) {
    TEST_ASSERT_TRUE(is_lowercase('a'));
    TEST_ASSERT_TRUE(is_lowercase('z'));
    TEST_ASSERT_TRUE(is_lowercase('m'));
    TEST_ASSERT_FALSE(is_lowercase('A'));
    TEST_ASSERT_FALSE(is_lowercase('Z'));
    TEST_ASSERT_FALSE(is_lowercase('0'));
    TEST_ASSERT_FALSE(is_lowercase(' '));
    TEST_ASSERT_FALSE(is_lowercase('!'));
}

void test_is_alphanumeric(void) {
    // Uppercase letters
    TEST_ASSERT_TRUE(is_alphanumeric('A'));
    TEST_ASSERT_TRUE(is_alphanumeric('Z'));
    // Lowercase letters
    TEST_ASSERT_TRUE(is_alphanumeric('a'));
    TEST_ASSERT_TRUE(is_alphanumeric('z'));
    // Numbers
    TEST_ASSERT_TRUE(is_alphanumeric('0'));
    TEST_ASSERT_TRUE(is_alphanumeric('9'));
    // Non-alphanumeric
    TEST_ASSERT_FALSE(is_alphanumeric(' '));
    TEST_ASSERT_FALSE(is_alphanumeric('!'));
    TEST_ASSERT_FALSE(is_alphanumeric('@'));
    TEST_ASSERT_FALSE(is_alphanumeric('#'));
    TEST_ASSERT_FALSE(is_alphanumeric('_'));
    TEST_ASSERT_FALSE(is_alphanumeric('-'));
}

