#include "av_call_function.h"
#include "av_base_model.h"
#include "av_validate_function.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "zend_hash.h"
#include "zend_string.h"
#include "zend_types.h"
#include <string.h>

/*
 * Whether the target's arg_info uses the userland layout (zend_arg_info
 * with zend_string names). Internal functions keep a different layout
 * (zend_internal_arg_info with char* names) and cannot declare model
 * parameters, so they receive their dependencies positionally only.
 */
static bool target_has_user_arg_info(const zend_function *function)
{
    return function->type != ZEND_INTERNAL_FUNCTION || (function->common.fn_flags & ZEND_ACC_USER_ARG_INFO) != 0;
}

/*
 * Resolves the class entry of a userland parameter type hint, or NULL when
 * the parameter has no single class type.
 */
static zend_class_entry *parameter_class_ce(const zend_arg_info *arg_info, zend_class_entry *scope)
{
    if (!ZEND_TYPE_IS_SET(arg_info->type) || !ZEND_TYPE_HAS_NAME(arg_info->type)) {
        return NULL;
    }

    zend_string *name = ZEND_TYPE_NAME(arg_info->type);
    if (ZSTR_HAS_CE_CACHE(name)) {
        zend_class_entry *ce = ZSTR_GET_CE_CACHE(name);
        if (!ce) {
            ce = zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
        }
        return ce;
    }

    if (zend_string_equals_literal_ci(name, "self")) {
        return scope;
    }

    if (zend_string_equals_literal_ci(name, "parent")) {
        return scope != NULL ? scope->parent : NULL;
    }

    return zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_DEFAULT);
}

/*
 * Takes the next unused positional entry of the dependencies array.
 *
 * Positional entries are consumed in order; named entries are matched
 * against parameter names in build_call_arguments() and skipped here.
 */
static zval *next_positional_dependency(HashTable *dependencies, uint32_t *cursor)
{
    uint32_t index = *cursor;
    while (index < zend_hash_num_elements(dependencies)) {
        zval *entry = zend_hash_index_find(dependencies, index);
        index++;

        if (entry != NULL) {
            *cursor = index;
            return entry;
        }
    }

    *cursor = index;
    return NULL;
}

/*
 * Builds one argument for a non-model parameter from the dependencies
 * array: matched by parameter name first, then in positional order.
 */
static zval *find_dependency(zval *dependencies, zend_string *name, uint32_t *cursor)
{
    if (dependencies == NULL) {
        return NULL;
    }

    if (name != NULL) {
        zval *dependency = zend_hash_find(Z_ARRVAL_P(dependencies), name);
        if (dependency != NULL) {
            return dependency;
        }
    }

    return next_positional_dependency(Z_ARRVAL_P(dependencies), cursor);
}

/*
 * Builds the argument list for the target function.
 *
 * BaseModel parameters of userland callables are hydrated from the raw
 * data array through the regular validate() flow (ModelConfigs, hooks,
 * ValidationException on invalid data). Every other parameter takes its
 * value from the dependencies array, matched by parameter name first and
 * then in positional order; missing optional parameters fall back to
 * their declared defaults.
 *
 * Arguments are built with an extra reference the caller releases with
 * release_call_arguments().
 */
static bool build_call_arguments(zend_function *function, zval *params, zval *dependencies, uint32_t *cursor, zval *args, uint32_t *args_count)
{
    uint32_t argument_count = 0;
    bool user_arg_info = target_has_user_arg_info(function);
    bool result = true;

    for (uint32_t i = 0; i < function->common.num_args; i++) {
        zend_class_entry *ce = NULL;
        zend_string *name = NULL;

        if (user_arg_info) {
            const zend_arg_info *arg_info = &function->common.arg_info[i];
            name = arg_info->name;
            ce = parameter_class_ce(arg_info, function->common.scope);
        }

        if (ce != NULL && instanceof_function(ce, AV_BaseModel_ce)) {
            zval model;
            object_init_ex(&model, ce);
            if (UNEXPECTED(EG(exception) != NULL)) {
                zval_ptr_dtor(&model);
                result = false;
                break;
            }

            if (av_hydrate_model(params, &model) == FAILURE) {
                zval_ptr_dtor(&model);
                result = false;
                break;
            }

            ZVAL_COPY_VALUE(&args[argument_count], &model);
            argument_count++;
            continue;
        }

        zval *dependency = find_dependency(dependencies, name, cursor);
        if (dependency != NULL) {
            ZVAL_COPY(&args[argument_count], dependency);
            argument_count++;
            continue;
        }

        // Missing values for optional parameters let the engine apply the
        // declared defaults; missing values for required ones are an error
        if (i >= function->common.required_num_args) {
            break;
        }

        const char *callable_name = function->common.function_name != NULL ? ZSTR_VAL(function->common.function_name) : "the callable";
        if (name != NULL) {
            zend_throw_exception_ex(zend_ce_error, 0, "No value provided for parameter $%s of %s().", ZSTR_VAL(name), callable_name);
        } else {
            zend_throw_exception_ex(zend_ce_error, 0, "No value provided for parameter %u of %s().", (unsigned)(i + 1), callable_name);
        }
        result = false;
        break;
    }

    // Variadic targets receive the leftover positional dependencies
    if (result && (function->common.fn_flags & ZEND_ACC_VARIADIC) != 0 && dependencies != NULL) {
        zval *dependency;
        while ((dependency = next_positional_dependency(Z_ARRVAL_P(dependencies), cursor)) != NULL) {
            ZVAL_COPY(&args[argument_count], dependency);
            argument_count++;
        }
    }

    *args_count = argument_count;
    return result;
}

static void release_call_arguments(zval *args, uint32_t args_count)
{
    for (uint32_t i = 0; i < args_count; i++) {
        zval_ptr_dtor(&args[i]);
    }
}

/*
 * Function implementation for call
 *
 * Calls a function with validated models: BaseModel parameters are
 * hydrated from the raw data array, other parameters come from the
 * dependencies array. Returns the callable's return value.
 */
ZEND_FUNCTION(call)
{
    zval *function;
    zval *params;
    zval *dependencies = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 3)
    Z_PARAM_ZVAL(function)
    Z_PARAM_ARRAY(params)
    Z_PARAM_OPTIONAL
    Z_PARAM_ARRAY(dependencies)
    ZEND_PARSE_PARAMETERS_END();

    char *error = NULL;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    if (zend_fcall_info_init(function, 0, &fci, &fcc, NULL, &error) != SUCCESS) {
        if (error != NULL) {
            zend_throw_exception(zend_ce_type_error, error, 0);
            efree(error);
        }
        RETURN_THROWS();
    }

    zend_function *target = fcc.function_handler;
    uint32_t args_count = 0;
    uint32_t cursor = 0;

    // The argument buffer is heap allocated: its size must cover every
    // declared parameter plus the leftover dependencies of a variadic
    // target
    uint32_t max_args = target->common.num_args + (dependencies != NULL ? zend_hash_num_elements(Z_ARRVAL_P(dependencies)) : 0);
    zval *args = emalloc((max_args > 0 ? max_args : 1) * sizeof(zval));

    if (!build_call_arguments(target, params, dependencies, &cursor, args, &args_count)) {
        release_call_arguments(args, args_count);
        efree(args);
        RETURN_THROWS();
    }

    zval retval;
    ZVAL_UNDEF(&retval);

    fci.size = sizeof(fci);
    fci.object = fcc.object;
    fci.retval = &retval;
    fci.param_count = args_count;
    fci.params = args;
    fci.named_params = NULL;

    zend_result call_result = zend_call_function(&fci, &fcc);

    release_call_arguments(args, args_count);
    efree(args);

    if (call_result != SUCCESS || UNEXPECTED(EG(exception) != NULL)) {
        if (Z_TYPE(retval) != IS_UNDEF) {
            zval_ptr_dtor(&retval);
        }
        RETURN_THROWS();
    }

    ZVAL_COPY_VALUE(return_value, &retval);
}
