#include "unity.h"
#include "test_helpers.h"
#include "helpers/mock_av_wrappers.h"
#include "helpers/av_value_to_string.h"
#include <Zend/zend.h>
#include <Zend/zend_types.h>
#include <Zend/zend_string.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Mock callbacks for the wrappers used by av_value_to_string.
// They build real zend_string allocations through the malloc-based stub so the
// produced strings can be inspected and freed with av_string_release.
// ---------------------------------------------------------------------------

static zend_string* concat3_stub(const char *str1, size_t str1_len,
                                 const char *str2, size_t str2_len,
                                 const char *str3, size_t str3_len,
                                 int num_calls) {
    size_t total = str1_len + str2_len + str3_len;
    zend_string *s = string_init_stub("", total, 0, num_calls);
    char *p = s->val;
    memcpy(p, str1, str1_len); p += str1_len;
    memcpy(p, str2, str2_len); p += str2_len;
    memcpy(p, str3, str3_len); p += str3_len;
    s->val[total] = '\0';
    s->len = total;
    return s;
}

static zend_string* string_copy_stub(zend_string *s, int num_calls) {
    return string_init_stub(s->val, s->len, 0, num_calls);
}

static zend_string* long_to_str_stub(zend_long num, int num_calls) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), ZEND_LONG_FMT, num);
    return string_init_stub(buf, (size_t)len, 0, num_calls);
}

static zend_string* double_to_str_stub(double num, int num_calls) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%.14g", num);
    return string_init_stub(buf, (size_t)len, 0, num_calls);
}

// Test-controlled state for the instanceof / resource / __toString paths.
static bool g_instanceof_result;
static const char *g_resource_type_name;
static zend_result g_tostring_result;
static const char *g_tostring_value;

static bool instanceof_stub(const zend_class_entry *instance_ce,
                            const zend_class_entry *ce, int num_calls) {
    (void)instance_ce; (void)ce; (void)num_calls;
    return g_instanceof_result;
}

static bool is_stringable_stub(const zend_class_entry *instance_ce, int num_calls) {
    (void)instance_ce; (void)num_calls;
    return g_instanceof_result;
}

static const char* rsrc_type_stub(zend_resource *res, int num_calls) {
    (void)res; (void)num_calls;
    return g_resource_type_name;
}

static zend_result tostring_stub(zend_object *object, zval *retval, int num_calls) {
    (void)object; (void)num_calls;
    ZVAL_UNDEF(retval);
    if (g_tostring_result == SUCCESS && g_tostring_value != NULL) {
        ZVAL_STR(retval, string_init_stub(g_tostring_value, strlen(g_tostring_value), 0, num_calls));
    }
    return g_tostring_result;
}

static void zval_ptr_dtor_stub(zval *zval_ptr, int num_calls) {
    (void)num_calls;
    if (zval_ptr != NULL && Z_TYPE_P(zval_ptr) == IS_STRING && Z_STR_P(zval_ptr) != NULL) {
        string_release_stub(Z_STR_P(zval_ptr), num_calls);
        ZVAL_UNDEF(zval_ptr);
    }
}

// ---------------------------------------------------------------------------
// Helpers to build zvals of each type without a running Zend engine.
// ---------------------------------------------------------------------------

static zend_class_entry make_ce(const char *name) {
    zend_class_entry ce;
    memset(&ce, 0, sizeof(ce));
    ce.name = string_init_stub(name, strlen(name), 0, 0);
    return ce;
}

static zend_object make_object(zend_class_entry *ce) {
    zend_object obj;
    memset(&obj, 0, sizeof(obj));
    obj.ce = ce;
    return obj;
}

static zend_resource make_resource(int type) {
    zend_resource res;
    memset(&res, 0, sizeof(res));
    res.type = type;
    return res;
}

// ---------------------------------------------------------------------------
// setUp / tearDown
// ---------------------------------------------------------------------------

void setUp(void) {
    av_string_init_Stub(string_init_stub);
    av_string_release_Stub(string_release_stub);
    av_emalloc_Stub(emalloc_stub);
    av_efree_Stub(efree_stub);
    av_string_concat3_Stub(concat3_stub);
    av_string_copy_Stub(string_copy_stub);
    av_long_to_str_Stub(long_to_str_stub);
    av_double_to_str_Stub(double_to_str_stub);
    av_instanceof_function_Stub(instanceof_stub);
    av_is_stringable_Stub(is_stringable_stub);
    av_rsrc_list_get_rsrc_type_Stub(rsrc_type_stub);
    av_call_tostring_Stub(tostring_stub);
    av_zval_ptr_dtor_Stub(zval_ptr_dtor_stub);

    g_instanceof_result = false;
    g_resource_type_name = "stream";
    g_tostring_result = FAILURE;
    g_tostring_value = NULL;
}

void tearDown(void) {
}

// ---------------------------------------------------------------------------
// null
// ---------------------------------------------------------------------------

void test_null_value_to_string(void) {
    TEST_ASSERT_EQUAL_STRING("null", av_value_to_string(NULL)->val);
}

void test_explicit_null_zval(void) {
    zval zv; ZVAL_NULL(&zv);
    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("null", result->val);
    av_string_release(result);
}

// ---------------------------------------------------------------------------
// booleans
// ---------------------------------------------------------------------------

void test_true_to_string(void) {
    zval zv; ZVAL_TRUE(&zv);
    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("true", result->val);
    av_string_release(result);
}

void test_false_to_string(void) {
    zval zv; ZVAL_FALSE(&zv);
    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("false", result->val);
    av_string_release(result);
}

// ---------------------------------------------------------------------------
// long
// ---------------------------------------------------------------------------

TEST_CASE(0)
TEST_CASE(1)
TEST_CASE(-1)
TEST_CASE(42)
TEST_CASE(-42)
TEST_CASE(2147483647)
TEST_CASE(-2147483648)
void test_long_to_string(zend_long value) {
    zval zv; ZVAL_LONG(&zv, value);
    zend_string *result = av_value_to_string(&zv);
    char expected[32];
    snprintf(expected, sizeof(expected), ZEND_LONG_FMT, value);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(result);
}

// ---------------------------------------------------------------------------
// double
// ---------------------------------------------------------------------------

TEST_CASE(0.0)
TEST_CASE(1.5)
TEST_CASE(-3.14)
TEST_CASE(100.0)
void test_double_to_string(double value) {
    zval zv; ZVAL_DOUBLE(&zv, value);
    zend_string *result = av_value_to_string(&zv);
    char expected[64];
    snprintf(expected, sizeof(expected), "%.14g", value);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(result);
}

// ---------------------------------------------------------------------------
// string
// ---------------------------------------------------------------------------

TEST_CASE("")
TEST_CASE("hello")
TEST_CASE("The quick brown fox")
TEST_CASE("with'quotes")
void test_string_to_string(const char *value) {
    zval zv; ZVAL_STR(&zv, string_init_stub(value, strlen(value), 0, 0));
    zend_string *result = av_value_to_string(&zv);
    char expected[256];
    snprintf(expected, sizeof(expected), "'%s'", value);
    TEST_ASSERT_EQUAL_STRING(expected, result->val);
    av_string_release(zv.value.str);
    av_string_release(result);
}

// ---------------------------------------------------------------------------
// array
// ---------------------------------------------------------------------------

void test_array_to_string(void) {
    zval zv; ZVAL_ARR(&zv, NULL);
    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("array", result->val);
    av_string_release(result);
}

// ---------------------------------------------------------------------------
// object
// ---------------------------------------------------------------------------

void test_object_with_tostring_uses_result(void) {
    zend_class_entry ce = make_ce("MyStringable");
    zend_object obj = make_object(&ce);
    zval zv; ZVAL_OBJ(&zv, &obj);

    g_instanceof_result = true;
    g_tostring_result = SUCCESS;
    g_tostring_value = "stringified";

    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("'stringified'", result->val);
    av_string_release(result);
    av_string_release(ce.name);
}

void test_object_stringable_but_tostring_fails_falls_back_to_class_name(void) {
    zend_class_entry ce = make_ce("FailingStringable");
    zend_object obj = make_object(&ce);
    zval zv; ZVAL_OBJ(&zv, &obj);

    g_instanceof_result = true;
    g_tostring_result = FAILURE;

    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("FailingStringable", result->val);
    av_string_release(result);
    av_string_release(ce.name);
}

void test_object_not_stringable_uses_class_name(void) {
    zend_class_entry ce = make_ce("PlainObject");
    zend_object obj = make_object(&ce);
    zval zv; ZVAL_OBJ(&zv, &obj);

    g_instanceof_result = false;

    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("PlainObject", result->val);
    av_string_release(result);
    av_string_release(ce.name);
}

// ---------------------------------------------------------------------------
// resource
// ---------------------------------------------------------------------------

void test_resource_to_string(void) {
    zend_resource res = make_resource(1);
    zval zv; ZVAL_RES(&zv, &res);
    g_resource_type_name = "stream";

    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("stream", result->val);
    av_string_release(result);
}

void test_resource_with_unknown_type_falls_back(void) {
    zend_resource res = make_resource(999);
    zval zv; ZVAL_RES(&zv, &res);
    g_resource_type_name = NULL;

    zend_string *result = av_value_to_string(&zv);
    TEST_ASSERT_EQUAL_STRING("resource", result->val);
    av_string_release(result);
}
