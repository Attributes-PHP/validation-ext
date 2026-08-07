#ifndef AV_HELPERS_DOCSTRINGS_H
#define AV_HELPERS_DOCSTRINGS_H

#include <Zend/zend_types.h>

/* Extract the type from @var tag in a docstring, returns NULL if not found */
zend_string* av_extract_var_type(const char *doc_comment, size_t doc_comment_len);

/* Extract array type specifically from @var tag, returns NULL if not an array type */
zend_string* av_extract_array_type(const char *doc_comment, size_t doc_comment_len);

#endif /* AV_HELPERS_DOCSTRINGS_H */
