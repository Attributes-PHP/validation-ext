#include "av_call_function.h"
#include "av_base_model.h"
#include "av_validate_function.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
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
 * The dependencies source: a plain array or an ArrayAccess container.
 *
 * Array entries are borrowed from the hash table; offsetGet() results are
 * owned zvals the fetch releases after use. Containers are only asked for
 * the dependencies the call actually consumes, so a lazy container
 * builds a service only when a parameter needs it.
 */
typedef struct {
    zval *source;
    bool is_array;
    uint32_t positional_cursor;
} av_dependencies;

/* A fetched dependency: owned only when it came from offsetGet() */
typedef struct {
    zval value;
    bool owned;
} av_dependency;

static void av_dependency_release(av_dependency *dependency)
{
    if (dependency->owned) {
        zval_ptr_dtor(&dependency->value);
    }
}

/*
 * Calls offsetExists()/offsetGet() on the container. Returns false without
 * touching retval when the call fails; a container exception propagates
 * as-is through the pending exception.
 */
static bool array_access_offset_call(av_dependencies *dependencies, const char *method, zval *offset, zval *retval)
{
    ZVAL_UNDEF(retval);

    if (zend_call_method(Z_OBJ_P(dependencies->source), NULL, NULL, method, strlen(method), retval, 1, offset, NULL) == NULL || EG(exception) != NULL) {
        if (Z_TYPE_P(retval) != IS_UNDEF) {
            zval_ptr_dtor(retval);
            ZVAL_UNDEF(retval);
        }
        return false;
    }

    return true;
}

/* Fetches a dependency by parameter name, without touching the cursor */
static bool fetch_dependency_by_name(av_dependencies *dependencies, zend_string *name, av_dependency *dependency)
{
    if (dependencies->source == NULL || name == NULL) {
        return false;
    }

    if (dependencies->is_array) {
        zval *entry = zend_hash_find(Z_ARRVAL_P(dependencies->source), name);
        if (entry == NULL) {
            return false;
        }

        ZVAL_COPY_VALUE(&dependency->value, entry);
        dependency->owned = false;
        return true;
    }

    zval offset;
    ZVAL_STR(&offset, name);

    zval exists;
    if (!array_access_offset_call(dependencies, "offsetexists", &offset, &exists)) {
        return false;
    }
    bool has_dependency = zend_is_true(&exists);
    zval_ptr_dtor(&exists);
    if (!has_dependency) {
        return false;
    }

    if (!array_access_offset_call(dependencies, "offsetget", &offset, &dependency->value)) {
        return false;
    }

    dependency->owned = true;
    return true;
}

/*
 * Takes the next unused positional entry of the dependencies source.
 *
 * Positional entries are consumed in order; named entries are matched
 * against parameter names in build_call_arguments() and skipped here.
 * Containers expose their positional entries through integer offsets, so
 * an ArrayObject behaves exactly like the equivalent array.
 */
static bool fetch_next_positional_dependency(av_dependencies *dependencies, av_dependency *dependency)
{
    if (dependencies->source == NULL) {
        return false;
    }

    uint32_t cursor = dependencies->positional_cursor;

    if (dependencies->is_array) {
        while (cursor < zend_hash_num_elements(Z_ARRVAL_P(dependencies->source))) {
            zval *entry = zend_hash_index_find(Z_ARRVAL_P(dependencies->source), cursor);
            cursor++;

            if (entry != NULL) {
                dependencies->positional_cursor = cursor;
                ZVAL_COPY_VALUE(&dependency->value, entry);
                dependency->owned = false;
                return true;
            }
        }

        dependencies->positional_cursor = cursor;
        return false;
    }

    zval offset;
    ZVAL_LONG(&offset, (zend_long)cursor);

    zval exists;
    if (!array_access_offset_call(dependencies, "offsetexists", &offset, &exists)) {
        return false;
    }
    bool has_dependency = zend_is_true(&exists);
    zval_ptr_dtor(&exists);
    dependencies->positional_cursor = cursor + 1;
    if (!has_dependency) {
        return false;
    }

    if (!array_access_offset_call(dependencies, "offsetget", &offset, &dependency->value)) {
        return false;
    }

    dependency->owned = true;
    return true;
}

/*
 * Builds one argument for a non-model parameter from the dependencies
 * source: matched by parameter name first, then in positional order.
 */
static bool find_dependency(av_dependencies *dependencies, zend_string *name, av_dependency *dependency)
{
    if (fetch_dependency_by_name(dependencies, name, dependency)) {
        return true;
    }

    if (UNEXPECTED(EG(exception) != NULL)) {
        return false;
    }

    return fetch_next_positional_dependency(dependencies, dependency);
}

/* Appends an argument to the buffer, growing it when needed */
static void args_push(zval **args, uint32_t *args_count, uint32_t *args_capacity, zval *value)
{
    if (*args_count == *args_capacity) {
        *args_capacity = *args_capacity > 0 ? *args_capacity * 2 : 4;
        *args = erealloc(*args, *args_capacity * sizeof(zval));
    }

    ZVAL_COPY(&(*args)[*args_count], value);
    (*args_count)++;
}

/*
 * Builds the argument list for the target function.
 *
 * BaseModel parameters of userland callables are hydrated from the raw
 * data array through the regular validate() flow (ModelConfigs, hooks,
 * ValidationException on invalid data). Every other parameter takes its
 * value from the dependencies source, matched by parameter name first
 * and then in positional order; missing optional parameters fall back to
 * their declared defaults.
 *
 * Arguments are built with an extra reference the caller releases with
 * release_call_arguments().
 */
static bool build_call_arguments(zend_function *function, zval *params, av_dependencies *dependencies, zval **args, uint32_t *args_count, uint32_t *args_capacity)
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

            args_push(args, &argument_count, args_capacity, &model);
            zval_ptr_dtor(&model);
            continue;
        }

        av_dependency dependency;
        if (find_dependency(dependencies, name, &dependency)) {
            args_push(args, &argument_count, args_capacity, &dependency.value);
            av_dependency_release(&dependency);
            continue;
        }

        if (UNEXPECTED(EG(exception) != NULL)) {
            result = false;
            break;
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
    if (result && (function->common.fn_flags & ZEND_ACC_VARIADIC) != 0 && dependencies->source != NULL) {
        av_dependency dependency;
        while (fetch_next_positional_dependency(dependencies, &dependency)) {
            args_push(args, &argument_count, args_capacity, &dependency.value);
            av_dependency_release(&dependency);
        }

        if (UNEXPECTED(EG(exception) != NULL)) {
            result = false;
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
 * dependencies array or ArrayAccess container. Returns the callable's
 * return value.
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
    Z_PARAM_ZVAL(dependencies)
    ZEND_PARSE_PARAMETERS_END();

    av_dependencies dependencies_source = {
        .source = dependencies,
        .is_array = dependencies != NULL && Z_TYPE_P(dependencies) == IS_ARRAY,
        .positional_cursor = 0,
    };

    // A container is only required to implement ArrayAccess: ArrayObject
    // and any lazy DI container qualify
    if (dependencies != NULL && !dependencies_source.is_array && (Z_TYPE_P(dependencies) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(dependencies), zend_ce_arrayaccess))) {
        zend_argument_type_error(3, "must be of type array|ArrayAccess, %s given", zend_zval_type_name(dependencies));
        RETURN_THROWS();
    }

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

    // The argument buffer is heap allocated: its size must cover every
    // declared parameter plus the leftover dependencies of a variadic
    // target. A container cannot be counted up front, so the buffer grows
    // on demand instead.
    uint32_t args_count = 0;
    uint32_t args_capacity = target->common.num_args;
    if (dependencies != NULL) {
        args_capacity += dependencies_source.is_array ? zend_hash_num_elements(Z_ARRVAL_P(dependencies)) : 8;
    }
    if (args_capacity == 0) {
        args_capacity = 1;
    }
    zval *args = emalloc(args_capacity * sizeof(zval));

    if (!build_call_arguments(target, params, &dependencies_source, &args, &args_count, &args_capacity)) {
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
