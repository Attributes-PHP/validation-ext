#include "av_docstring_parser.h"
#include "av_wrappers.h"
#include <string.h>

/*
 * Byte range into a doc comment or type expression. All scanning operates
 * on [start, end) pairs; no Zend string API is needed inside the
 * tokenizer itself.
 */
typedef struct {
    const char *start;
    const char *end;
} av_region;

static zend_always_inline av_region region_init(const char *start, const char *end)
{
    av_region region;
    region.start = start;
    region.end = end;
    return region;
}

static zend_always_inline bool region_is_empty(av_region region)
{
    return region.start >= region.end;
}

static zend_always_inline void region_trim(av_region *region)
{
    while (region->start < region->end && (*region->start == ' ' || *region->start == '\t')) {
        region->start++;
    }
    while (region->end > region->start && (region->end[-1] == ' ' || region->end[-1] == '\t')) {
        region->end--;
    }
}

static zend_always_inline char ascii_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c + ('a' - 'A')) : c;
}

/* Case-insensitive comparison of a region against a lower-case literal. */
static bool region_equals_lower(av_region region, const char *literal, size_t literal_len)
{
    if ((size_t)(region.end - region.start) != literal_len) {
        return false;
    }

    for (size_t i = 0; i < literal_len; i++) {
        if (ascii_lower(region.start[i]) != literal[i]) {
            return false;
        }
    }

    return true;
}

/* Case-insensitive prefix check; outputs the remaining region on match. */
static bool region_starts_with_lower(av_region region, const char *prefix, size_t prefix_len, av_region *rest)
{
    if ((size_t)(region.end - region.start) < prefix_len) {
        return false;
    }

    for (size_t i = 0; i < prefix_len; i++) {
        if (ascii_lower(region.start[i]) != prefix[i]) {
            return false;
        }
    }

    *rest = region_init(region.start + prefix_len, region.end);
    return true;
}

/*
 * Maps a basic type name to its MAY_BE_* mask. Class/alias names are not
 * handled here: the caller treats them as class names.
 */
static bool basic_type_mask(av_region name, uint32_t *mask)
{
    static const struct {
        const char *name;
        size_t name_len;
        uint32_t mask;
    } table[] = {
        {"int", sizeof("int") - 1, MAY_BE_LONG},   {"integer", sizeof("integer") - 1, MAY_BE_LONG}, {"float", sizeof("float") - 1, MAY_BE_DOUBLE}, {"string", sizeof("string") - 1, MAY_BE_STRING},
        {"bool", sizeof("bool") - 1, MAY_BE_BOOL}, {"boolean", sizeof("boolean") - 1, MAY_BE_BOOL}, {"true", sizeof("true") - 1, MAY_BE_TRUE},     {"false", sizeof("false") - 1, MAY_BE_FALSE},
        {"null", sizeof("null") - 1, MAY_BE_NULL}, {"mixed", sizeof("mixed") - 1, MAY_BE_ANY},      {"array", sizeof("array") - 1, MAY_BE_ARRAY},  {"object", sizeof("object") - 1, MAY_BE_OBJECT},
    };
    const size_t table_size = sizeof(table) / sizeof(table[0]);

    for (size_t i = 0; i < table_size; i++) {
        if (region_equals_lower(name, table[i].name, table[i].name_len)) {
            *mask = table[i].mask;
            return true;
        }
    }

    return false;
}

static void free_type_list(av_docstring_type_node *head);

static void free_array_spec(av_array_spec *spec)
{
    if (spec == NULL) {
        return;
    }

    free_type_list(spec->key_spec);
    free_type_list(spec->value_spec);
    av_efree(spec);
}

static void free_type_list(av_docstring_type_node *head)
{
    while (head != NULL) {
        av_docstring_type_node *next = head->next;

        if (head->class_name != NULL) {
            av_string_release(head->class_name);
        }
        free_array_spec(head->array_spec);
        av_efree(head);

        head = next;
    }
}

/*
 * When region starts with "array<" or "list<", outputs the region inside
 * the angle brackets (honoring nesting) and the region after the closing
 * '>'. Returns false for anything else, including unbalanced brackets.
 */
static bool match_generic(av_region region, av_region *inner, av_region *rest)
{
    av_region after;

    if (!region_starts_with_lower(region, "array", sizeof("array") - 1, &after) && !region_starts_with_lower(region, "list", sizeof("list") - 1, &after)) {
        return false;
    }

    region_trim(&after);
    if (region_is_empty(after) || after.start[0] != '<') {
        return false;
    }

    uint32_t depth = 0;
    for (const char *p = after.start + 1; p < region.end; p++) {
        if (*p == '<') {
            depth++;
        } else if (*p == '>') {
            if (depth == 0) {
                *inner = region_init(after.start + 1, p);
                *rest = region_init(p + 1, region.end);
                return true;
            }
            depth--;
        }
    }

    return false;
}

/*
 * Finds the first delimiter at bracket depth 0 ('|' or ','). Returns the
 * position or NULL when the region contains none.
 */
static const char *find_top_level_delim(av_region region, char delim)
{
    uint32_t depth = 0;

    for (const char *p = region.start; p < region.end; p++) {
        if (*p == '<') {
            depth++;
        } else if (*p == '>') {
            depth--;
        } else if (*p == delim && depth == 0) {
            return p;
        }
    }

    return NULL;
}

static av_docstring_type_node *parse_type_list(av_region region);

/*
 * Parses the inside of a located generic region ("array<KEY, VALUE>" or
 * "array<VALUE>") into a spec.
 *
 * Key specs are restricted to int, string or a union of both; anything
 * else (classes, nested arrays, other basic types, unions) invalidates
 * the whole spec, which then degrades to plain `array` behavior.
 */
static av_array_spec *parse_spec_inner(av_region inner)
{
    region_trim(&inner);
    if (region_is_empty(inner)) {
        return NULL;
    }

    av_array_spec *spec = av_emalloc(sizeof(av_array_spec));
    memset(spec, 0, sizeof(av_array_spec));

    const char *comma = find_top_level_delim(inner, ',');
    if (comma != NULL) {
        spec->has_key_spec = true;
        spec->key_spec = parse_type_list(region_init(inner.start, comma));
        spec->value_spec = parse_type_list(region_init(comma + 1, inner.end));
    } else {
        spec->value_spec = parse_type_list(inner);
    }

    if (spec->value_spec == NULL) {
        free_array_spec(spec);
        return NULL;
    }

    if (spec->has_key_spec) {
        for (const av_docstring_type_node *node = spec->key_spec; node != NULL; node = node->next) {
            if (node->is_class || node->array_spec != NULL || (node->mask & ~(MAY_BE_LONG | MAY_BE_STRING)) != 0) {
                free_array_spec(spec);
                return NULL;
            }
        }
    }

    return spec;
}

/*
 * Parses one union arm into a chain of one or two nodes (two when the arm
 * is nullable: "?User" becomes a User node followed by a null arm).
 * Returns NULL on an empty or malformed arm.
 */
static av_docstring_type_node *parse_single_type(av_region arm)
{
    region_trim(&arm);
    if (region_is_empty(arm)) {
        return NULL;
    }

    bool nullable = false;
    if (arm.start[0] == '?') {
        nullable = true;
        arm.start++;
        region_trim(&arm);
        if (region_is_empty(arm)) {
            return NULL;
        }
    }

    av_docstring_type_node *node = av_emalloc(sizeof(av_docstring_type_node));
    memset(node, 0, sizeof(av_docstring_type_node));

    av_region inner;
    av_region rest;
    if (match_generic(arm, &inner, &rest)) {
        region_trim(&rest);
        if (!region_is_empty(rest)) {
            av_efree(node);
            return NULL;
        }
        node->array_spec = parse_spec_inner(inner);
        if (node->array_spec == NULL) {
            av_efree(node);
            return NULL;
        }
    } else if (arm.end - arm.start >= 2 && arm.end[-1] == ']' && arm.end[-2] == '[') {
        // Legacy "T[]" form: value spec from the base type, keys unchecked
        av_array_spec *spec = av_emalloc(sizeof(av_array_spec));
        memset(spec, 0, sizeof(av_array_spec));
        spec->value_spec = parse_type_list(region_init(arm.start, arm.end - 2));
        if (spec->value_spec == NULL) {
            av_efree(spec);
            av_efree(node);
            return NULL;
        }
        node->array_spec = spec;
    } else {
        uint32_t mask = 0;
        if (basic_type_mask(arm, &mask)) {
            node->mask = mask;
        } else {
            node->is_class = true;
            node->class_name = av_string_init(arm.start, arm.end - arm.start, 0);
        }
    }

    if (nullable) {
        av_docstring_type_node *null_node = av_emalloc(sizeof(av_docstring_type_node));
        memset(null_node, 0, sizeof(av_docstring_type_node));
        null_node->mask = MAY_BE_NULL;
        node->next = null_node;
    }

    return node;
}

/*
 * Parses a full type expression (one or more '|' separated arms) into a
 * linked list of nodes. Returns the head or NULL when nothing parsed.
 */
static av_docstring_type_node *parse_type_list(av_region region)
{
    av_docstring_type_node *head = NULL;
    av_docstring_type_node *tail = NULL;

    region_trim(&region);
    while (!region_is_empty(region)) {
        const char *pipe = find_top_level_delim(region, '|');
        av_docstring_type_node *node = parse_single_type(pipe != NULL ? region_init(region.start, pipe) : region);

        if (node == NULL) {
            free_type_list(head);
            return NULL;
        }

        if (head == NULL) {
            head = node;
        } else {
            tail->next = node;
        }
        for (tail = node; tail->next != NULL; tail = tail->next)
            ;

        if (pipe == NULL) {
            break;
        }
        region = region_init(pipe + 1, region.end);
        region_trim(&region);
    }

    return head;
}

/*
 * Finds the type expression of the first "@var" tag: the bytes between the
 * tag (plus whitespace) and the end of its line. Trailing whitespace is
 * trimmed. Only the first @var occurrence is considered.
 */
static bool find_var_type_region(const char *doc_start, const char *doc_end, const char **type_start, const char **type_end)
{
    for (const char *p = doc_start; p + 4 <= doc_end; p++) {
        if (p[0] != '@' || p[1] != 'v' || p[2] != 'a' || p[3] != 'r') {
            continue;
        }

        const char *start = p + 4;
        while (start < doc_end && (*start == ' ' || *start == '\t')) {
            start++;
        }

        const char *end = start;
        while (end < doc_end && *end != '\n' && *end != '\r') {
            end++;
        }
        // Strip trailing whitespace and the doc comment close decoration
        // ("*/", " *") so single-line comments do not leave garbage at the
        // end of the type expression
        while (end > start) {
            char last = end[-1];
            if (last == ' ' || last == '\t' || last == '*' || last == '/') {
                end--;
            } else {
                break;
            }
        }

        if (end > start) {
            *type_start = start;
            *type_end = end;
            return true;
        }

        return false;
    }

    return false;
}

av_array_spec *av_docstring_parse_array_type(const char *type_start, size_t type_len)
{
    av_region region = region_init(type_start, type_start + type_len);
    region_trim(&region);
    if (region_is_empty(region)) {
        return NULL;
    }

    av_region inner;
    av_region rest;
    if (match_generic(region, &inner, &rest)) {
        region_trim(&rest);
        if (!region_is_empty(rest)) {
            return NULL;
        }
        return parse_spec_inner(inner);
    }

    // Legacy "T[]" form
    if (region.end - region.start >= 2 && region.end[-1] == ']' && region.end[-2] == '[') {
        av_array_spec *spec = av_emalloc(sizeof(av_array_spec));
        memset(spec, 0, sizeof(av_array_spec));
        spec->value_spec = parse_type_list(region_init(region.start, region.end - 2));
        if (spec->value_spec == NULL) {
            free_array_spec(spec);
            return NULL;
        }
        return spec;
    }

    // Plain "array", "int", "User", ...: no shape to enforce
    return NULL;
}

av_array_spec *av_docstring_parse_array_spec(zend_property_info *property)
{
    if (property == NULL || property->doc_comment == NULL) {
        return NULL;
    }

    const char *start = ZSTR_VAL(property->doc_comment);
    const char *end = start + ZSTR_LEN(property->doc_comment);

    const char *type_start;
    const char *type_end;
    if (!find_var_type_region(start, end, &type_start, &type_end)) {
        return NULL;
    }

    return av_docstring_parse_array_type(type_start, type_end - type_start);
}

void av_docstring_free_array_spec(av_array_spec *spec)
{
    free_array_spec(spec);
}

static const char *basic_mask_name(uint32_t mask)
{
    switch (mask) {
        case MAY_BE_LONG:
            return "integer";
        case MAY_BE_DOUBLE:
            return "float";
        case MAY_BE_STRING:
            return "string";
        case MAY_BE_BOOL:
            return "boolean";
        case MAY_BE_TRUE:
            return "true";
        case MAY_BE_FALSE:
            return "false";
        case MAY_BE_NULL:
            return "null";
        case MAY_BE_ARRAY:
            return "array";
        case MAY_BE_OBJECT:
            return "object";
        default:
            return "mixed";
    }
}

/* Renders a single node ("int" -> "integer", "array<int>" -> "array<int>"). */
static zend_string *render_single_type(const av_docstring_type_node *node)
{
    if (node->array_spec != NULL) {
        av_array_spec *spec = node->array_spec;

        // "array<KEY, VALUE>" or "array<VALUE>"; an empty spec renders "array<>"
        zend_string *inner = NULL;
        if (spec->value_spec != NULL) {
            zend_string *value = av_docstring_type_list_to_string(spec->value_spec);
            if (spec->has_key_spec && spec->key_spec != NULL) {
                zend_string *key = av_docstring_type_list_to_string(spec->key_spec);
                zend_string *comma = av_string_init(", ", sizeof(", ") - 1, 0);
                inner = av_string_concat3(ZSTR_VAL(key), ZSTR_LEN(key), ZSTR_VAL(comma), ZSTR_LEN(comma), ZSTR_VAL(value), ZSTR_LEN(value));
                av_string_release(comma);
                av_string_release(key);
                av_string_release(value);
            } else {
                inner = value;
            }
        }
        if (inner == NULL) {
            inner = av_string_init("", 0, 0);
        }

        zend_string *prefix = av_string_init("array<", sizeof("array<") - 1, 0);
        zend_string *suffix = av_string_init(">", sizeof(">") - 1, 0);
        zend_string *rendered = av_string_concat3(ZSTR_VAL(prefix), ZSTR_LEN(prefix), ZSTR_VAL(inner), ZSTR_LEN(inner), ZSTR_VAL(suffix), ZSTR_LEN(suffix));
        av_string_release(prefix);
        av_string_release(suffix);
        av_string_release(inner);
        return rendered;
    }

    if (node->is_class) {
        return av_string_copy(node->class_name);
    }

    const char *name = basic_mask_name(node->mask);
    return av_string_init(name, strlen(name), 0);
}

zend_string *av_docstring_type_list_to_string(const av_docstring_type_node *node)
{
    uint32_t total = 0;
    for (const av_docstring_type_node *current = node; current != NULL; current = current->next) {
        total++;
    }

    if (total == 0) {
        const char *mixed = "mixed";
        return av_string_init(mixed, strlen(mixed), 0);
    }

    zend_string *result = NULL;
    uint32_t index = 0;
    for (const av_docstring_type_node *current = node; current != NULL; current = current->next, index++) {
        zend_string *part = render_single_type(current);

        if (result == NULL) {
            result = part;
            continue;
        }

        const char *separator = (index + 1 == total) ? " or " : ", ";
        size_t separator_len = (index + 1 == total) ? (sizeof(" or ") - 1) : (sizeof(", ") - 1);
        zend_string *joined = av_string_concat3(ZSTR_VAL(result), ZSTR_LEN(result), separator, separator_len, ZSTR_VAL(part), ZSTR_LEN(part));
        av_string_release(result);
        av_string_release(part);
        result = joined;
    }

    return result;
}
