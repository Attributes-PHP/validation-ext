#include "av_intersection.h"
#include "av_field.h"
#include "Zend/zend_API.h"
#include "Zend/zend_attributes.h"

zend_class_entry *AV_Fields_Intersection_ce;

/*
 * Each arm must name a loadable class or interface, mimicking PHP
 * intersection type hints: enums and traits cannot participate. A missing
 * name autoloads, and a name that still cannot be resolved throws a
 * ValueError (an autoloader error propagates as-is).
 */
static bool is_valid_intersection_class_name(zval *value, uint32_t arg_num)
{
    zend_class_entry *ce = zend_lookup_class(Z_STR_P(value));

    if (ce != NULL && (ce->ce_flags & (ZEND_ACC_TRAIT | ZEND_ACC_ENUM)) == 0) {
        return true;
    }

    if (EG(exception) == NULL) {
        zend_argument_value_error(arg_num, "must be a class or interface, \"%s\" given", ZSTR_VAL(Z_STR_P(value)));
    }

    return false;
}

ZEND_METHOD(AV_Fields_Intersection, __construct)
{
    zval *arms;
    uint32_t arms_count;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_VARIADIC('z', arms, arms_count)
    ZEND_PARSE_PARAMETERS_END();

    /* Each variadic arm: string (class name), per the stub signature */
    for (uint32_t i = 0; i < arms_count; i++) {
        if (Z_TYPE_P(&arms[i]) != IS_STRING) {
            zend_argument_type_error(i + 1, "must be of type string, %s given", zend_zval_type_name(&arms[i]));
            RETURN_THROWS();
        }

        if (!is_valid_intersection_class_name(&arms[i], i + 1)) {
            RETURN_THROWS();
        }
    }

    zval of;
    array_init_size(&of, arms_count);
    for (uint32_t i = 0; i < arms_count; i++) {
        add_next_index_zval(&of, &arms[i]);
        Z_TRY_ADDREF(arms[i]);
    }

    zend_update_property(AV_Fields_Intersection_ce, Z_OBJ_P(getThis()), "of", sizeof("of") - 1, &of);
    zval_ptr_dtor(&of);
}

void av_register_Intersection_class(void)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Attributes\\Validation\\Fields", "Intersection", class_AV_Fields_Intersection_methods);
    AV_Fields_Intersection_ce = zend_register_internal_class_ex(&ce, NULL);
    ZEND_ASSERT(AV_Fields_Intersection_ce != NULL);

    ZEND_ASSERT(AV_Fields_Field_ce != NULL);
    zend_class_implements(AV_Fields_Intersection_ce, 1, AV_Fields_Field_ce);

    /* Register $of property */
    zend_declare_property_null(AV_Fields_Intersection_ce, "of", sizeof("of") - 1, ZEND_ACC_PUBLIC);

    /* Register as an internal attribute that can be used on properties and parameters */
    zend_internal_attribute_register(AV_Fields_Intersection_ce, ZEND_ATTRIBUTE_TARGET_PROPERTY | ZEND_ATTRIBUTE_TARGET_PARAMETER);
}
