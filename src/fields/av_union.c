#include "av_union.h"
#include "av_field.h"
#include "../av_type.h"
#include "av_dict.h"
#include "av_intersection.h"
#include "av_sequence.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Union_ce;

/*
 * Each variadic arm: int|string|\Attributes\Validation\Type|Sequence|Union|Intersection|Dict
 *
 * Checked eagerly so a malformed Union fails at construction instead of at
 * validation time; the validator still applies the semantic rules (valid
 * masks, resolvable class names).
 */
static bool is_valid_union_arm(zval *value)
{
    switch (Z_TYPE_P(value)) {
        case IS_LONG:
        case IS_STRING:
            return true;

        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(value);
            return ce == AV_Type_ce || ce == AV_Fields_Sequence_ce || ce == AV_Fields_Union_ce || ce == AV_Fields_Intersection_ce || ce == AV_Fields_Dict_ce;
        }

        default:
            return false;
    }
}

/*
 * A string arm must name a loadable class, interface or enum: a missing
 * name autoloads, and a name that still cannot be resolved throws a
 * ValueError (an autoloader error propagates as-is).
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

ZEND_METHOD(AV_Fields_Union, __construct)
{
    zval *arms;
    uint32_t arms_count;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('z', arms, arms_count)
    ZEND_PARSE_PARAMETERS_END();

    for (uint32_t i = 0; i < arms_count; i++) {
        if (!is_valid_union_arm(&arms[i])) {
            zend_argument_type_error(i + 1, "must be of type int|string|\\Attributes\\Validation\\Type|Sequence|Union|Intersection|Dict, %s given", zend_zval_type_name(&arms[i]));
            RETURN_THROWS();
        }

        if (Z_TYPE_P(&arms[i]) == IS_STRING && !is_resolvable_class_name(&arms[i], i + 1)) {
            RETURN_THROWS();
        }
    }

    zval of;
    array_init_size(&of, arms_count);
    for (uint32_t i = 0; i < arms_count; i++) {
        add_next_index_zval(&of, &arms[i]);
        Z_TRY_ADDREF(arms[i]);
    }

    zend_update_property(AV_Fields_Union_ce, Z_OBJ_P(getThis()), "of", sizeof("of") - 1, &of);
    zval_ptr_dtor(&of);
}

void av_register_Union_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Union", class_AV_Fields_Union_methods);
    AV_Fields_Union_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Union_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Union_ce, 1, AV_Fields_Field_ce);

    /* Register $of property */
    zend_declare_property_null(AV_Fields_Union_ce, "of", sizeof("of") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Union_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
