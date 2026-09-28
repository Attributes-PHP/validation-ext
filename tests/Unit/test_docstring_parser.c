#include "unity.h"
#include "test_helpers.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_docstring_parser.h"
#include <Zend/zend_type_info.h>
#include <Zend/zend_types.h>
#include <Zend/zend_string.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static zend_property_info make_property_with_doc(const char *doc_comment)
{
    zend_property_info property;
    memset(&property, 0, sizeof(property));
    property.doc_comment = string_init_stub(doc_comment, strlen(doc_comment), 0, 0);
    return property;
}

static void assert_single_basic_node(const av_docstring_type_node *node, uint32_t expected_mask)
{
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_NULL(node->next);
    TEST_ASSERT_FALSE(node->is_class);
    TEST_ASSERT_NULL(node->class_name);
    TEST_ASSERT_NULL(node->array_spec);
    TEST_ASSERT_EQUAL_UINT32(expected_mask, node->mask);
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
    av_string_concat3_Stub(concat3_stub);
    av_string_copy_Stub(string_copy_stub);
}

void tearDown(void)
{}

// ---------------------------------------------------------------------------
// av_docstring_parse_array_type
// ---------------------------------------------------------------------------

void test_parses_list_form_basic_type(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("array<string>", strlen("array<string>"));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_FALSE(spec->has_key_spec);
    TEST_ASSERT_NULL(spec->key_spec);
    assert_single_basic_node(spec->value_spec, MAY_BE_STRING);

    av_docstring_free_array_spec(spec);
}

void test_parses_dict_form_basic_types(void)
{
    const char *type = "array<int, string>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_TRUE(spec->has_key_spec);
    assert_single_basic_node(spec->key_spec, MAY_BE_LONG);
    assert_single_basic_node(spec->value_spec, MAY_BE_STRING);

    av_docstring_free_array_spec(spec);
}

void test_parses_union_value_arms(void)
{
    const char *type = "array<string|int>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_NOT_NULL(spec->value_spec);
    TEST_ASSERT_NOT_NULL(spec->value_spec->next);
    TEST_ASSERT_NULL(spec->value_spec->next->next);
    TEST_ASSERT_EQUAL_UINT32(MAY_BE_STRING, spec->value_spec->mask);
    TEST_ASSERT_EQUAL_UINT32(MAY_BE_LONG, spec->value_spec->next->mask);

    av_docstring_free_array_spec(spec);
}

void test_parses_class_value_type(void)
{
    const char *type = "array<Address>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_FALSE(spec->has_key_spec);
    TEST_ASSERT_NOT_NULL(spec->value_spec);
    TEST_ASSERT_TRUE(spec->value_spec->is_class);
    TEST_ASSERT_EQUAL_STRING("Address", ZSTR_VAL(spec->value_spec->class_name));
    TEST_ASSERT_NULL(spec->value_spec->array_spec);

    av_docstring_free_array_spec(spec);
}

void test_parses_nullable_class_with_null_arm(void)
{
    const char *type = "array<?User>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_NOT_NULL(spec->value_spec);
    TEST_ASSERT_TRUE(spec->value_spec->is_class);
    TEST_ASSERT_EQUAL_STRING("User", ZSTR_VAL(spec->value_spec->class_name));
    TEST_ASSERT_NOT_NULL(spec->value_spec->next);
    TEST_ASSERT_EQUAL_UINT32(MAY_BE_NULL, spec->value_spec->next->mask);

    av_docstring_free_array_spec(spec);
}

void test_parses_legacy_bracket_form(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("string[]", strlen("string[]"));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_FALSE(spec->has_key_spec);
    assert_single_basic_node(spec->value_spec, MAY_BE_STRING);

    av_docstring_free_array_spec(spec);
}

void test_parses_list_keyword_form(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("list<float>", strlen("list<float>"));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_FALSE(spec->has_key_spec);
    assert_single_basic_node(spec->value_spec, MAY_BE_DOUBLE);

    av_docstring_free_array_spec(spec);
}

void test_parses_nested_array_shape(void)
{
    const char *type = "array<array<int>>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_NOT_NULL(spec->value_spec);
    TEST_ASSERT_NOT_NULL(spec->value_spec->array_spec);
    TEST_ASSERT_FALSE(spec->value_spec->array_spec->has_key_spec);
    assert_single_basic_node(spec->value_spec->array_spec->value_spec, MAY_BE_LONG);

    av_docstring_free_array_spec(spec);
}

void test_parses_case_insensitively(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("Array<STRING>", strlen("Array<STRING>"));

    TEST_ASSERT_NOT_NULL(spec);
    assert_single_basic_node(spec->value_spec, MAY_BE_STRING);

    av_docstring_free_array_spec(spec);
}

void test_returns_null_for_plain_array(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("array", strlen("array"));
    TEST_ASSERT_NULL(spec);
}

void test_returns_null_for_non_array_type(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("int", strlen("int"));
    TEST_ASSERT_NULL(spec);
}

void test_returns_null_for_empty_type(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("  ", 2);
    TEST_ASSERT_NULL(spec);
}

void test_returns_null_for_unbalanced_brackets(void)
{
    const char *type = "array<int, string";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));
    TEST_ASSERT_NULL(spec);
}

void test_returns_null_for_invalid_key_type(void)
{
    const char *type = "array<Address, string>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));
    TEST_ASSERT_NULL(spec);
}

void test_returns_null_for_trailing_garbage_after_shape(void)
{
    const char *type = "array<int> nonsense";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));
    TEST_ASSERT_NULL(spec);
}

// ---------------------------------------------------------------------------
// av_docstring_parse_array_spec (doc comment entry point)
// ---------------------------------------------------------------------------

void test_parses_shape_from_property_doc_comment(void)
{
    zend_property_info property = make_property_with_doc("/** @var array<string, DateTime> */");
    av_array_spec *spec = av_docstring_parse_array_spec(&property);

    TEST_ASSERT_NOT_NULL(spec);
    TEST_ASSERT_TRUE(spec->has_key_spec);
    assert_single_basic_node(spec->key_spec, MAY_BE_STRING);
    TEST_ASSERT_NOT_NULL(spec->value_spec);
    TEST_ASSERT_TRUE(spec->value_spec->is_class);
    TEST_ASSERT_EQUAL_STRING("DateTime", ZSTR_VAL(spec->value_spec->class_name));

    av_docstring_free_array_spec(spec);
    string_release_stub(property.doc_comment, 0);
}

void test_strips_doc_comment_close_decoration(void)
{
    zend_property_info property = make_property_with_doc("/** Some prose. @var array<int> */");
    av_array_spec *spec = av_docstring_parse_array_spec(&property);

    TEST_ASSERT_NOT_NULL(spec);
    assert_single_basic_node(spec->value_spec, MAY_BE_LONG);

    av_docstring_free_array_spec(spec);
    string_release_stub(property.doc_comment, 0);
}

void test_returns_null_without_var_tag(void)
{
    zend_property_info property = make_property_with_doc("/** Just prose. */");
    av_array_spec *spec = av_docstring_parse_array_spec(&property);
    TEST_ASSERT_NULL(spec);

    string_release_stub(property.doc_comment, 0);
}

void test_returns_null_without_doc_comment(void)
{
    zend_property_info property;
    memset(&property, 0, sizeof(property));
    TEST_ASSERT_NULL(av_docstring_parse_array_spec(&property));
    TEST_ASSERT_NULL(av_docstring_parse_array_spec(NULL));
}

// ---------------------------------------------------------------------------
// av_docstring_type_list_to_string
// ---------------------------------------------------------------------------

void test_renders_single_basic_type(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("array<string>", strlen("array<string>"));
    TEST_ASSERT_NOT_NULL(spec);

    zend_string *rendered = av_docstring_type_list_to_string(spec->value_spec);
    TEST_ASSERT_EQUAL_STRING("string", ZSTR_VAL(rendered));

    av_string_release(rendered);
    av_docstring_free_array_spec(spec);
}

void test_renders_union_types_with_or_grammar(void)
{
    const char *type = "array<string|int>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));
    TEST_ASSERT_NOT_NULL(spec);

    zend_string *rendered = av_docstring_type_list_to_string(spec->value_spec);
    TEST_ASSERT_EQUAL_STRING("string or integer", ZSTR_VAL(rendered));

    av_string_release(rendered);
    av_docstring_free_array_spec(spec);
}

void test_renders_class_name(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("array<Address>", strlen("array<Address>"));
    TEST_ASSERT_NOT_NULL(spec);

    zend_string *rendered = av_docstring_type_list_to_string(spec->value_spec);
    TEST_ASSERT_EQUAL_STRING("Address", ZSTR_VAL(rendered));

    av_string_release(rendered);
    av_docstring_free_array_spec(spec);
}

void test_renders_nullable_class_with_null_arm(void)
{
    av_array_spec *spec = av_docstring_parse_array_type("array<?User>", strlen("array<?User>"));
    TEST_ASSERT_NOT_NULL(spec);

    zend_string *rendered = av_docstring_type_list_to_string(spec->value_spec);
    TEST_ASSERT_EQUAL_STRING("User or null", ZSTR_VAL(rendered));

    av_string_release(rendered);
    av_docstring_free_array_spec(spec);
}

void test_renders_nested_array_shape(void)
{
    const char *type = "array<array<int>>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));
    TEST_ASSERT_NOT_NULL(spec);

    zend_string *rendered = av_docstring_type_list_to_string(spec->value_spec);
    TEST_ASSERT_EQUAL_STRING("array<integer>", ZSTR_VAL(rendered));

    av_string_release(rendered);
    av_docstring_free_array_spec(spec);
}

void test_renders_dict_shape_with_key(void)
{
    const char *type = "array<array<int, string>>";
    av_array_spec *spec = av_docstring_parse_array_type(type, strlen(type));
    TEST_ASSERT_NOT_NULL(spec);

    // The outer value is a nested array shape: rendering it goes through
    // the dict grammar "array<KEY, VALUE>"
    zend_string *rendered = av_docstring_type_list_to_string(spec->value_spec);
    TEST_ASSERT_EQUAL_STRING("array<integer, string>", ZSTR_VAL(rendered));

    av_string_release(rendered);
    av_docstring_free_array_spec(spec);
}

void test_renders_mixed_for_null_list(void)
{
    zend_string *rendered = av_docstring_type_list_to_string(NULL);
    TEST_ASSERT_EQUAL_STRING("mixed", ZSTR_VAL(rendered));
    av_string_release(rendered);
}
