#include "unity.h"
#include "test_helpers.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_string.h"

void setUp(void)
{
    // This is run before EACH test
    av_string_init_Stub(string_init_stub);
    av_string_release_Stub(string_release_stub);
    av_emalloc_Stub(emalloc_stub);
    av_efree_Stub(efree_stub);
    av_string_concat3_Stub(concat3_stub);
}

void tearDown(void)
{
    // This is run after EACH test
}

// ============= PascalCase Tests =============

TEST_CASE("first_name", "FirstName")
TEST_CASE("first-name", "FirstName")
TEST_CASE("first name", "FirstName")
TEST_CASE("firstName", "FirstName")
TEST_CASE("FirstName", "FirstName")
TEST_CASE("first_name_last-name", "FirstNameLastName")
TEST_CASE("name", "Name")
TEST_CASE("", "")
TEST_CASE("user2_name", "User2Name")
void test_to_pascal_case(const char *input, const char *expected)
{
    zend_string *input_str = av_string_init(input, strlen(input), 0);
    zend_string *result = av_to_pascal_case(input_str);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(input_str);
    av_string_release(result);
}

// ============= camelCase Tests =============

TEST_CASE("first_name", "firstName")
TEST_CASE("first-name", "firstName")
TEST_CASE("FirstName", "firstName")
TEST_CASE("firstName", "firstName")
TEST_CASE("Name", "name")
TEST_CASE("first_name_last-name", "firstNameLastName")
TEST_CASE("user2_name", "user2Name")
void test_to_camel_case(const char *input, const char *expected)
{
    zend_string *input_str = av_string_init(input, strlen(input), 0);
    zend_string *result = av_to_camel_case(input_str);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(input_str);
    av_string_release(result);
}

// ============= snake_case Tests =============

TEST_CASE("FirstName", "first_name")
TEST_CASE("firstName", "first_name")
TEST_CASE("first-name", "first_name")
TEST_CASE("first name", "first_name")
TEST_CASE("first_name", "first_name")
TEST_CASE("Name", "name")
TEST_CASE("HTTPResponse", "h_t_t_p_response")
TEST_CASE("user2Name", "user2_name")
void test_to_snake_case(const char *input, const char *expected)
{
    zend_string *input_str = av_string_init(input, strlen(input), 0);
    zend_string *result = av_to_snake_case(input_str);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(input_str);
    av_string_release(result);
}

// ============= kebab-case Tests =============

TEST_CASE("FirstName", "first-name")
TEST_CASE("firstName", "first-name")
TEST_CASE("first_name", "first-name")
TEST_CASE("first name", "first-name")
TEST_CASE("first-name", "first-name")
TEST_CASE("Name", "name")
TEST_CASE("user2Name", "user2-name")
void test_to_kebab_case(const char *input, const char *expected)
{
    zend_string *input_str = av_string_init(input, strlen(input), 0);
    zend_string *result = av_to_kebab_case(input_str);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(input_str);
    av_string_release(result);
}

// ============= Edge Cases =============

void test_no_input_modification(void)
{
    zend_string *input = av_string_init("first_name", sizeof("first_name") - 1, 0);
    const char *original_val = strdup(input->val);
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

    free((void *)original_val);
    av_string_release(input);
}

// ============= Static Helper Function Tests =============

TEST_MATRIX([ 'A', 'B', 'C', 'D', 'M', 'Z' ])
void test_is_uppercase(char letter)
{
    TEST_ASSERT_TRUE(is_uppercase(letter));
}

TEST_MATRIX([ 'a', 'b', 'c', 'd', '1', '2', '5', ' ', '!', '%' ])
void test_is_uppercase_not_valid(char letter)
{
    TEST_ASSERT_FALSE(is_uppercase(letter));
}

TEST_MATRIX([ 'a', 'z', 'm', 'k', 'l', 't' ])
void test_is_lowercase(char letter)
{
    TEST_ASSERT_TRUE(is_lowercase(letter));
}

TEST_MATRIX([ 'A', 'B', 'C', 'Z', 'T', ' ', '!', '%' ])
void test_is_lowercase_not_valid(char letter)
{
    TEST_ASSERT_FALSE(is_lowercase(letter));
}

TEST_MATRIX([ 'A', 'B', 'C', 'D', 'M', 'Z' ])
void test_is_alphanumeric_uppercase(char letter)
{
    TEST_ASSERT_TRUE(is_alphanumeric(letter));
}

TEST_MATRIX([ 'a', 'z', 'm', 'k', 'l', 't' ])
void test_is_alphanumeric_lowercase(char letter)
{
    TEST_ASSERT_TRUE(is_alphanumeric(letter));
}

TEST_MATRIX([ '1', '0', '2', '3', '4', '9', '8' ])
void test_is_alphanumeric_numbers(char letter)
{
    TEST_ASSERT_TRUE(is_alphanumeric(letter));
}

TEST_MATRIX([ ' ', '!', '+', '_', '-', '/', '@', '#' ])
void test_is_alphanumeric_non_alphanumeric(char letter)
{
    TEST_ASSERT_FALSE(is_alphanumeric(letter));
}

// ============= av_string_dot_concat Tests =============

TEST_CASE("first", "second", "first.second")
TEST_CASE("", "second", ".second")
TEST_CASE("first", "", "first.")
TEST_CASE("", "", ".")
TEST_CASE("123", "456", "123.456")
TEST_CASE("hello-world", "test_case", "hello-world.test_case")
TEST_CASE("a", "b", "a.b")
TEST_CASE("prefix", "suffix", "prefix.suffix")
void test_av_string_dot_concat(const char *first, const char *second, const char *expected)
{
    zend_string *first_str = av_string_init(first, strlen(first), 0);
    zend_string *second_str = av_string_init(second, strlen(second), 0);

    zend_string *result = av_string_dot_concat(first_str, second_str);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    TEST_ASSERT_EQUAL(strlen(expected), result->len);

    av_string_release(first_str);
    av_string_release(second_str);
    av_string_release(result);
}

TEST_CASE(NULL, "second")
TEST_CASE("first", NULL)
TEST_CASE(NULL, NULL)
void test_av_string_dot_concat_null_inputs(const char *first_str, const char *second_str)
{
    zend_string *first = NULL, *second = NULL, *result;

    if (first_str != NULL) {
        first = av_string_init(first_str, strlen(first_str), 0);
    }
    if (second_str != NULL) {
        second = av_string_init(second_str, strlen(second_str), 0);
    }

    result = av_string_dot_concat(first, second);
    TEST_ASSERT_NULL(result);

    av_string_release(first);
    av_string_release(second);
}
