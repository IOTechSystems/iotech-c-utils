//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_TYPECODE_H_
#define _IOT_TYPECODE_H_

/**
 * @file
 * @brief IOTech TypeCode API
 */

#include "iot/data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate a basic typecode
 *
 * The function allocates a typecode for a basic type (i.e. not a map, array or vector).
 *
 * @param type Basic typecode type
 * @return     Pointer and allocated typecode of the requested type
 */
extern iot_typecode_t * iot_typecode_alloc_basic (iot_data_type_t type);

/**
 * @brief Allocate a map typecode
 *
 * The function allocates a typecode for a map, setting the key and element type. The key
 * type must be a basic type and the element type can be any type or NULL to indicate a variable as opposed
 * to a fixed type element.
 *
 * @param key_type     The type of the map key
 * @param element_type The type of the map element, NULL to indicate any type
 * @return             Pointer to the allocated map typecode
 */
extern iot_typecode_t * iot_typecode_alloc_map (iot_data_type_t key_type, iot_typecode_t * element_type);

/**
 * @brief Allocate an array typecode
 *
 * The function allocates a typecode for an array, only simple C array types are supported (all basic
 * types except String).
 *
 * @param element_type The type of the array element
 * @return             Pointer to the allocated array typecode
 */
extern iot_typecode_t * iot_typecode_alloc_array (iot_data_type_t element_type);

/**
 * @brief Allocate a vector typecode
 *
 * The function allocates a typecode for a vector, setting the element type. The element type
 * can be any type or NULL to indicate a variable as opposed to a fixed type element.
 *
 * @param element_type The type of the vector element, NULL to indicate any type
 * @return             Pointer to the allocated vector typecode
 */
extern iot_typecode_t * iot_typecode_alloc_vector (iot_typecode_t * element_type);

/**
 * @brief Free a typecode
 *
 * The function frees an allocated typecode
 *
 * @param typecode Pointer to the typecode to be freed
 */
extern void iot_typecode_free (iot_typecode_t * typecode);

/**
 * @brief Returns whether two typecodes are equal
 *
 * The function compares whether two typecodes are equal.
 *
 * @param tc1 The first typecode to compare
 * @param tc2 The second typecode to compare
 * @return    Whether the two typecodes are equal
 */
extern bool iot_typecode_equal (const iot_typecode_t * tc1, const iot_typecode_t * tc2);

/**
 * @brief Returns the type of a typecode
 *
 * The function returns the type of a typecode. For example a typecode may represent an array of strings,
 * but it's type is array.
 *
 * @param typecode Pointer to the typecode
 * @return         The type of the typecode
 */
extern iot_data_type_t iot_typecode_type (const iot_typecode_t * typecode);

/**
 * @brief Returns the string type name of a typecode
 *
 * The function returns the type name of a typecode. For example a typecode may represent an array of strings,
 * but it's type name is "Array".
 *
 * @param typecode Pointer to the typecode
 * @return         The string type of the typecode
 */
const char * iot_typecode_type_name (const iot_typecode_t * typecode);

/**
 * @brief Returns the key type of a map typecode
 *
 * The function returns the key type of a map typecode. For example a typecode may represent a map of boolean
 * keyed by string, where the key type is string.
 *
 * @param typecode Pointer to the array typecode
 * @return         The type of the array typecode key
 */
extern iot_data_type_t iot_typecode_key_type (const iot_typecode_t * typecode);

/**
 * @brief Returns the element type of a map, vector or array typecode
 *
 * The function returns the element type of a map, vector or array typecode. For example a typecode may represent a map of boolean
 * keyed by string, where the element type is boolean.
 *
 * @param typecode Pointer to the array, map or vector typecode
 * @return         The element type of the map, array or vector. May be NULL for vectors or maps of varying element type
 */
extern const iot_typecode_t * iot_typecode_element_type (const iot_typecode_t * typecode);

#ifdef __cplusplus
}
#endif
#endif
