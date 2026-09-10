#include "unity.h"
#include "test_helpers.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_replace_placeholders.h"
#include "helpers/av_value_to_string.h"
#include <Zend/zend.h>
#include <Zend/zend_types.h>
#include <Zend/zend_string.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Mock callbacks for the wrappers used by av_replace_placeholders (and the
// real av_value_to_string it delegates {value} to). They build real zend_string
// allocations through the malloc-based stub so the produced strings can be
// inspected and freed with av_string_release.
// ---------------------------------------------------------------------------

static zend_string *concat3_stub(const char *str1, size_t str1_len, const char *str2, size_t str2_len, const char *str3, size_t str3_len, int num_calls)
{
    size_t total = str1_len + str2_len + str3_len;
    zend_string *s = string_alloc_stub(total, 0, num_calls);
    char *p = s->val;
    memcpy(p, str1, str1_len);
    p += str1_len;
    memcpy(p, str2, str2_len);
    p += str2_len;
    memcpy(p, str3, str3_len);
    p += str3_len;
    s->val[total] = '\0';
    s->len = total;
    return s;
}

static zend_string *string_copy_stub(zend_string *s, int num_calls)
{
    return string_init_stub(s->val, s->len, 0, num_calls);
}

// Manual integer-to-string conversion: avoids snprintf(), which the PHP
// headers (pulled in transitively via av_structs.h -> php.h) redefine to
// ap_php_snprintf, an unresolved symbol in the Ceedling unit-test build.
static zend_string *long_to_str_stub(zend_long num, int num_calls)
{
    char buf[32];
    size_t len = 0;

    if (num == 0) {
        buf[len++] = '0';
    } else {
        zend_long n = num;
        if (n < 0) {
            buf[len++] = '-';
            n = -n;
        }
        char tmp[32];
        size_t tmp_len = 0;
        while (n > 0) {
            tmp[tmp_len++] = (char)('0' + (n % 10));
            n /= 10;
        }
        while (tmp_len > 0)
            buf[len++] = tmp[--tmp_len];
    }

    return string_init_stub(buf, len, 0, num_calls);
}

// Doubles are not exercised by these tests; provide a minimal stub.
static zend_string *double_to_str_stub(double num, int num_calls)
{
    (void)num;
    return string_init_stub("0", 1, 0, num_calls);
}

// ---------------------------------------------------------------------------
// Hand-written stub for build_union_type_string (the {expected} provider).
// The real implementation lives in av_error_messages.c, which pulls in the full
// Zend engine and is therefore not linked into this unit test. CMock cannot
// mock it in isolation because it shares its header with the function under
// test, so we provide a minimal definition returning a configurable string.
// ---------------------------------------------------------------------------

static const char *g_expected_type_string;

zend_string *build_union_type_string(zend_type property_type)
{
    (void)property_type;
    return string_init_stub(g_expected_type_string, strlen(g_expected_type_string), 0, 0);
}

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

static av_property_info make_prop_info(zend_property_info *prop, zend_type *type)
{
    memset(prop, 0, sizeof(*prop));
    memset(type, 0, sizeof(*type));
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

    g_expected_type_string = "an integer";
}

void tearDown(void) {}

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
// {expected} placeholder (delegates to build_union_type_string, stubbed here)
// ---------------------------------------------------------------------------

void test_expected_placeholder_replaced_with_type_string(void)
{
    g_expected_type_string = "an integer";

    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("age", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type);

    const char *template = "The {field} must be {expected}.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("The age must be an integer.", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_expected_placeholder_repeated(void)
{
    g_expected_type_string = "a string";

    zval value;
    ZVAL_NULL(&value);
    av_field field = make_field("name", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type);

    const char *template = "{field} must be {expected} or {expected}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("name must be a string or a string", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

// ---------------------------------------------------------------------------
// Mixed placeholders
// ---------------------------------------------------------------------------

void test_all_three_placeholders_replaced(void)
{
    g_expected_type_string = "an integer";

    zval value;
    ZVAL_LONG(&value, 7);
    av_field field = make_field("count", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type);

    const char *template = "The {field} must be {expected}, got {value}.";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("The count must be an integer, got 7.", result->val);
    av_string_release(result);
    av_string_release(field.name);
}

void test_non_placeholder_braced_text_is_preserved(void)
{
    g_expected_type_string = "a float";

    zval value;
    ZVAL_LONG(&value, 1);
    av_field field = make_field("price", &value);

    zend_property_info prop;
    zend_type type;
    av_property_info prop_info = make_prop_info(&prop, &type);

    // "{fieldx}" is not a placeholder and must be left untouched.
    const char *template = "{field} {fieldx} must be {expected}; got {value}";
    zend_string *result = av_replace_placeholders(template, strlen(template), &field, &prop_info);
    TEST_ASSERT_EQUAL_STRING("price {fieldx} must be a float; got 1", result->val);
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
