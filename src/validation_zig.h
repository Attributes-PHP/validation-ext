#ifndef VALIDATION_ZIG_H
#define VALIDATION_ZIG_H

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations for Zig-implemented validation functions
// These are exported from the Zig compilation unit

/**
 * Validate data against a model using Zig implementation
 * @param data The data to validate
 * @param model The model to validate against
 * @return 0 on success, non-zero on failure
 */
int zig_validate(void *data, void *model);

/**
 * Validate data using a callable with Zig implementation
 * @param data The data to validate
 * @param call The callable to use for validation
 * @return 0 on success, non-zero on failure
 */
int zig_validate_callable(void *data, void *call);

#ifdef __cplusplus
}
#endif

#endif // VALIDATION_ZIG_H
