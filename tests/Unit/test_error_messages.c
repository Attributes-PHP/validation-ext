#include "unity.h"
#include "test_helpers.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_error_messages.h"
#include <Zend/zend.h>
#include <Zend/zend_type_info.h>
#include <Zend/zend_types.h>
#include <Zend/zend_string.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Helpers to build av_field / av_property_info without a running Zend engine.
// ---------------------------------------------------------------------------

static av_field make_field(const char *name, zval *value)
{
    av_field field;
    field.parent = NULL;
    field.name = string_init_stub(name, strlen(name), 0, 0);
    field.value = value;
    return field;
}

// Builds an av_property_info carrying a single basic type mask. The real
// build_union_type_string() (now linked from av_error_messages.c) derives the
// {expected} string from this mask, so the placeholder tests exercise the real
// type-to-article logic instead of a stub.
static av_property_info make_prop_info(zend_property_info *prop, zend_type *type, uint32_t type_mask)
{
    memset(prop, 0, sizeof(*prop));
    memset(type, 0, sizeof(*type));
    type->type_mask = type_mask;
    type->ptr = NULL;
    prop->type = *type;
    av_property_info info;
    info.model = NULL;
    info.model_ce = NULL;
    info.property = prop;
    return info;
}

// ---------------------------------------------------------------------------
// setUp / tearDown
// ---------------------------------------------------------------------------

void setUp(void)
{
    av_string_init_Stub(string_init_stub);
    av_string_release_Stub(string_release_stub);
    av_emalloc_Stub(emalloc_stub);
    av_efree_Stub(efree_stub);
    av_string_alloc_Stub(string_alloc_stub);
    av_string_truncate_Stub(string_truncate_stub);
    av_string_concat3_Stub(concat3_stub);
    av_string_copy_Stub(string_copy_stub);
    av_long_to_str_Stub(long_to_str_stub);
    av_double_to_str_Stub(double_to_str_stub);
    av_memnstr_Stub(memnstr_stub);
}

void tearDown(void)
{}

// ---------------------------------------------------------------------------
// No placeholders
// ---------------------------------------------------------------------------

void test_no_placeholders_returns_copy_of_template(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("email", &value);

    const char *template = "A plain message without placeholders.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING(template, result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_empty_template_returns_empty_string(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("email", &value);

    zend_string *result = av_replace_placeholders("", 0, &field, NULL);
    TEST_ASSERT_EQUAL_STRING("", result->val);
    TEST_ASSERT_EQUAL(0, result->len);
    av_string_release(result);
    av_string_release(field.name);
}

void test_text_without_placeholder_braces_is_not_replaced(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("name", &value);

    const char *template = "field {field field field}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING(template, result->val);
    av_string_release(result);
    av_string_release(field.name);
}

// ---------------------------------------------------------------------------
// {field} placeholder
// ---------------------------------------------------------------------------

void test_field_placeholder_replaced_with_field_name(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("username", &value);

    const char *template = "The {field} field is required.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("The username field is required.", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_multiple_field_placeholders_all_replaced(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("name", &value);

    const char *template = "{field}, {field} and {field} again";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("name, name and name again", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_adjacent_field_placeholders(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("x", &value);

    const char *template = "{field}{field}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("xx", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

// ---------------------------------------------------------------------------
// {value} placeholder (delegates to av_value_to_string)
// ---------------------------------------------------------------------------

void test_value_placeholder_replaced_with_null_value(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("count", &value);

    const char *template = "The {field} got {value}.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("The count got null.", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_value_placeholder_replaced_with_long_value(void)
{
    zval value;
    ZVAL_LONG(&value, 42);
    av_field field = make_field("count", &value);

    const char *template = "{field} expected a number, got {value}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("count expected a number, got 42", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_value_placeholder_replaced_with_string_value(void)
{
    zval value;
    ZVAL_STR(&value, string_init_stub("hello", strlen("hello"), 0, 0));
    av_field field = make_field("title", &value);

    const char *template = "{field} must be a string, got {value}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("title must be a string, got 'hello'", result->val);
    av_string_release(result);
    av_string_release(field.name);
    av_string_release(value.value.str);
}

void test_value_placeholder_replaced_with_boolean_value(void)
{
    zval value;
    ZVAL_TRUE(&value);
    av_field field = make_field("active", &value);

    const char *template = "The {field} is {value}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("The active is true", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

// ---------------------------------------------------------------------------
// {expected} placeholder (delegates to the real build_union_type_string)
// ---------------------------------------------------------------------------

void test_expected_placeholder_replaced_with_type_string(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("age", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type, MAY_BE_LONG);

    const char *template = "The {field} must be {expected}.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("The age must be integer.", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_expected_placeholder_repeated(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("name", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type, MAY_BE_STRING);

    const char *template = "{field} must be {expected} or {expected}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("name must be string or string", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

// ---------------------------------------------------------------------------
// Mixed placeholders
// ---------------------------------------------------------------------------

void test_all_three_placeholders_replaced(void)
{
    zval value;
    ZVAL_LONG(&value, 7);
    av_field field = make_field("count", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type, MAY_BE_LONG);

    const char *template = "The {field} must be {expected}, got {value}.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("The count must be integer, got 7.", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_non_placeholder_braced_text_is_preserved(void)
{
    zval value;
    ZVAL_LONG(&value, 1);
    av_field field = make_field("price", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type, MAY_BE_DOUBLE);

    // "{fieldx}" is not a placeholder and must be left untouched.
    const char *template = "{field} {fieldx} must be {expected}; got {value}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("price {fieldx} must be float; got 1", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_standalone_token_followed_by_suffix_is_replaced(void)
{
    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("id", &value);

    // "{field}" is a standalone token immediately followed by an unrelated
    // "x" that must survive intact.
    const char *template = "pre{field}xpost";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, NULL);
    TEST_ASSERT_EQUAL_STRING("preidxpost", result->val);
    av_string_release(result);
    av_string_release(field.name);
}
