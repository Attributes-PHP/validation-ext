#include "av_error_messages.h"
#include "av_wrappers.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"
#include "Zend/zend_compile.h"
#include "Zend/zend_enum.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_hash.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_operators.h"
#include "Zend/zend_string.h"
#include "Zend/zend_types.h"
#include <stddef.h>
#include <string.h>

const char *av_default_error_type_messages[] = {
    [AV_ERROR_REQUIRED] = "Field is required",
    [AV_ERROR_TYPE] = "Must be {expected}",
};

static const av_basic_type_mapping basic_type_mappings[] = {
    {MAY_BE_BOOL, "boolean", sizeof("boolean") - 1}, {MAY_BE_LONG, "integer", sizeof("integer") - 1}, {MAY_BE_DOUBLE, "float", sizeof("float") - 1}, {MAY_BE_STRING, "string", sizeof("string") - 1},
    {MAY_BE_ARRAY, "array", sizeof("array") - 1},    {MAY_BE_OBJECT, "object", sizeof("object") - 1}, {MAY_BE_NULL, "null", sizeof("null") - 1},
};

#define BASIC_TYPE_MAPPINGS_SIZE (sizeof(basic_type_mappings) / sizeof(basic_type_mappings[0]))

static zend_always_inline void add_field_error_to_array(zval *errors_array, const char *error_message, size_t length)
{
    zval error_msg;
    av_zval_stringl(&error_msg, error_message, length);
    av_hash_next_index_insert(Z_ARRVAL_P(errors_array), &error_msg);
}

static zend_always_inline void add_field_error(zval *errors, zend_string *field_name, const char *error_message, size_t length)
{
    ZEND_ASSERT(Z_TYPE_P(errors) == IS_ARRAY);

    zval *existing = av_hash_find(Z_ARRVAL_P(errors), field_name);
    ZEND_ASSERT(existing == NULL || Z_TYPE_P(existing) == IS_ARRAY);

    if (existing && Z_TYPE_P(existing) == IS_ARRAY) {
        add_field_error_to_array(existing, error_message, length);
    } else {
        zval error_array;
        ZVAL_ARR(&error_array, av_new_array(0));
        add_field_error_to_array(&error_array, error_message, length);
        av_hash_add(Z_ARRVAL_P(errors), field_name, &error_array);
    }
}

static zend_string *generate_type_name(const zend_type type)
{
    if (ZEND_TYPE_IS_INTERSECTION(type)) {
        return av_string_init("mixed", 5, 0);
    }

    if (ZEND_TYPE_HAS_NAME(type)) {
        return av_string_copy(ZEND_TYPE_NAME(type));
    }

    uint32_t type_mask = ZEND_TYPE_PURE_MASK(type);

    for (size_t i = 0; i < BASIC_TYPE_MAPPINGS_SIZE; i++) {
        if (type_mask == basic_type_mappings[i].mask) {
            return av_string_init(basic_type_mappings[i].name, basic_type_mappings[i].length, 0);
        }
    }

    return av_string_init("mixed", 5, 0);
}

/*
 * Resolves the class entry of a named type when it declares an enum; returns
 * NULL for anything else (plain classes, interfaces, unknown names).
 */
static zend_class_entry *resolve_enum_ce(const zend_type type)
{
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(type));

    zend_class_entry *ce = av_lookup_class_ex(ZEND_TYPE_NAME(type), NULL, ZEND_FETCH_CLASS_DEFAULT);
    if (ce == NULL || !(ce->ce_flags & ZEND_ACC_ENUM)) {
        return NULL;
    }

    return ce;
}

static zend_string *enum_case_label(zend_class_entry *ce, zend_object *case_obj)
{
    if (ce->enum_backing_type != IS_UNDEF) {
        return av_value_to_string(zend_enum_fetch_case_value(case_obj));
    }

    return av_value_to_string(zend_enum_fetch_case_name(case_obj));
}

static uint32_t count_basic_types(uint32_t pure_mask)
{
    uint32_t total = 0;
    for (size_t i = 0; i < BASIC_TYPE_MAPPINGS_SIZE; i++) {
        if (pure_mask & basic_type_mappings[i].mask)
            total += 1;
    }

    return total;
}

static zend_always_inline zend_string *build_union_only_basic_types(uint32_t pure_mask)
{
    size_t total = count_basic_types(pure_mask);
    ZEND_ASSERT(total >= 1);

    size_t max_string_size = 0;
    for (size_t i = 0; i < BASIC_TYPE_MAPPINGS_SIZE; i++) {
        if (pure_mask & basic_type_mappings[i].mask)
            max_string_size += basic_type_mappings[i].length;
    }

    // All types but the last two are followed by ", ", the last two by " or ".
    if (total >= 2)
        max_string_size += (total - 2) * (sizeof(", ") - 1) + (sizeof(" or ") - 1);

    zend_string *result = av_string_alloc(max_string_size, 0);
    char *output = ZSTR_VAL(result);
    size_t output_pos = 0;
    size_t remaining = total;

    for (size_t i = 0; i < BASIC_TYPE_MAPPINGS_SIZE; i++) {
        if (!(pure_mask & basic_type_mappings[i].mask))
            continue;

        if (output_pos > 0) {
            const char *separator = (remaining == 1) ? " or " : ", ";
            size_t separator_len = (remaining == 1) ? (sizeof(" or ") - 1) : (sizeof(", ") - 1);
            memcpy(output + output_pos, separator, separator_len);
            output_pos += separator_len;
        }

        memcpy(output + output_pos, basic_type_mappings[i].name, basic_type_mappings[i].length);
        output_pos += basic_type_mappings[i].length;
        remaining -= 1;
    }

    output[output_pos] = '\0';

    ZEND_ASSERT(output_pos == max_string_size);
    return av_string_truncate(result, output_pos, 0);
}

static zend_always_inline zend_string *build_single_type_string(zend_type property_type)
{
    const zend_type *type;
    ZEND_TYPE_FOREACH(property_type, type)
    {
        return generate_type_name(*type);
    }
    ZEND_TYPE_FOREACH_END();

    return NULL;
}

static zend_always_inline void append_union_type_part(zend_string **result, uint32_t index, uint32_t total, const char *name, size_t name_len)
{
    // index is the number of parts appended so far: the last part sits at index total - 1
    ZEND_ASSERT(index < total);

    if (*result == NULL) {
        *result = av_string_init(name, name_len, 0);
        return;
    }

    const char *separator = (index + 1 == total) ? " or " : ", ";
    size_t separator_len = (index + 1 == total) ? (sizeof(" or ") - 1) : (sizeof(", ") - 1);
    zend_string *temp = av_string_concat3(ZSTR_VAL(*result), ZSTR_LEN(*result), separator, separator_len, name, name_len);
    av_string_release(*result);
    *result = temp;
}

// Number of parts an enum type contributes to a union list: one per case.
static uint32_t count_enum_type_parts(zend_class_entry *ce)
{
    ZEND_ASSERT(ce != NULL);

    if (ce->type == ZEND_USER_CLASS && !(ce->ce_flags & ZEND_ACC_CONSTANTS_UPDATED)) {
        av_update_class_constants(ce);
    }

    uint32_t total = 0;
    zend_class_constant *c;

    ZEND_HASH_MAP_FOREACH_PTR(&ce->constants_table, c)
    {
        if ((ZEND_CLASS_CONST_FLAGS(c) & ZEND_CLASS_CONST_IS_CASE) == 0) {
            continue;
        }
        total += 1;
    }
    ZEND_HASH_FOREACH_END();

    return total > 0 ? total : 1;
}

/*
 * Appends the cases of an enum type to a union list, one part per case,
 * using the comma/"or" grammar based on index/total.
 * Returns the number of parts appended (0 when no case could be evaluated).
 */
static uint32_t append_enum_type_parts(zend_string **result, uint32_t index, uint32_t total, zend_class_entry *ce)
{
    ZEND_ASSERT(ce != NULL);

    if (ce->type == ZEND_USER_CLASS && !(ce->ce_flags & ZEND_ACC_CONSTANTS_UPDATED)) {
        av_update_class_constants(ce);
    }

    uint32_t appended = 0;
    zend_class_constant *c;

    ZEND_HASH_MAP_FOREACH_PTR(&ce->constants_table, c)
    {
        if ((ZEND_CLASS_CONST_FLAGS(c) & ZEND_CLASS_CONST_IS_CASE) == 0) {
            continue;
        }

        zval *case_zv = &c->value;
        if (Z_TYPE_P(case_zv) == IS_CONSTANT_AST) {
            if (av_zval_update_constant_ex(case_zv, c->ce) == FAILURE) {
                break;
            }
        }

        zend_string *label = enum_case_label(ce, Z_OBJ_P(case_zv));
        append_union_type_part(result, index + appended, total, ZSTR_VAL(label), ZSTR_LEN(label));
        av_string_release(label);
        appended += 1;
    }
    ZEND_HASH_FOREACH_END();

    return appended;
}

zend_string *build_union_type_string(zend_type property_type)
{
    uint32_t pure_mask = ZEND_TYPE_PURE_MASK(property_type);
    uint32_t basic_total = count_basic_types(pure_mask);

    // Single classification pass over the named types: detects enums and
    // classes and counts the parts each named type contributes to a union
    // list (classes one part, enums one per case).
    bool has_class = false;
    bool has_enum = false;
    uint32_t named_total = 0;
    const zend_type *type;
    ZEND_TYPE_FOREACH(property_type, type)
    {
        if (!ZEND_TYPE_HAS_NAME(*type) || ZEND_TYPE_IS_INTERSECTION(*type))
            continue;

        zend_class_entry *ce = resolve_enum_ce(*type);
        if (ce != NULL) {
            has_enum = true;
            named_total += count_enum_type_parts(ce);
        } else {
            has_class = true;
            named_total += 1;
        }
    }
    ZEND_TYPE_FOREACH_END();

    // Basic types only
    if (!has_class && !has_enum) {
        if (basic_total <= 1)
            return build_single_type_string(property_type);

        return build_union_only_basic_types(pure_mask);
    }

    // Class types only: class names or-joined
    if (basic_total == 0 && !has_enum) {
        zend_string *result = NULL;

        ZEND_TYPE_FOREACH(property_type, type)
        {
            if (!ZEND_TYPE_HAS_NAME(*type) || ZEND_TYPE_IS_INTERSECTION(*type))
                continue;

            zend_string *type_part = generate_type_name(*type);

            if (!result) {
                result = type_part;
            } else {
                zend_string *prefix = av_string_init(" or ", sizeof(" or ") - 1, 0);
                zend_string *temp = av_string_concat3(ZSTR_VAL(result), ZSTR_LEN(result), ZSTR_VAL(prefix), ZSTR_LEN(prefix), ZSTR_VAL(type_part), ZSTR_LEN(type_part));
                av_string_release(result);
                av_string_release(prefix);
                av_string_release(type_part);
                result = temp;
            }
        }
        ZEND_TYPE_FOREACH_END();

        return result;
    }

    // Mixed basic, class and enum types (also enum-only unions): basic names
    // first, then named types in declaration order, joined like a union list
    // (commas, final " or "). Basic types and classes contribute one part
    // each, enums one part per case.
    uint32_t total = basic_total + named_total;
    zend_string *result = NULL;
    uint32_t index = 0;

    for (size_t i = 0; i < BASIC_TYPE_MAPPINGS_SIZE; i++) {
        if (!(pure_mask & basic_type_mappings[i].mask))
            continue;

        append_union_type_part(&result, index, total, basic_type_mappings[i].name, basic_type_mappings[i].length);
        index += 1;
    }

    ZEND_TYPE_FOREACH(property_type, type)
    {
        if (!ZEND_TYPE_HAS_NAME(*type) || ZEND_TYPE_IS_INTERSECTION(*type))
            continue;

        zend_class_entry *ce = resolve_enum_ce(*type);
        if (ce != NULL) {
            uint32_t appended = append_enum_type_parts(&result, index, total, ce);
            if (appended == 0) {
                zend_string *class_name = ZEND_TYPE_NAME(*type);
                append_union_type_part(&result, index, total, ZSTR_VAL(class_name), ZSTR_LEN(class_name));
                appended = 1;
            }
            index += appended;
            continue;
        }

        zend_string *class_name = ZEND_TYPE_NAME(*type);
        append_union_type_part(&result, index, total, ZSTR_VAL(class_name), ZSTR_LEN(class_name));
        index += 1;
    }
    ZEND_TYPE_FOREACH_END();

    return result;
}

/*
 * Retrieves the ErrorMessage attribute associated with the given property.
 *
 * Attribute usage example: #[ErrorMessage(required: "{field} is missing", type: "Ups wrong type {expected}")]
 */
static zend_attribute *get_error_message_attribute(av_property_info *property)
{
    if (property == NULL || property->property == NULL || property->property->attributes == NULL) {
        return NULL;
    }

    return av_get_attribute_str(property->property->attributes, "attributes\\validation\\fields\\errormessage", sizeof("attributes\\validation\\fields\\errormessage") - 1);
}

static zend_always_inline bool attribute_argument_name_equals(const zend_string *name, const char *expected, size_t length)
{
    return av_binary_strcasecmp(ZSTR_VAL(name), ZSTR_LEN(name), expected, length) == 0;
}

/*
 * Returns the error message template for the given error type: the custom
 * template declared with the #[ErrorMessage] attribute when the property
 * carries one, the default template otherwise.
 *
 * Named attribute arguments are matched by name (case-insensitively),
 * positional ones by constructor order (required first, then type).
 *
 * The result is a freshly allocated zend_string the caller must release.
 * Returns NULL when the error type is unsupported or the attribute value
 * cannot be evaluated; an exception is thrown in both cases.
 */
static zend_string *get_custom_error_template(av_error_type type, av_property_info *property)
{
    ZEND_ASSERT(AV_ERROR_TYPE == type || AV_ERROR_REQUIRED == type);

    zend_attribute *attribute = get_error_message_attribute(property);
    if (attribute == NULL) {
        const char *default_template = av_default_error_type_messages[type];
        return av_string_init(default_template, strlen(default_template), 0);
    }

    const char *argument_name;
    size_t argument_length;
    uint32_t argument_position;

    switch (type) {
        case AV_ERROR_REQUIRED:
            argument_name = "required";
            argument_position = 0;
            argument_length = sizeof("required") - 1;
            break;
        case AV_ERROR_TYPE:
            argument_name = "type";
            argument_position = 1;
            argument_length = sizeof("type") - 1;
            break;
        default:
            av_throw_value_error("Unsupported error type");
            return NULL;
    }

    uint32_t position = 0;

    for (uint32_t i = 0; i < attribute->argc; i++) {
        zend_attribute_arg *argument = &attribute->args[i];

        if (argument->name == NULL) {
            if (position != argument_position) {
                position += 1;
                continue;
            }
        } else if (!attribute_argument_name_equals(argument->name, argument_name, argument_length)) {
            continue;
        }

        zval value;
        if (av_get_attribute_value(&value, attribute, i, property->model_ce) != SUCCESS) {
            return NULL;
        }

        if (Z_TYPE_P(&value) == IS_STRING) {
            zend_string *template = av_string_copy(Z_STR_P(&value));
            av_zval_ptr_dtor(&value);
            return template;
        }

        if (Z_TYPE_P(&value) != IS_UNDEF) {
            av_throw_value_error("Only string arguments are valid for Attributes\\Validation\\Fields\\ErrorMessage::construct(...)");
            return NULL;
        }

        av_zval_ptr_dtor(&value);
        break;
    }

    const char *default_template = av_default_error_type_messages[type];
    return av_string_init(default_template, strlen(default_template), 0);
}

/**
 * Returns a dot concatenation string like: firstParentProperty.secondProperty.lastProperty
 */
static zend_string *get_property_full_path(av_field *field)
{
    if (!field->parent || ZSTR_LEN(field->parent) == 0) {
        return av_string_copy(field->name);
    }

    return av_string_concat3(ZSTR_VAL(field->parent), ZSTR_LEN(field->parent), ".", 1, ZSTR_VAL(field->name), ZSTR_LEN(field->name));
}

void av_add_field_error_with_prefix(av_error_type type, av_field *field, av_property_info *property, zval *errors)
{
    zend_string *template = get_custom_error_template(type, property);
    if (template == NULL) {
        return;
    }

    zend_string *replaced_message = av_replace_placeholders(ZSTR_VAL(template), ZSTR_LEN(template), field, property);
    av_string_release(template);
    if (replaced_message == NULL) {
        return;
    }

    zend_string *full_path = get_property_full_path(field);
    add_field_error(errors, full_path, ZSTR_VAL(replaced_message), ZSTR_LEN(replaced_message));
    av_string_release(full_path);
    av_string_release(replaced_message);
}

/*
 * Converts any PHP zval into a zend_string suitable for inclusion in an
 * error message template (the {value} placeholder).
 *
 * The conversion is fully type-aware so error messages stay readable:
 *   null     -> "null"
 *   bool     -> "true" / "false"
 *   int      -> numeric string
 *   double   -> numeric string
 *   string   -> single-quoted value
 *   array    -> "array"
 *   object   -> __toString() result when Stringable, else the class name
 *   resource -> resource type name, or "resource"
 *
 * All Zend internals are reached through the mockable av_wrappers so the
 * function can be unit tested in isolation.
 */
zend_string *av_value_to_string(zval *value)
{
    if (value == NULL) {
        return av_string_init("null", sizeof("null") - 1, 0);
    }

    ZVAL_DEREF(value);

    switch (Z_TYPE_P(value)) {
        case IS_NULL:
            return av_string_init("null", sizeof("null") - 1, 0);
        case IS_TRUE:
            return av_string_init("true", sizeof("true") - 1, 0);
        case IS_FALSE:
            return av_string_init("false", sizeof("false") - 1, 0);
        case IS_LONG:
            return av_long_to_str(Z_LVAL_P(value));
        case IS_DOUBLE:
            return av_double_to_str(Z_DVAL_P(value));
        case IS_STRING:
            return av_string_concat3("'", 1, Z_STRVAL_P(value), Z_STRLEN_P(value), "'", 1);
        case IS_ARRAY:
            return av_string_init("array", sizeof("array") - 1, 0);
        case IS_OBJECT:
            if (av_is_stringable(Z_OBJCE_P(value))) {
                zval result;
                if (av_call_tostring(Z_OBJ_P(value), &result) == SUCCESS) {
                    zend_string *str = av_string_concat3("'", 1, Z_STRVAL(result), Z_STRLEN(result), "'", 1);
                    av_zval_ptr_dtor(&result);
                    return str;
                }
            }
            return av_string_copy(Z_OBJCE_P(value)->name);
        case IS_RESOURCE: {
            const char *type_name = av_rsrc_list_get_rsrc_type(Z_RES_P(value));
            const char *fallback = "resource";
            return av_string_init(type_name ? type_name : fallback, strlen(type_name ? type_name : fallback), 0);
        }
        default:
            return av_string_init("(unknown)", sizeof("(unknown)") - 1, 0);
    }
}

/*
 * Substitutes the {field}, {value} and {expected} placeholders of an error
 * message template:
 *
 *   {field}    -> field->name
 *   {value}    -> av_value_to_string(field->value)
 *   {expected} -> build_union_type_string(prop_info->property->type)
 *
 * Each placeholder may occur multiple times. The result is a freshly
 * allocated zend_string that the caller must release with av_string_release.
 *
 * All Zend internals are reached through the mockable av_wrappers so the
 * function can be unit tested in isolation.
 */
zend_string *av_replace_placeholders(const char *template, size_t length, av_field *field, av_property_info *prop_info)
{
    struct {
        const char *search;
        size_t len;
        size_t counts;
        zend_string *replace;
    } table[] = {{"{field}", sizeof("{field}") - 1, 0, field->name}, {"{value}", sizeof("{value}") - 1, 0, NULL}, {"{expected}", sizeof("{expected}") - 1, 0, NULL}};
    size_t table_size = sizeof(table) / sizeof(table[0]);

    size_t max_template_size = length;
    size_t total_placeholders = 0;
    for (size_t i = 0; i < table_size; i++) {
        const char *search_pos = template;
        while ((search_pos = av_memnstr(search_pos, table[i].search, table[i].len, template + length))) {
            table[i].counts += 1;
            search_pos += table[i].len;
        }

        if (table[i].counts == 0)
            continue;

        total_placeholders += table[i].counts;

        if (table[i].replace == NULL) {
            if (i == 1) { // {value}
                table[i].replace = av_value_to_string(field->value);
            } else if (i == 2) { // {expected}
                ZEND_ASSERT(prop_info != NULL && prop_info->property != NULL);
                table[i].replace = build_union_type_string(prop_info->property->type);
            }
        }
        ZEND_ASSERT(table[i].replace != NULL);
        max_template_size += table[i].counts * (ZSTR_LEN(table[i].replace) - table[i].len);
    }

    if (total_placeholders == 0) {
        return av_string_init(template, length, 0);
    }

    // Allocate and process in a single pass
    zend_string *result = av_string_alloc(max_template_size, 0);
    char *output = ZSTR_VAL(result);
    size_t output_pos = 0;

    const char *input = template;
    const char *input_end = template + length;

    while (input < input_end) {
        if (total_placeholders == 0) {
            output[output_pos++] = *input++;
            continue;
        }

        bool replaced = false;
        for (size_t i = 0; i < table_size; i++) {
            if (table[i].counts == 0)
                continue;
            if (input + table[i].len > input_end)
                continue;
            if (memcmp(input, table[i].search, table[i].len) != 0)
                continue;

            table[i].counts -= 1;
            total_placeholders -= 1;

            memcpy(output + output_pos, ZSTR_VAL(table[i].replace), ZSTR_LEN(table[i].replace));
            output_pos += ZSTR_LEN(table[i].replace);
            input += table[i].len;
            replaced = true;
            break;
        }

        if (!replaced) {
            output[output_pos++] = *input++;
        }
    }

    ZEND_ASSERT(output_pos == max_template_size);

    // Null-terminate and truncate to actual size
    output[output_pos] = '\0';
    result = av_string_truncate(result, output_pos, 0);

    for (size_t i = 1; i < table_size; i++) {
        if (table[i].replace != NULL) {
            av_string_release(table[i].replace);
        }
    }

    return result;
}
