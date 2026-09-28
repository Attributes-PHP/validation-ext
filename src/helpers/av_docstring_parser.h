/**
 * Docstring array type parsing
 *
 * PHP cannot express generics in native type hints, so array shapes are
 * declared through property doc comments:
 *
 *   @var array<string, DateTime>
 *   public array $birthdays;
 *
 *   @var array<Address>            // list form: keys are not checked
 *   public array $addresses;
 *
 *   @var array<int, string|int>     // union value types
 *   public array $scores;
 *
 *   @var array<array<int>>          // nested arrays
 *   public array $matrix;
 *
 * Also accepted: the "list<T>" and the legacy "T[]" forms.
 */

#ifndef AV_DOCSTRING_PARSER_H
#define AV_DOCSTRING_PARSER_H

#include "php.h"
#include "zend_type_info.h"

/**
 * A single type expression inside an array spec.
 *
 * A type is either:
 *  - a basic type described by a MAY_BE_* mask ("int", "string", ...),
 *  - a class/alias name ("Address", "DateTime", "self"),
 *  - or a nested array spec ("array<...>" or "list<...>").
 *
 * Union arms ("string|int") are kept as a linked list. Nullability
 * ("?User") is represented as an additional MAY_BE_NULL arm at the end
 * of the chain. Nodes are allocated with av_emalloc() and own their
 * zend_string members.
 *
 * ce_resolved/ce are validation-time state, not part of the parsed
 * shape: the validator memoizes the resolved class entry there so a
 * class arm is looked up once per spec lifetime (one validation call)
 * instead of once per array element. The parser zeroes both fields.
 */
typedef struct av_docstring_type_node {
    bool is_class;                       /* class/alias type vs basic mask */
    uint32_t mask;                       /* MAY_BE_LONG, MAY_BE_STRING, ... */
    zend_string *class_name;             /* owned; NULL unless is_class */
    struct av_docstring_type_node *next; /* union arm chain */
    struct av_array_spec *array_spec;    /* owned; non-NULL for nested arrays */
    bool ce_resolved;                    /* class entry already resolved */
    zend_class_entry *ce;                /* resolved class entry; not owned */
} av_docstring_type_node;

/**
 * Full "array<KEY, VALUE>" spec.
 *
 * has_key_spec is true for the two-parameter form "array<int, X>" (keys are
 * then validated) and false for the one-parameter forms "array<X>",
 * "list<X>" and "X[]" (keys are not checked).
 *
 * A NULL return from the parse functions means "no usable array shape
 * found", in which case the property behaves as a plain `array`.
 */
typedef struct av_array_spec {
    bool has_key_spec;
    av_docstring_type_node *key_spec; /* only int|string (or a union of both) are valid keys */
    av_docstring_type_node *value_spec;
} av_array_spec;

/**
 * Parses the @var tag of the property doc comment into an array spec.
 *
 * Returns a newly allocated spec the caller must release with
 * av_docstring_free_array_spec(), or NULL when:
 *  - the property has no doc comment,
 *  - the doc comment has no @var tag,
 *  - the @var type is not an array shape (plain "array", "int", ...),
 *  - the shape is malformed (unbalanced '<', invalid key type, empty).
 *
 * The property doc comment is owned by the zend_property_info and is never
 * released or modified here.
 */
av_array_spec *av_docstring_parse_array_spec(zend_property_info *property);

/**
 * Same as av_docstring_parse_array_spec() but takes the raw @var type
 * expression (e.g. "array<int, string>") instead of a property info.
 * Exposed to allow unit testing the parser without a Zend engine.
 */
av_array_spec *av_docstring_parse_array_type(const char *type_start, size_t type_len);

/**
 * Recursively frees a spec, its union chains, nested specs and the
 * zend_strings owned by them. NULL is a no-op.
 */
void av_docstring_free_array_spec(av_array_spec *spec);

/**
 * Renders a union chain back to its user-facing form, using the same
 * grammar as the native type error messages (", " separated parts with a
 * final " or "):
 *
 *   "string", "string or integer", "Address", "array<int>"
 *
 * Returns a newly allocated zend_string the caller must release.
 */
zend_string *av_docstring_type_list_to_string(const av_docstring_type_node *node);

#endif /* AV_DOCSTRING_PARSER_H */
