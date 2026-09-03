#include "php.h"
#include "zend_string.h"
#include "zend_type_info.h"
#include "zend_types.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "../av_model_configs.h"
#include "../av_base_model.h"
#include "../av_exception.h"
#include "../helpers/av_error_messages.h"
#include <stdint.h>
#include "av_typehint_validator.h"
#include <math.h>

zend_class_entry *datetime_ce;
zend_class_entry *datetime_interface_ce;

/**
 * Initializes necessary class entries.
 */
void av_init_typehint_validator() {
    zend_string *datetime_str = zend_string_init("DateTime", sizeof("DateTime") - 1, 0);
    datetime_ce = zend_lookup_class_ex(datetime_str, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    zend_string_release(datetime_str);

    zend_string *datetime_interface_str = zend_string_init("DateTimeInterface", sizeof("DateTimeInterface") - 1, 0);
    datetime_interface_ce = zend_lookup_class_ex(datetime_interface_str, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    zend_string_release(datetime_interface_str);

    if (!datetime_ce || !datetime_interface_ce) {
        zend_throw_error(NULL, "DateTime or DateTimeInterface class not available. Ensure the datetime extension is loaded.");
    }
}

static zend_class_entry *resolve_single_class_type(zend_string *name, zend_class_entry *self_ce) {
    if (zend_string_equals_literal_ci(name, "self")) {
        return self_ce;
    } else if (zend_string_equals_literal_ci(name, "parent")) {
        return self_ce->parent;
    } else {
        return zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
    }
}

static zend_always_inline zend_class_entry *get_ce_from_type(zend_property_info *info, zend_type *type) {
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(*type));
    zend_string *name = ZEND_TYPE_NAME(*type);
    if (ZSTR_HAS_CE_CACHE(name)) {
        zend_class_entry *ce = ZSTR_GET_CE_CACHE(name);
        if (!ce) {
            ce = zend_lookup_class_ex(name, NULL, ZEND_FETCH_CLASS_NO_AUTOLOAD);
        }
        return ce;
    }
    return resolve_single_class_type(name, info->ce);
}

static bool handle_intersection(av_field *field, av_property_info *prop_info, zend_type *value_type) {
    zend_type *intersection_type;
    ZEND_ASSERT(ZEND_TYPE_IS_INTERSECTION(*value_type));

    ZEND_TYPE_LIST_FOREACH(ZEND_TYPE_LIST(*value_type), intersection_type) {
        ZEND_ASSERT(!ZEND_TYPE_HAS_LIST(*intersection_type));

        zend_class_entry *ce = get_ce_from_type(prop_info->property, intersection_type);
        if (!ce || !instanceof_function(Z_OBJCE_P(field->value), ce)) {
            return false;
        }
    } ZEND_TYPE_LIST_FOREACH_END();
    return false;
}

/**
 * Check if a class entry is DateTime or implements DateTimeInterface.
 */
static bool is_datetime_class(zend_class_entry *ce) {
    if (!ce) return false;
    return (ce == datetime_ce || ce == datetime_interface_ce || 
            instanceof_function(ce, datetime_interface_ce));
}

static bool coerce_datetime(zval *value, zend_class_entry *target_ce, av_model_configs_properties *properties) {
    const zend_string *str = Z_STR_P(value);
    if (ZSTR_LEN(str) <= 12) {
        return false;
    }

    zval format, datetime_obj;
    ZVAL_STRING(&format, "X-m-d\\TH:i:sP");

    zval args[2];
    ZVAL_COPY(&args[0], &format);
    ZVAL_COPY(&args[1], value);

    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval function_name;

    ZVAL_STRING(&function_name, "date_create_from_format");

    fci.size = sizeof(fci);
    fci.object = NULL;
    fci.function_name = function_name;
    fci.retval = &datetime_obj;
    fci.param_count = 2;
    fci.params = args;
    fci.named_params = NULL;

    fcc.function_handler = NULL;
    fcc.calling_scope = NULL;
    fcc.called_scope = NULL;
    fcc.object = NULL;

    zend_result result = zend_call_function(&fci, &fcc);

    zval_ptr_dtor(&format);
    zval_ptr_dtor(&function_name);

    if (result == SUCCESS && Z_TYPE(datetime_obj) == IS_OBJECT && Z_OBJCE_P(&datetime_obj) == datetime_ce) {
        zval_ptr_dtor(value);
        ZVAL_COPY(value, &datetime_obj);
        return true;
    }

    if (Z_TYPE(datetime_obj) != IS_UNDEF) {
        zval_ptr_dtor(&datetime_obj);
    }

    return false;
}

static bool handle_class(av_field *field, av_property_info *prop_info, zend_type *value_type, av_model_configs_properties *properties, zval *errors) {
    ZEND_ASSERT(ZEND_TYPE_HAS_NAME(*value_type));

    zend_class_entry *ce = get_ce_from_type(prop_info->property, value_type);
    if (!ce) {
        return false;
    }

    if (Z_TYPE_P(field->value) == IS_STRING && is_datetime_class(ce)) {
        return coerce_datetime(field->value, ce, properties);
    }

    if (Z_TYPE_P(field->value) == IS_OBJECT) {
        return instanceof_function(Z_OBJCE_P(field->value), ce);
    }

    if (Z_TYPE_P(field->value) == IS_ARRAY) {
        if (!ce || ce == AV_BaseModel_ce || !instanceof_function(ce, AV_BaseModel_ce)) return false;

        zval model_obj;
        object_init_ex(&model_obj, ce);

        zend_string *nested_path = field->parent ?
            zend_string_copy(field->parent) :
            zend_string_copy(field->name);

        bool result = av_validate_model_internal(field->value, prop_info, properties, errors, nested_path);

        zend_string_release(nested_path);

        if (result) {
            zval_ptr_dtor(field->value);
            ZVAL_COPY(field->value, &model_obj);
        } else {
            zval_ptr_dtor(&model_obj);
        }

        return result;
    }

    return false;
}

static bool coerce_bool(av_field *field) {
    zend_uchar type_code = Z_TYPE_P(field->value);

    switch (type_code) {
        case IS_LONG: {
            zend_long lval = Z_LVAL_P(field->value);
            if (lval == 0) {
                ZVAL_FALSE(field->value);
                return true;
            }

            if (lval == 1) {
                ZVAL_TRUE(field->value);
                return true;
            }

            break;
        }
        case IS_DOUBLE: {
            double dval = Z_DVAL_P(field->value);
            if (fabs(dval - 1.0) <= AV_EPSILON) {
                ZVAL_TRUE(field->value);
                return true;
            }

            if (fabs(dval - 0.0) <= AV_EPSILON) {
                ZVAL_FALSE(field->value);
                return true;
            }

            break;
        }
        case IS_STRING: {
            zend_string *s = Z_STR_P(field->value);
            size_t len = ZSTR_LEN(s);
            char *str = ZSTR_VAL(s);

            switch (len) {
                case 1: // '1', 't', 'y' vs '0', 'f', 'n'
                    if (str[0] == '1' || str[0] == 't' || str[0] == 'T' || str[0] == 'y' || str[0] == 'Y') {
                        ZVAL_TRUE(field->value);
                        return true;
                    }
                    if (str[0] == '0' || str[0] == 'f' || str[0] == 'F' || str[0] == 'n' || str[0] == 'N') {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;
                case 2: // "on" vs "no"
                    if (zend_binary_strcasecmp(str, 2, "on", 2) == 0) {
                        ZVAL_TRUE(field->value);
                        return true;
                    }
                    if (zend_binary_strcasecmp(str, 2, "no", 2) == 0) {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;

                case 3: // "yes" vs "off"
                    if (zend_binary_strcasecmp(str, 3, "yes", 3) == 0) {
                        ZVAL_TRUE(field->value);
                        return true;
                    }
                    if (zend_binary_strcasecmp(str, 3, "off", 3) == 0) {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;

                case 4: // "true"
                    if (zend_binary_strcasecmp(str, 4, "true", 4) == 0) {
                        ZVAL_TRUE(field->value);
                        return true;
                    }

                    break;

                case 5: // "false"
                    if (zend_binary_strcasecmp(str, 5, "false", 5) == 0) {
                        ZVAL_FALSE(field->value);
                        return true;
                    }

                    break;
            }

            break;
        }
    }

    return false;
}

/**
 * Validates that a value matches the property's type hint.
 *
 * For union types, tries each type in the union.
 *
 * @param field         The field related structure
 * @param prop_info     Property type information
 * @param properties    Model configuration properties (for recursive validation)
 * @param errors        Error collection array
 * @return              true if validation succeeds, false otherwise
 */
bool av_validate_type_hint(av_field *field, av_property_info *prop_info, av_model_configs_properties *properties, zval *errors)
{
    zend_type property_type = prop_info->property->type;

    if (!ZEND_TYPE_IS_SET(property_type)) return true;

    if (ZEND_TYPE_CONTAINS_CODE(property_type, Z_TYPE_P(field->value))) return true;

    zend_type *type;
    ZEND_TYPE_FOREACH(property_type, type) {
        if (ZEND_TYPE_IS_INTERSECTION(*type)) {
            if (handle_intersection(field, prop_info, type)) return true;
            continue;
        }

        if (ZEND_TYPE_HAS_NAME(*type)) {
            if (handle_class(field, prop_info, type, properties, errors)) return true;
            continue;
        }

        uint32_t type_mask = ZEND_TYPE_PURE_MASK(*type);
        if (!properties->strict && type_mask & MAY_BE_BOOL) {
            if (coerce_bool(field)) return true;
            continue;
        }

        if (zend_verify_scalar_type_hint(type_mask, field->value, properties->strict, 0)) return true;
    }  ZEND_TYPE_FOREACH_END();

    av_add_field_error_with_prefix(AV_ERROR_TYPE, field, prop_info, errors);
    return false;
}
