#include "unity.h"
#include "test_structs.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_string.h"

void setUp(void) {
    // This is run before EACH test
}

void tearDown(void) {
    // This is run after EACH test
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
