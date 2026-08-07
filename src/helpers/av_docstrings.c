#include "av_docstrings.h"
#include "Zend/zend_API.h"
#include <string.h>

/**
 * Extracts the type from the first @var tag in a docstring.
 * Returns a new zend_string with the type, or NULL if not found.
 * 
 * Example:
 *   Input: "@var array<string, int>"
 *   Output: "array<string, int>"
 */
zend_string* av_extract_var_type(const char *doc_comment, size_t doc_comment_len)
{
    const char *ptr = doc_comment;
    const char *end = doc_comment + doc_comment_len;
    const char *type_start = NULL;
    size_t type_len = 0;

    // Find @var (4 bytes)
    while (ptr + 4 <= end) {
        if (memcmp(ptr, "@var", 4) == 0) {
            ptr += 4;
            
            // Skip whitespace
            while (ptr < end && (*ptr == ' ' || *ptr == '\t')) {
                ptr++;
            }
            
            type_start = ptr;
            
            // Scan until newline or space not preceded by backslash
            while (ptr < end) {
                if (*ptr == '\n') {
                    break;
                }
                if (*ptr == ' ' && *(ptr - 1) != '\\') {
                    break;
                }
                ptr++;
            }
            
            type_len = ptr - type_start;
            
            if (type_len > 0) {
                return zend_string_init(type_start, type_len, 0);
            }
            break; // Only process first @var
        }
        ptr++;
    }
    
    return NULL;
}

/**
 * Extracts the array type from the first @var tag in a docstring.
 * Returns a new zend_string with the array type if it starts with "array",
 * "list", or ends with "[]", otherwise returns NULL.
 * 
 * Example:
 *   Input: "@var array<string, int>"
 *   Output: "array<string, int>"
 *   
 *   Input: "@var list<string>"
 *   Output: "list<string>"
 *   
 *   Input: "@var string[]"
 *   Output: "string[]"
 */
zend_string* av_extract_array_type(const char *doc_comment, size_t doc_comment_len)
{
    const char *ptr = doc_comment;
    const char *end = doc_comment + doc_comment_len;
    const char *type_start = NULL;
    size_t type_len = 0;

    // Find @var (4 bytes) - ultra-fast byte search
    while (ptr + 4 <= end) {
        if (memcmp(ptr, "@var", 4) == 0) {
            ptr += 4;
            
            // Skip whitespace
            while (ptr < end && (*ptr == ' ' || *ptr == '\t')) {
                ptr++;
            }
            
            type_start = ptr;
            
            // Scan until newline or space not preceded by backslash
            while (ptr < end) {
                if (*ptr == '\n') {
                    break;
                }
                if (*ptr == ' ' && *(ptr - 1) != '\\') {
                    break;
                }
                ptr++;
            }
            
            type_len = ptr - type_start;
            
            if (type_len > 0) {
                // Quick check for array-related types
                // Check for "array" prefix (5 bytes)
                if (type_len >= 5 && memcmp(type_start, "array", 5) == 0) {
                    return zend_string_init(type_start, type_len, 0);
                }
                
                // Check for "list" prefix (4 bytes)
                if (type_len >= 4 && memcmp(type_start, "list", 4) == 0) {
                    return zend_string_init(type_start, type_len, 0);
                }
                
                // Check for "[]" suffix
                if (type_len >= 2 && type_start[type_len - 2] == '[' && type_start[type_len - 1] == ']') {
                    return zend_string_init(type_start, type_len, 0);
                }
            }
            break; // Only process first @var
        }
        ptr++;
    }
    
    return NULL;
}
