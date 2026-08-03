#include "call_function.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_closures.h"
#include "validate_function.h"

/* Function implementation for call */
ZEND_FUNCTION(call)
{
    zval *function;
    zval *params;
    zval *dependencies = NULL;
    
    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_ZVAL(function)
        Z_PARAM_ARRAY(params)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(dependencies)
    ZEND_PARSE_PARAMETERS_END();
    
    // TODO: Implement proper parameter validation
    // For now, just call the function directly without validation
    // This is a stub implementation
    
    zval result;
    zval function_name;
    
    // If function is a string, call it as a function name
    if (Z_TYPE_P(function) == IS_STRING) {
        ZVAL_COPY(&function_name, function);
        
        // Call the function with the provided parameters
        zval retval;
        if (call_user_function_ex(CG(function_table), NULL, &function_name, &retval, Z_ARRVAL_P(params), 0, NULL) == SUCCESS) {
            ZVAL_COPY(&result, &retval);
            zval_ptr_dtor(&retval);
        } else {
            zend_throw_exception_ex(NULL, 0, "Failed to call function %s", Z_STRVAL(function_name));
            zval_ptr_dtor(&function_name);
            RETURN_THROWS();
        }
        zval_ptr_dtor(&function_name);
    }
    // If function is callable (closure, etc.)
    else if (Z_TYPE_P(function) == IS_CALLABLE) {
        // For callable objects, we need to handle them differently
        // This is a simplified implementation
        zend_throw_exception_ex(NULL, 0, "Callable functions not yet supported");
        RETURN_THROWS();
    } else {
        zend_argument_type_error(1, "must be of type string or callable, %s given", zend_zval_type_name(function));
        RETURN_THROWS();
    }
    
    RETURN_COPY(&result);
}
