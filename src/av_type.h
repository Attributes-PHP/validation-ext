/**
 * Type enum and type constants
 *
 * Registers the Attributes\Validation\Type int-backed enum that names the
 * element types accepted by the array type-hint attributes (Sequence,
 * Union, Intersection and Dict), plus the sugar constants of the same
 * names in the Attributes\Validation namespace so attribute arguments read
 * #[Union(bool, int, string)].
 *
 * The backing values are the Zend engine's MAY_BE_* type masks, so C code
 * can compare Type case values against engine masks directly, without a
 * translation table. Attribute arguments may combine them with the
 * bitwise OR operator (#[Union(bool | int | string)]); the bitmask form
 * loses the declaration order that plain argument lists keep, so
 * coercions apply in a fixed order instead.
 *
 * The engine has no mask for dict-shaped arrays (it only knows array):
 * the dict case carries a private bit, far above every engine mask
 * (including the shifted MAY_BE_ARRAY_OF_* family), which the validator
 * folds into MAY_BE_ARRAY.
 */

#ifndef AV_TYPE_H
#define AV_TYPE_H

#include "php.h"
#include "Zend/zend_type_info.h"

/* Type enum class entry */
extern zend_class_entry *AV_Type_ce;

/* Type case backing values: the Zend engine type masks */
#define AV_TYPE_BOOL     MAY_BE_BOOL
#define AV_TYPE_INT      MAY_BE_LONG
#define AV_TYPE_FLOAT    MAY_BE_DOUBLE
#define AV_TYPE_STRING   MAY_BE_STRING
#define AV_TYPE_SEQUENCE MAY_BE_ARRAY
#define AV_TYPE_OBJECT   MAY_BE_OBJECT

/* Private dict bit: no engine mask exists for dict-shaped arrays */
#define AV_TYPE_DICT ((zend_long)(1 << 28))

/* Every mask a Type case value may carry */
#define AV_TYPE_ALL_BITS (MAY_BE_BOOL | MAY_BE_LONG | MAY_BE_DOUBLE | MAY_BE_STRING | MAY_BE_ARRAY | MAY_BE_OBJECT | ((uint32_t)AV_TYPE_DICT))

/* Registration functions */
void av_register_Type_enum(void);
void av_register_type_constants(int module_number);

#endif /* AV_TYPE_H */
