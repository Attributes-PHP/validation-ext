#include "av_dict.h"
#include "av_field.h"
#include "../av_type.h"
#include "av_intersection.h"
#include "av_sequence.h"
#include "av_union.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Dict_ce;

/*
 * Accepted shapes of the __construct arguments, checked eagerly so a
 * malformed Dict fails at construction instead of at validation time.
 * The validator still applies the semantic rules (key arms restricted to
 * the integer and string types, valid masks, resolvable class names).
 */

/* $key: int|\Attributes\Validation\Type|Union */
static bool is_valid_dict_key(zval *value)
{
    switch (Z_TYPE_P(value)) {
        case IS_LONG:
            return true;

        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(value);
            return ce == AV_Type_ce || ce == AV_Fields_Union_ce;
        }

        default:
            return false;
    }
}

/* $of: int|string|\Attributes\Validation\Type|Union|Intersection|Sequence|Dict */
static bool is_valid_dict_of(zval *value)
{
    switch (Z_TYPE_P(value)) {
        case IS_LONG:
        case IS_STRING:
            return true;

        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(value);
            return ce == AV_Type_ce || ce == AV_Fields_Union_ce || ce == AV_Fields_Intersection_ce || ce == AV_Fields_Sequence_ce || ce == AV_Fields_Dict_ce;
        }

        default:
            return false;
    }
}

/*
 * A string argument must name a loadable class, interface or enum: a
 * missing name autoloads, and a name that still cannot be resolved throws
 * a ValueError (an autoloader error propagates as-is).
 */
static bool is_resolvable_class_name(zval *value, uint32_t arg_num)
{
    if (zend_lookup_class(Z_STR_P(value)) != NULL) {
        return true;
    }

    if (EG(exception) == NULL) {
        zend_argument_value_error(arg_num, "must be a valid class, interface or enum, \"%s\" given", ZSTR_VAL(Z_STR_P(value)));
    }

    return false;
}

ZEND_METHOD(AV_Fields_Dict, __construct)
{
    zval *key, *of;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ZVAL(key)
    Z_PARAM_ZVAL(of)
    ZEND_PARSE_PARAMETERS_END();

    if (!is_valid_dict_key(key)) {
        zend_argument_type_error(1, "must be of type int|\\Attributes\\Validation\\Type|Union, %s given", zend_zval_type_name(key));
        RETURN_THROWS();
    }

    if (!is_valid_dict_of(of)) {
        zend_argument_type_error(2, "must be of type int|string|\\Attributes\\Validation\\Type|Union|Intersection|Sequence|Dict, %s given", zend_zval_type_name(of));
        RETURN_THROWS();
    }

    if (Z_TYPE_P(of) == IS_STRING && !is_resolvable_class_name(of, 2)) {
        RETURN_THROWS();
    }

    zend_update_property(AV_Fields_Dict_ce, Z_OBJ_P(getThis()), "key", sizeof("key") - 1, key);
    zend_update_property(AV_Fields_Dict_ce, Z_OBJ_P(getThis()), "of", sizeof("of") - 1, of);
}

void av_register_Dict_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Dict", class_AV_Fields_Dict_methods);
    AV_Fields_Dict_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Dict_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Dict_ce, 1, AV_Fields_Field_ce);

    /* Register $key and $of properties */
    zend_declare_property_null(AV_Fields_Dict_ce, "key", sizeof("key") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(AV_Fields_Dict_ce, "of", sizeof("of") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Dict_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
