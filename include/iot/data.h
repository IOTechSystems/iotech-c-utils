//
// Copyright (c) 2019-2023 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_DATA_H_
#define _IOT_DATA_H_

/**
 * @file
 * @brief IOTech Data Representation API
 */

#include "iot/defs.h"
#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data type enumeration
 */
typedef enum iot_data_type_t
{
  IOT_DATA_INT8 = 0u,     /**< Signed 8 bit integer */
  IOT_DATA_UINT8 = 1u,    /**< Unsigned 8 bit integer */
  IOT_DATA_INT16 = 2u,    /**< Signed 16 bit integer */
  IOT_DATA_UINT16 = 3u,   /**< Unsigned 16 bit integer */
  IOT_DATA_INT32 = 4u,    /**< Signed 32 bit integer */
  IOT_DATA_UINT32 = 5u,   /**< Unsigned 32 bit integer */
  IOT_DATA_INT64 = 6u,    /**< Signed 64 bit integer */
  IOT_DATA_UINT64 = 7u,   /**< Unsigned 64 bit integer */
  IOT_DATA_FLOAT32 = 8u,  /**< 32 bit float */
  IOT_DATA_FLOAT64 = 9u,  /**< 64 bit float */
  IOT_DATA_BOOL = 10u,    /**< Boolean */
  IOT_DATA_POINTER = 11u, /**< Pointer */
  IOT_DATA_STRING = 12u,  /**< String */
  IOT_DATA_NULL = 13u,    /**< Null */
  IOT_DATA_BINARY = 14u,  /**< Binary */
  IOT_DATA_ARRAY = 15u,   /**< Array of basic type (integer, float, bool or pointer) */
  IOT_DATA_VECTOR = 16u,  /**< Vector */
  IOT_DATA_LIST = 17u,    /**< List */
  IOT_DATA_MAP = 18u,     /**< Map */
  IOT_DATA_MULTI = 19u,   /**< Multiple data types, used for array, map, vector and list contained type */
  IOT_DATA_INVALID = 20u  /**< Invalid data type */
} __attribute__ ((__packed__)) iot_data_type_t;

/** Macro to determine type for C int */
#define IOT_DATA_INT ((sizeof (int) == sizeof (int64_t)) ? IOT_DATA_INT64 : IOT_DATA_INT32)
/** Macro to determine type for C unsigned */
#define IOT_DATA_UNSIGNED ((sizeof (unsigned) == sizeof (uint64_t)) ? IOT_DATA_UINT64 : IOT_DATA_UINT32)

/**
 * Data ownership enumeration
 */
typedef enum iot_data_ownership_t
{
  IOT_DATA_COPY = 0u, /**< Data is copied and copy freed when no longer used */
  IOT_DATA_TAKE = 1u, /**< Data is taken and freed when no longer used */
  IOT_DATA_REF = 2u   /**< Data is referenced and never freed */
} iot_data_ownership_t;

/** Enum type for data tags */
typedef enum iot_data_tag_t
{
  IOT_DATA_TAG_USER1 = 1u, /**< Identifier for user tag 1 */
  IOT_DATA_TAG_USER2 = 2u  /**< Identifier for user tag 2 */
} iot_data_tag_t;

/** Opaque iot data structure */
typedef struct iot_data_t iot_data_t;

/**
* Type for data typecode structure
*/
typedef struct iot_typecode_t
{
  iot_data_type_t type;         /**< Core data type */
  iot_data_type_t element_type; /**< Element type for Array, Map, Vector and List */
  iot_data_type_t key_type;     /**< Key type for map */
} iot_typecode_t;

/**
* Type for simple data type static allocation
*/
typedef void * iot_data_static_t[2u + 16u / sizeof (void*)];

/**
* Type for list type static allocation
*/
typedef void * iot_data_list_static_t[4u + 8u / sizeof (void*)];

/**
* Macro to cast static data instance (iot_data_static_t) to a iot_data_t pointer
*/
#define IOT_DATA_STATIC(s) ((iot_data_t *) (s))

/** Type for set of data string constant values */
typedef struct iot_data_consts_t
{
  iot_data_static_t category; /**< constant "category" string */
  iot_data_static_t config;   /**< constant "config" string */
  iot_data_static_t name;     /**< constant "name" string */
  iot_data_static_t state;    /**< constant "state" string */
  iot_data_static_t type;     /**< constant "type" string */
} iot_data_consts_t;

/** Set of constant data string values */
extern iot_data_consts_t iot_data_consts;

/**
 * Type for data map iterator structure. Do not use struct members directly.
 */
typedef struct iot_data_map_iter_t
{
  const struct iot_data_map_t * _map; /**< Pointer to data map structure */
  struct iot_node_t * _node;          /**< Pointer to map node structure */
  uint32_t _count;                    /**< Position counter */
} iot_data_map_iter_t;

/**
 * Type for data vector iterator structure. Do not use struct members directly.
 */
typedef struct iot_data_vector_iter_t
{
  const struct iot_data_vector_t * _vector; /**< Pointer to data vector structure */
  uint32_t _index;                          /**< Index of the given vector */
} iot_data_vector_iter_t;

/**
 * Type for data list iterator structure. Do not use struct members directly.
 */
typedef struct iot_data_list_iter_t
{
  const struct iot_data_list_t * _list; /**< Pointer to data list structure */
  struct iot_element_t * _element;      /**< Pointer to list element structure */
} iot_data_list_iter_t;

/**
 * Alias for data array iterator structure. Do not use struct members directly.
 */
typedef struct iot_data_array_iter_t
{
  const struct iot_data_array_t * _array; /**< Pointer to data array structure */
  uint32_t _index;                        /**< Index of the given data array */
} iot_data_array_iter_t;

/**
 * Alias for generic iterator structure. Do not use struct members directly.
 */
typedef struct iot_data_iter_t
{
  iot_data_type_t _type; /**< Iterator type */
  union
  {
    iot_data_vector_iter_t vector;
    iot_data_list_iter_t list;
    iot_data_map_iter_t map;
  } _iter; /**< Iterator */
} iot_data_iter_t;

/** Type for data comparison function pointer */
typedef bool (*iot_data_cmp_fn) (const iot_data_t * data, const void * arg);

/** Type for data free function pointer */
typedef void (*iot_data_free_fn) (void * ptr);

/** Type for data update function pointer */
typedef iot_data_t * (*iot_data_update_fn) (const iot_data_t * data, void * arg);

/** Function to compare string data with a string value */
extern iot_data_cmp_fn iot_data_string_cmp;

/**
 * @brief Set on a per thread basis allocation policy for data (cache or heap)
 *
 * @param set   Whether to allocate from heap
 * @return      Previous allocation policy
 */
extern bool iot_data_alloc_heap (bool set);

/**
 * @brief Return the hash of a String, Array or Binary data type
 *
 * @param data  The data (can be NULL)
 * @return      The hash value or zero if data is NULL
 */
extern uint32_t iot_data_hash (const iot_data_t * data);

/**
 * @brief Compress a composed data type (Vector, List or Map) by eliminating duplicate data values
 *
 * @param data  The data to be compressed.
 */
extern void iot_data_compress (iot_data_t * data);

/**
 * @brief Compress a composed data type (Vector, List or Map) by eliminating duplicate data values
 *        using a supplied Map cache
 *
 * @param data  The data to be compressed.
 * @param cache The map used to eliminate duplicate values (must be a generic map with key type IOT_DATA_MULTI)
 */
extern void iot_data_compress_with_cache (iot_data_t * data, iot_data_t * cache);

/**
 * @brief Increment the data reference count
 * @param data  Pointer to data
 * @return      Returned pointer to data
 */
extern iot_data_t * iot_data_add_ref (const iot_data_t * data);

/**
 * @brief Return the data reference count
 * @param data  Pointer to data
 * @return      Returned pointer to data
 */
extern uint32_t iot_data_ref_count (const iot_data_t * data);

/**
 * @brief Free memory allocated to data
 *
 * The function to decrement reference count by 1 and free the allocated memory only if the reference count is <= 1
 *
 * @param data  Pointer to data (can be NULL)
 */
extern void iot_data_free (iot_data_t * data);

/**
 * @brief Get core data type
 *
 * The function to return the basic data type for the data
 *
 * @param data  Pointer to data (can be NULL)
 * @return      Data type enumeration. Returns IOT_DATA_INVALID if data pointer is NULL
 */
extern iot_data_type_t iot_data_type (const iot_data_t * data);

/**
 * @brief Check if data instance is of a given type
 *
 * The function to returns whether a data instance is of the given type
 *
 * @param data  Pointer to data (can be NULL)
 * @param type  The data type
 * @return      Whether the data is of the given type. Returns false is data is NULL.
 */
extern bool iot_data_is_of_type (const iot_data_t * data, iot_data_type_t type);

/**
 * @brief Check if data instance is static (const)
 *
 * The function to returns whether a data instance is static
 *
 * @param data  Pointer to data (can be NULL)
 * @return      Whether the data is statically allocated
 */
extern bool iot_data_is_static (const iot_data_t * data);

/**
 * @brief Check if data instance has a value of NaN.
 * 
 * Floating point values can have a value of NaN. If the data instance is a 
 * floating point type, then check if the value is NaN.
 * 
 * @param data Pointer to data
 * @return     Whether the data is of the value NaN
 */
extern bool iot_data_is_nan (const iot_data_t * data);

/**
 * @brief Get data type code
 *
 * Get the type code of data
 *
 * @param data  Pointer to data
 * @param tc    Pointer to type code to be set
 */
extern void iot_data_typecode (const iot_data_t * data, iot_typecode_t * tc);

/**
 * @brief Return data_type for the type name
 *
 * The function to return data_type for the typename
 *
 * @param name   Datatype string
 * @return       Datatype from the enum defined in iot_data_type_t
 */
extern iot_data_type_t iot_data_name_type (const char * name);

/**
 * @brief Return typename corresponding to data
 *
 * The function to return the typename corresponding to data
 *
 * @param  data  Pointer to data
 * @return       Data type string
 */
extern const char * iot_data_type_name (const iot_data_t * data);

/**
 * @brief Return string name for a data type
 *
 * The function to return the name of a data type
 *
 * @param  type  Data type
 * @return       Data type string
 */
extern const char * iot_data_type_string (iot_data_type_t type);

/**
 * @brief Set metadata for data
 *
 * Set metadata on a data item. Meta data is freed when the containing data is freed.
 * This function should not be used with null or boolean data types as these are implemented as shared
 * constant instances.
 *
 * @param  data      Pointer to data
 * @param  metadata  Pointer to metadata
 * @param  key       Pointer to metadata key
 */
extern void iot_data_set_metadata (iot_data_t * data, iot_data_t * metadata, const iot_data_t * key);

/**
 * @brief Get metadata for data
 *
 * Gets the metadata associated with a data item.
 *
 * @param  data  Pointer to data
 * @param  key   Pointer to metadata key
 * @return       Pointer to metadata (NULL if not found)
 */
extern const iot_data_t * iot_data_get_metadata (const iot_data_t * data, const iot_data_t * key);

/**
 * @brief Return address allocated for data
 *
 * The function to return the address allocated for data
 *
 * @param data  Pointer to data
 * @return      Pointer that holds the address of data
 */
extern const void * iot_data_address (const iot_data_t * data); /* Not for Maps, Vectors or Lists */

/**
 * @brief Allocate memory for data_type int8
 *
 * The function to allocate memory for data_type int8
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_i8 (int8_t val);

/**
 * @brief Allocate memory for data_type uint8
 *
 * The function to allocate memory for data_type uint8
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_ui8 (uint8_t val);

/**
 * @brief Allocate memory for data_type int16
 *
 * The function to allocate memory for data_type int16
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_i16 (int16_t val);

/**
 * @brief Allocate memory for data_type uint16
 *
 * The function to allocate memory for data_type uint16
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_ui16 (uint16_t val);

/**
 * @brief Allocate memory for data_type int32
 *
 * The function to allocate memory for data_type int32
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_i32 (int32_t val);

/**
 * @brief Allocate memory for data_type uint32
 *
 * The function to allocate memory for data_type uint32
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_ui32 (uint32_t val);

/**
 * @brief Allocate memory for data_type int64
 *
 * The function to allocate memory for data_type int64
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_i64 (int64_t val);

/**
 * @brief Allocate memory for data_type uint64
 *
 * The function to allocate memory for data_type uint64
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_ui64 (uint64_t val);

/**
 * @brief Allocate constant uint64 data
 *
 * The function to allocate data for a constant uint64, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        uint64 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_ui64 (iot_data_static_t * data, uint64_t val);

/**
 * @brief Allocate constant int64 data
 *
 * The function to allocate data for a constant int64, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        int64 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_i64 (iot_data_static_t * data, int64_t val);

/**
 * @brief Allocate constant uint32 data
 *
 * The function to allocate data for a constant uint32, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        uint32 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_ui32 (iot_data_static_t * data, uint32_t val);

/**
 * @brief Allocate constant int32 data
 *
 * The function to allocate data for a constant int32, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        int32 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_i32 (iot_data_static_t * data, int32_t val);

/**
 * @brief Allocate constant uint16 data
 *
 * The function to allocate data for a constant uint16, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        uint16 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_ui16 (iot_data_static_t * data, uint16_t val);

/**
 * @brief Allocate constant int16 data
 *
 * The function to allocate data for a constant int16, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        int16 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_i16 (iot_data_static_t * data, int16_t val);

/**
 * @brief Allocate constant uint8 data
 *
 * The function to allocate data for a constant uint8, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        uint8 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_ui8 (iot_data_static_t * data, uint8_t val);

/**
 * @brief Allocate constant int8 data
 *
 * The function to allocate data for a constant uint8, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        int8 value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_i8 (iot_data_static_t * data, int8_t val);

/**
 * @brief Allocate constant float data
 *
 * The function to allocate data for a constant float, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        float value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_f32 (iot_data_static_t * data, float val);

/**
 * @brief Allocate constant double data
 *
 * The function to allocate data for a constant double, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param val        double value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_f64 (iot_data_static_t * data, double val);

/**
 * @brief Allocate memory for data_type float
 *
 * The function to allocate memory for data_type float
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_f32 (float val);

/**
 * @brief Allocate memory for data_type double
 *
 * The function to allocate memory for data_type double
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_f64 (double val);

/**
 * @brief Allocate memory for data_type bool
 *
 * The function to allocate memory for data_type bool
 *
 * @param val  Datavalue
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_bool (bool val);

/**
 * @brief Allocate data for data_type null
 *
 * The function to allocate memory for data_type null
 *
 * @return     Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_null (void);

/**
 * @brief Allocate data for a string containing a UUID
 *
 * @return  Pointer to the allocated data
 */
extern iot_data_t * iot_data_alloc_uuid_string (void);

/**
 * @brief Allocate data for a binary UUID of type UINT8 and length 16
 *
 * @return  Pointer to the allocated data
 */
extern iot_data_t * iot_data_alloc_uuid (void);

/**
 * @brief Allocate memory for a string
 *
 * The function to allocate data for a string
 *
 * @param val        String
 * @param ownership  If the ownership is set to IOT_DATA_COPY, input val is duplicated with the address allocated, else
 *                   the address passed is stored
 * @return           Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_string (const char * val, iot_data_ownership_t ownership);

/**
 * @brief Allocate constant string data
 *
 * The function to allocate data for a constant string, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param str        Constant string value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_string (iot_data_static_t * data, const char * str);

/**
 * @brief Allocate memory for a formatted string
 *
 * The function to allocate data for a formatted string
 *
 * @param format  String with formatting directives
 * @param ...     Arguments for formatting directives
 *
 * @return        Pointer to the allocated memory
 */

extern iot_data_t * iot_data_alloc_string_fmt (const char * format, ...)  __attribute__ ((format (printf, 1, 2)));

/**
 * @brief Allocate data for a pointer, with associated free function
 *
 * The function to allocate data for a pointer
 *
 * @param ptr        Opaque pointer
 * @param free_fn    Function to free pointer (can be NULL)
 * @return           Pointer to the allocated data
 */
extern iot_data_t * iot_data_alloc_pointer (void * ptr, iot_data_free_fn free_fn);

/**
 * @brief Allocate constant pointer data
 *
 * The function to allocate data for a constant pointer, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @param ptr        Constant pointer value.
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_pointer (iot_data_static_t * data, const void * ptr);

/**
 * @brief Allocate data for a list
 *
 * The function to allocate data for a list
 *
 * @return  Pointer to the allocated data list
 */
extern iot_data_t * iot_data_alloc_list (void);

/**
 * @brief Allocate constant list data
 *
 * The function to allocate data for a constant list, using fixed static storage, so need
 * not be deleted.
 *
 * @param data       Address of static storage for data
 * @return           Pointer to the allocated data (same address as the static storage)
 */
extern iot_data_t * iot_data_alloc_const_list (iot_data_list_static_t * data);

/**
 * @brief Allocate data for a list
 *
 * The function to allocate data for a list of fixed element type
 *
 * @param  element_type Datatype of the list elements
 * @return              Pointer to the allocated data list
 */
extern iot_data_t * iot_data_alloc_typed_list (iot_data_type_t element_type);

/**
 * @brief Find list element type
 *
 * Function to return the element type of a list.
 *
 * @param list      List
 * @return          Type of list elements
 */
extern iot_data_type_t iot_data_list_type (const iot_data_t * list);

/**
 * @brief Get the list length
 *
 * The function to get the length of the input list
 *
 * @param list  Input list
 * @return      Length of the list
 */
extern uint32_t iot_data_list_length (const iot_data_t * list);

/**
 * @brief Empty a list of all elements
 *
 * The function to empty a list of all elements
 *
 * @param list  Input list
 */
extern void iot_data_list_empty (iot_data_t * list);

/**
 * @brief Find matching element in a list using compare function
 *
 * Applies a compare function to each element in a list until the compare
 * function returns true or the end of the list is reached. The list is searched from tail to head.
 *
 * @param list    Input list
 * @param cmp     A comparison function which takes an element and an argument and returns true or false
 * @param arg     Pointer to user supplied argument that is passed to the comparison function.
 * @return        Pointer to the first element for which the comparison function return true, NULL otherwise
 */
extern const iot_data_t * iot_data_list_find (const iot_data_t * list, iot_data_cmp_fn cmp, const void * arg);

/**
 * @brief Remove matching element in a list using compare function
 *
 * Applies a compare function to each element in a list until the compare
 * function returns true or the end of the list is reached. The list is searched from tail to head,
 * and the first matching element is removed and freed.
 *
 * @param list    Input list
 * @param cmp     A comparison function which takes an element and an argument and returns true or false
 * @param arg     Pointer to user supplied argument that is passed to the comparison function.
 * @return        Whether an element removed
 */
extern bool iot_data_list_remove (iot_data_t * list, iot_data_cmp_fn cmp, const void * arg);

/**
 * @brief Associate a list iterator with a list
 *
 * The function initialises an list iterator asociating it with a list. Note that
 * the iterator is unsafe in that the list cannot be modified when being iterated, other
 * than by using the iot_data_list_iter_replace function. The iterator can be moved
 * from tail to head using the next function or from haed to tail using the prev function.
 *
 *
 * @param list List
 * @param iter List iterator
 */
extern void iot_data_list_iter (const iot_data_t * list, iot_data_list_iter_t * iter);

/**
 * @brief Update the iterator to point to the next element in a list
 *
 * The function to set the iterator to point to the next element of a list, moving from tail to head.
 * On reaching the head of the list, the iterator is set to the tail of the list.
 *
 * @param iter  Input iterator
 * @return      Returns whether the iterator is still valid (has not passed the head of the list)
 */
extern bool iot_data_list_iter_next (iot_data_list_iter_t * iter);

/**
 * @brief Returns whether a list iterator has a next element
 *
 * The function returns whether the iterator next function will return a value
 *
 * @param iter  Input iterator
 * @return      Whether the iterator has a next element
 */
extern bool iot_data_list_iter_has_next (const iot_data_list_iter_t * iter);

/**
 * @brief Update the iterator to point to the previous element in a list
 *
 * The function to set the iterator to point to the previous element of a list, moving from head to tail.
 * On reaching the tail of the list, the iterator is set to the head of the list.
 *
 * @param iter  Input iterator
 * @return      Returns whether the iterator is still valid (has not passed the tail of the list)
 */
extern bool iot_data_list_iter_prev (iot_data_list_iter_t * iter);

/**
 * @brief Get the value of the element associated with a list iterator
 *
 * The function returns the list element referenced by the list iterator.
 *
 * @param iter  Input iterator
 * @return      Pointer to the current list element
 */
extern const iot_data_t * iot_data_list_iter_value (const iot_data_list_iter_t * iter);

/**
 * @brief Get the string value from the list referenced by an iterator
 *
 * Get string type value from the list referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      String type value if iter is valid, NULL otherwise
 */
extern const char * iot_data_list_iter_string_value (const iot_data_list_iter_t * iter);

/**
 * @brief Get pointer value from the list referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      Pointer type value from the list if iter is valid, NULL otherwise
 */
extern const void * iot_data_list_iter_pointer_value (const iot_data_list_iter_t * iter);

/**
 * @brief Remove the data associated with a list iterator. The list iterator is set to
 * the previous list element.
 *
 * @param iter  List iterator
 * @return      Returns whether iterator was associated with a data value
 */
extern bool iot_data_list_iter_remove (iot_data_list_iter_t * iter);

/**
 * @brief Replace the value associated with a list iterator
 *
 * The function to replace a value in a list referenced by the iterator
 *
 * @param iter  Input iterator
 * @param value New value to store in the list
 * @return      Pointer to the previous value associated with the iterator if valid, NULL otherwise
 */
extern iot_data_t * iot_data_list_iter_replace (const iot_data_list_iter_t * iter, iot_data_t * value);

/**
 * @brief Push a value onto the tail of a list
 *
 * The function to push a value onto the tail of a list
 *
 * @param list  Input list
 * @param value Value to add to tail of the list
 */
extern void iot_data_list_tail_push (iot_data_t * list, iot_data_t * value);

/**
 * @brief Pop a value from the tail of a list
 *
 * The function to pop a value from the tail of a list
 *
 * @param list  Input list
 * @return      Value from the list tail or NULL if list empty
 */
extern iot_data_t * iot_data_list_tail_pop (iot_data_t * list);

/**
 * @brief Push a value onto the head of a list
 *
 * The function to push a value onto the head of a list
 *
 * @param list  Input list
 * @param value Value to add to head of the list
 */
extern void iot_data_list_head_push (iot_data_t * list, iot_data_t * value);

/**
 * @brief Pop a value from the head of a list
 *
 * The function to pop a value from the head of a list
 *
 * @param list  Input list
 * @return      Value from the list head or NULL if list empty
 */
extern iot_data_t * iot_data_list_head_pop (iot_data_t * list);

/**
 * @brief Allocate data for a binary array
 *
 * The function to allocate data for a binary array of given size (in bytes), Binaries can be used as arrays.
 *
 * @param data       Pointer to an array of bytes
 * @param length     Lenngth of the byte array
 * @param ownership  If the ownership is set to IOT_DATA_COPY, a new allocation is made and data is copied to the allocated
 *                   memory, else the ownership of the data is taken.
 * @return           Pointer to the allocated data
 */
extern iot_data_t * iot_data_alloc_binary (void * data, uint32_t length, iot_data_ownership_t ownership);

/**
 * @brief Take buffer from a Binary or an Array. If internal buffer cannot be taken (is referenced or shared) a copy is returned
 * @param data       Pointer to the binary or array
 * @param len        Pointer to the returned buffer length
 * @return           Binary or Array data buffer
 */
extern void * iot_data_binary_take (iot_data_t * data, uint32_t * len);

/**
 * @brief Convert an array to a binary
 * @param data       Pointer to the array to be converted
 */
extern void iot_data_binary_to_array (iot_data_t * data);

/**
 * @brief Create binary from string data (NULL string terminator is ignored)
 * @param data       Pointer to the string data
 */
extern iot_data_t * iot_data_binary_from_string (const iot_data_t * data);

/**
 * @brief Create string from binary data
 * @param data       Pointer to the binary data
 */
extern iot_data_t * iot_data_string_from_binary (const iot_data_t * data);

/**
 * @brief Convert a binary to an array
 * @param data       Pointer to the binary to be converted
 */
extern void iot_data_array_to_binary (iot_data_t * data);

/**
 * @brief Allocate memory for an array
 *
 * The function to allocate memory for an array of given size and type. Note that only basic C integer, boolean and floating
 * point types are supported (not string or composed types).
 *
 * @param data       Pointer to C array of data
 * @param length     Number of elements in the array
 * @param type       Type of array element
 * @param ownership  If the ownership is set to IOT_DATA_COPY, a new allocation is made and data is copied to the allocated
 *                   memory, else the ownership of the data is taken.
 * @return           Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_array (void * data, uint32_t length, iot_data_type_t type, iot_data_ownership_t ownership);

/**
 * @brief Find array element type
 *
 * Function to return the element type of an Array.
 *
 * @param array      Array
 * @return           Type of array elements
 */
extern iot_data_type_t iot_data_array_type (const iot_data_t * array);

/**
 * @brief Check if array instance is of a given type
 *
 * The function to returns whether an array instance is of the given type
 *
 * @param array  Pointer to array (can be NULL)
 * @param type  The array content data type
 * @return      Whether the array content is of the given type. Returns false is array is NULL.
 */
extern bool iot_data_array_is_of_type (const iot_data_t * array, iot_data_type_t type);

/**
 * @brief Find the number of Array elements
 *
 * Function to return the number of elements in an Array
 *
 * @param array      Array
 * @return           Type of array data
 */
extern uint32_t iot_data_array_length (const iot_data_t * array);

/**
 * @brief Find the array size
 *
 * Function to return the size (in bytes) of the contained array
 *
 * @param array      Array
 * @return           Size of contained array
 */
extern uint32_t iot_data_array_size (const iot_data_t * array);

/**
 * @brief  Allocate map data type
 *
 * The function to allocate a data map with a key type
 *
 * @param key_type  Datatype of the map keys
 * @return          Pointer to the allocated data map
 */
extern iot_data_t * iot_data_alloc_map (iot_data_type_t key_type);

/**
 * @brief  Allocate map data type
 *
 * The function to allocate a data map with a key type and element type
 *
 * @param key_type     Datatype of the map keys
 * @param element_type Datatype of the map values
 * @return             Pointer to the allocated data map
 */
extern iot_data_t * iot_data_alloc_typed_map (iot_data_type_t key_type, iot_data_type_t element_type);

/**
 * @brief Find map element type
 *
 * Function to return the element type of a map
 *
 * @param map    Map
 * @return       Type of map elements
 */
extern iot_data_type_t iot_data_map_type (const iot_data_t * map);

/**
 * @brief Allocate a data vector
 *
 * The function to allocate memory for an vector type
 *
 * @param size  Length of the vector for allocation, could be zero to create zero length vector
 * @return      Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_vector (uint32_t size);

/**
 * @brief Allocate a data vector
 *
 * The function to allocate a data vector of fixed element type
 *
 * @param size         Length of the vector for allocation, could be zero to create zero length vector
 * @param element_type Datatype of the vector elements
 * @return             Pointer to the allocated data vector
 */
extern iot_data_t * iot_data_alloc_typed_vector (uint32_t size, iot_data_type_t element_type);

/**
 * @brief Find vector element type
 *
 * Function to return the element type of a vector
 *
 * @param vector    Vector
 * @return          Type of vector elements
 */
extern iot_data_type_t iot_data_vector_type (const iot_data_t * vector);
/**
 * @brief Allocate memory of data_type type for a string value
 *
 * The function to allocate memory of data_type type for a string value
 *
 * @param type   Datatype for memory allocation
 * @param value  String value
 * @return       Pointer to the allocated memory for valid type, NULL on error
 *
 */
extern iot_data_t * iot_data_alloc_from_string (iot_data_type_t type, const char * value);

/**
 * @brief Allocate memory of data_type that correspond to typename for a string value
 *
 * The function to allocate memory of data_type that correspond to typename for a string value
 *
 * @param type   Typename of the data_type
 * @param value  String value
 * @return       Pointer to the allocated memory for valid type, NULL on error
 */
extern iot_data_t * iot_data_alloc_from_strings (const char * type, const char * value);

/**
 * @brief Allocate memory for base64 decoded data
 *
 * The function allocates a byte array for base64 decoded data
 *
 * @param value  Data to decode from base64
 * @return       Pointer to the allocated memory of byte array, NULL on error
 */
extern iot_data_t * iot_data_alloc_array_from_base64 (const char * value);

/**
 * @brief Get the value of the element at the given index
 *
 * This function returns the address of the array element at the given index
 *
 * @param array  Input array
 * @param index  Index in array
 * @return       Pointer to the element at the given index in the array
 */
void * iot_data_array_get (const iot_data_t * array, uint32_t index);

/**
 * @brief Increment data value by 1
 *
 * The function to increment the value by 1
 *
 * @param data  Value to increment. Applicable for integer and float data types
 */
extern void iot_data_increment (iot_data_t * data);

/**
 * @brief Decrement data value by 1
 *
 * The function to decrement the value by 1
 *
 * @param data  Value to decrement. Applicable for integer and float data types
 */
extern void iot_data_decrement (iot_data_t * data);

/**
 * @brief Get value of type i8 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern int8_t iot_data_i8 (const iot_data_t * data);

/**
 * @brief Get value of type ui8 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern uint8_t iot_data_ui8 (const iot_data_t * data);

/**
 * @brief Get value of type int16 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern int16_t iot_data_i16 (const iot_data_t * data);

/**
 * @brief Get value of type uint16 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern uint16_t iot_data_ui16 (const iot_data_t * data);

/**
 * @brief Get value of type int32 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern int32_t iot_data_i32 (const iot_data_t * data);

/**
 * @brief Get value of type ui32 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern uint32_t iot_data_ui32 (const iot_data_t * data);

/**
 * @brief Get value of type i64 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern int64_t iot_data_i64 (const iot_data_t * data);

/**
 * @brief Get value of type uint64 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern uint64_t iot_data_ui64 (const iot_data_t * data);

/**
 * @brief Get value of type float stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern float iot_data_f32 (const iot_data_t * data);

/**
 * @brief Get value of type double stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern double iot_data_f64 (const iot_data_t * data);

/**
 * @brief Get value of type bool stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern bool iot_data_bool (const iot_data_t * data);

/**
 * @brief Get value of type string stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Returned string pointer or NULL if date not of type string
 */
extern const char * iot_data_string (const iot_data_t * data);

/**
 * @brief Get value pointer stored in data
 *
 * The function to get the pointer value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Returned pointer or NULL if date not of type POINTER
 */
extern const void * iot_data_pointer (const iot_data_t * data);

/**
 * @brief Cast integer, float or boolean values
 *
 * The function returns a data value cast to a given type.
 * False is returned if type conversion is not possible.
 *
 * @param data  Data to be converted
 * @param type  Type of data value to be returned
 * @param val   Pointer to returned value, set if data type can be converted
 * @return      Whether data type could be converted
 */
extern bool iot_data_cast (const iot_data_t * data, iot_data_type_t type, void * val);

/**
 * @brief Add key-value pair to a map
 *
 * The function to add a key-value pair to a map
 *
 * @param map  Map to add a new key-value pair
 * @param key  Input key
 * @param val  Input value
 * Note: The ownership of key and value passed is owned by the map and cannot be reused, unless reference counted
 */
extern void iot_data_map_add (iot_data_t * map, iot_data_t * key, iot_data_t * val);

/**
 * @brief Add key-value pair to a map if key not already in use
 *
 * The function to add a key-value pair to a map if key not already in use
 *
 * @param map  Map to add a new key-value pair
 * @param key  Input key
 * @param val  Input value
 * @return     Whether key-value pair has been added
 * Note: The ownership of key and value passed is owned by the map and cannot be reused, unless reference counted
 */
extern bool iot_data_map_add_unused (iot_data_t * map, iot_data_t * key, iot_data_t * val);

/**
 * @brief Merge the contents of one map into another. Key types must be compatible.
 *
 * @param map  Map to which new map contents are added
 * @param add  Map containing contents to be merged, can be NULL
 */
extern void iot_data_map_merge (iot_data_t * map, const iot_data_t * add);

/**
 * @brief Remove a value by key from a map
 *
 * The function to remove a key-value pair from a map by key
 *
 * @param map  Map to remove a key-value pair
 * @param key  Key. Can be NULL (function will return false)
 * @return     Whether key-value pair has been removed
 */
extern bool iot_data_map_remove (iot_data_t * map, const iot_data_t * key);

/**
 * @brief Get the size of a map
 *
 * The function to get the map size
 *
 * @param map Input map
 * @return    Size of the map
 */
extern uint32_t iot_data_map_size (const iot_data_t * map);

/**
 * @brief Empty a map of all elements
 *
 * Function to empty a map of all elements
 *
 * @param map  Input map
 */
extern void iot_data_map_empty (iot_data_t * map);

/**
 * @brief Add key-value pair to a map
 *
 * The function to add a key-value pair to a map where a key is of string type
 *
 * @param map  Map to add a new key-value pair
 * @param key  String key. The key is allocated using IOT_DATA_REF and if the key is not a string constant the key string must not be changed or freed while the key used by the map. Use iot_data_map_add to add a key where this cannot be guaranteed.
 * @param val  Input value
 * Note: The ownership of value passed is owned by the map and cannot be reused, unless reference counted.
 */
extern void iot_data_string_map_add (iot_data_t * map, const char * key, iot_data_t * val);

/**
 * @brief Remove a value by string key from a map
 *
 * The function to remove a key-value pair from a map where a key is of string type
 *
 * @param map  Map to remove a key-value pair
 * @param key  String key. Can be NULL (function will return false)
 * @return     Whether key-value pair has been removed
 */
extern bool iot_data_string_map_remove (iot_data_t * map, const char * key);

/**
 * @brief  Get value from the map for a key provided
 *
 * The function to get the value corresponding to an input key from the map
 *
 * @param map  Map to get the value
 * @param key  Input key
 * @return     Pointer to a value corresponding to the key of type iot_data
 */
extern const iot_data_t * iot_data_map_get (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief  Get value from the map for a key provided. Returns NULL if cannot be found or of wring type.
 *
 * The function to get the value corresponding to an input key from the map
 *
 * @param map  Map to get the value
 * @param key  Input key
 * @param type Return data type
 * @return     Pointer to a value corresponding to the key or NULL if cannot be found or of wrong type
 */
extern const iot_data_t * iot_data_map_get_typed (const iot_data_t * map, const iot_data_t * key, iot_data_type_t type);

/**
 * @brief Get string value corresponding to key from a map
 *
 * Function to get a string value from a map
 *
 * @param map  Map from which get a value
 * @param key  Key for the value
 * @return     String value corresponding to the key, or NULL if not found
 */
extern const char * iot_data_map_get_string (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief Get numeric value corresponding to a key from a map.
 *
 * Function to get a numeric value from a map
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param type         Data type to return (an integer or floating point type)
 * @param val          Pointer to the numeric type to be set
 * @return             Returns if value could be found in map and cast to required type
 */
extern bool iot_data_map_get_number (const iot_data_t * map, const iot_data_t * key, iot_data_type_t type, void * val);

/**
 * @brief Get integer value corresponding to a key from a map.
 *
 * Function to get a numeric value from a map
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param val          Pointer to the int to be set
 * @return             Returns if value could be found in map and cast to an int
 */
extern bool iot_data_map_get_int (const iot_data_t * map, const iot_data_t * key, int * val);

/**
 * @brief Get int64_t value corresponding to key from a map
 *
 * Function to get a string value from a map
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param default_val  Default int64 value
 * @return             int64_t value corresponding to the key, or default_val if not found
 */
extern int64_t iot_data_map_get_i64 (const iot_data_t * map, const iot_data_t * key, int64_t default_val);

/**
 * @brief Get uint64_t value corresponding to key from a map
 *
 * Function to get a string value from a map
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param default_val  Default int64 value
 * @return             int64_t value corresponding to the key, or default_val if not found
 */
extern uint64_t iot_data_map_get_ui64 (const iot_data_t * map, const iot_data_t * key, uint64_t default_val);

/**
 * @brief Get bool value corresponding to key from a map
 *
 * Get bool value corresponding to key from the map, if the value type is IOT_DATA_BOOL, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param default_val  Default bool value
 * @return             Bool value corresponding to the key, or default_val if not found
 */
extern bool iot_data_map_get_bool (const iot_data_t * map, const iot_data_t * key, bool default_val);

/**
 * @brief Get double value corresponding to key from a map
 *
 * The function to get double value corresponding to key from the map, if the value type is IOT_DATA_FLOAT64, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param default_val  Default double value
 * @return             Double value corresponding to the key, else default_val
 */
extern double iot_data_map_get_f64 (const iot_data_t * map, const iot_data_t * key, double default_val);

/**
 * @brief Get vector corresponding to key from a map
 *
 * The function to get a vector corresponding to key from the map, if the type value is not IOT_DATA_VECTOR then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @return             Vector found by key in the map. NULL if not found or not a vector.
 */
extern const iot_data_t * iot_data_map_get_vector (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief Get map corresponding to key from a string map
 *
 * The function to get a map corresponding to key from the map, if the type value is not IOT_DATA_MAP then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @return             Map found by key in the map. NULL if not found or not a Map
 */
extern const iot_data_t * iot_data_map_get_map (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief Get array corresponding to key from a string map
 *
 * The function to get an array corresponding to key from the map, if the type value is not IOT_DATA_ARRAY then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @return             Array found by key in the map. NULL if not found or not an array
 */
extern const iot_data_t * iot_data_map_get_array (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief Get list corresponding to key from a map
 *
 * The function to get a list corresponding to key from the map, if the type value is not IOT_DATA_LIST then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @return             List found by key in the map. NULL if not found or not a list
 */
extern const iot_data_t * iot_data_map_get_list (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief Get pointer corresponding to key from a map
 *
 * The function to get a pointer corresponding to key from the map, if the type value is not IOT_DATA_POINTER then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @return             Pointer found by key in the map. NULL if not found or not a pointer
 */
extern const void * iot_data_map_get_pointer (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief  Get value from the map for a key provided
 *
 * The function to get the value from the map for an input key provided as a string
 *
 * @param map  Map to get the value
 * @param key  Input key of type String
 * @return     Pointer to a value corresponding to the key of type iot_data
 */
extern const iot_data_t * iot_data_string_map_get (const iot_data_t * map, const char * key);

/**
 * @brief Get string value corresponding to key from a map
 *
 * The function to get the string value from the map for an input key provided as a string
 *
 * @param map  Map from which get a value
 * @param key  String key for the value
 * @return     String value corresponding to the key, or NULL if not found
 */
extern const char * iot_data_string_map_get_string (const iot_data_t * map, const char * key);

/**
 * @brief Get numeric value corresponding to a string key from a map.
 *
 * Function to get a numeric value from a map
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param type         Data type to return (an integer or floating point type)
 * @param val          Pointer to the numeric type to be set
 * @return             Returns if value could be found in map and cast to required type
 */
extern bool iot_data_string_map_get_number (const iot_data_t * map, const char * key, iot_data_type_t type, void * val);

/**
 * @brief Get integer value corresponding to a string key from a map.
 *
 * Function to get a numeric value from a map
 *
 * @param map          Map from which get a value
 * @param key          Key for the value
 * @param val          Pointer to the int to be set
 * @return             Returns if value could be found in map and cast to an int
 */
extern bool iot_data_string_map_get_int (const iot_data_t * map, const char * key, int * val);

/**
 * @brief Get int64_t value corresponding to a string key from a map
 *
 * The function to get a int64 value corresponding to key from the map, if the value type is IOT_DATA_INT64, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @param default_val  Default int64 value
 * @return             Int64_t value corresponding to the key, else default_val
 */
extern int64_t iot_data_string_map_get_i64 (const iot_data_t * map, const char * key, int64_t default_val);

/**
 * @brief Get uint64_t value corresponding to a string key from a map
 *
 * The function to get a uint64 value corresponding to key from the map, if the value type is IOT_DATA_UINT64, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @param default_val  Default int64 value
 * @return             Uint64_t value corresponding to the key, else default_val
 */
extern uint64_t iot_data_string_map_get_ui64 (const iot_data_t * map, const char * key, uint64_t default_val);


/**
 * @brief Get bool value corresponding to key from a map
 *
 * The function to get bool value corresponding to key from the map, if the value type is IOT_DATA_BOOL, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          String key for the
 * @param default_val  Default bool value
 * @return             Bool value corresponding to the key, else default_val
 */
extern bool iot_data_string_map_get_bool (const iot_data_t * map, const char * key, bool default_val);

/**
 * @brief Get double value corresponding to key from a map
 *
 * The function to get double value corresponding to key from the map, if the value type is IOT_DATA_FLOAT64, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @param default_val  Default double value
 * @return             Double value corresponding to the key, else default_val
 */
extern double iot_data_string_map_get_f64 (const iot_data_t * map, const char * key, double default_val);

/**
 * @brief Get vector corresponding to key from a string map
 *
 * The function to get a vector corresponding to key from the map, if the type value is not IOT_DATA_VECTOR then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @return             Vector found by key in the map. NULL if not found or element not a vector.
 */
extern const iot_data_t * iot_data_string_map_get_vector (const iot_data_t * map, const char * key);

/**
 * @brief Get map corresponding to key from a string map
 *
 * The function to get a map corresponding to key from the map, if the type value is not IOT_DATA_MAP then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @return             Map found by key in the map. NULL if not found or element not a map.
 */
extern const iot_data_t * iot_data_string_map_get_map (const iot_data_t * map, const char * key);

/**
 * @brief Get list corresponding to key from a string map
 *
 * The function to get a list corresponding to key from the map, if the type value is not IOT_DATA_LIST then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @return             List found by key in the map. NULL if not found or element not a list.
 */
extern const iot_data_t * iot_data_string_map_get_list (const iot_data_t * map, const char * key);

/**
 * @brief Get pointer corresponding to key from a string map
 *
 * The function to get a pointer corresponding to key from the map, if the type value is not IOT_DATA_POINTER then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @return             Pointer found by key in the map. NULL if not found or not a map.
 */
extern const void * iot_data_string_map_get_pointer (const iot_data_t * map, const char * key);

/**
 * @brief  Get the key type of map
 *
 * The function to get the key type of the input map
 *
 * @param map  Input map
 * @return     Data type of the map
 */
extern iot_data_type_t iot_data_map_key_type (const iot_data_t * map);

/**
 * @brief Check if map instance key is of a given type
 *
 * The function to returns whether a map instance key is of the given type
 *
 * @param map   Pointer to map (can be NULL)
 * @param type  The map key data type
 * @return      Whether the map key is of the given type. Returns false is map is NULL.
 */
extern bool iot_data_map_key_is_of_type (const iot_data_t * map, iot_data_type_t type);

/**
 * @brief  Decode base64 value and store it as a byte array in a map for a given key
 *
 * The function to decode base64 value in a map store it as a byte array for an input key
 *
 * @param map  Input map
 * @param key  Input key
 * @return     'true' on Success, 'false' on Failure
 */
extern bool iot_data_map_base64_to_array (const iot_data_t * map, const iot_data_t * key);

/**
 * @brief Add an element to an vector at index
 *
 * The function to add a value to an vector at index
 *
 * @param vector  Input vector to add an element
 * @param index  Index in an vector
 * @param val    Pointer to a value of type iot_data to add, can be NULL
 * Note: The ownership of the value passed is owned by the vector and cannot be reused, unless reference counted
 */
extern void iot_data_vector_add (iot_data_t * vector, uint32_t index, iot_data_t * val);

/**
 * @brief Get a value at an index from then vector
 *
 * Function to get a value from a vector at a given index
 *
 * @param vector Input vector
 * @param index  Vector index for value
 * @return       Pointer to a value at the index
 */
extern const iot_data_t * iot_data_vector_get (const iot_data_t * vector, uint32_t index);

/**
 * @brief Get pointer from value at a given vector index
 *
 * Function to get a pointer from a value at a vector index, if the index is invalid or value type is not IOT_DATA_POINTER then NULL is returned
 *
 * @param vector Input vector
 * @param index  Vector index for value
 * @return       Pointer from value at index (NULL if invalid index ot value not a pointer)
 */
extern const void * iot_data_vector_get_pointer (const iot_data_t * vector, uint32_t index);

/**
 * @brief Resize a vector
 *
 * Resize a vector. If the vector is reduced in size elements no longer
 * included in the vector are freed.
 *
 * @param vector  Input vector
 * @param size    new vector size
 */
extern void iot_data_vector_resize (iot_data_t * vector, uint32_t size);

/**
 * @brief Compact a vector
 *
 * Compact a vector. All NULL elements in a vector are removed and the vector is resized accordingly.
 *
 * @param vector  Input vector
 * @return        Size of the compacted vector
 */
extern uint32_t iot_data_vector_compact (iot_data_t * vector);

/**
 * @brief Get the vector size
 * @param vector  Input vector
 * @return       Size of an vector
 */
extern uint32_t iot_data_vector_size (const iot_data_t * vector);

/**
 * @brief Get the number of elements of a given type in a vector
 * @param vector  Input vector
 * @param type    Type of elements to count (IOT_DATA_MULTI to count all types)
 * @param recurse Whether to recurse to contained vectors or count them as an element
 * @return        Number of contained elements of given type
 */
extern uint32_t iot_data_vector_element_count (const iot_data_t * vector, iot_data_type_t type, bool recurse);

/**
 * @brief Initialise iterator for an array
 *
 * The function initialises an iterator to point to the beginning of an array. Note that
 * the iterator is unsafe in that the array cannot be modified when being iterated.
 *
 * @param array   Input array
 * @param iter  Iterator to initialise
 */
extern void iot_data_array_iter (const iot_data_t * array, iot_data_array_iter_t * iter);

/**
 * @brief Update the iterator to point to the next element within an array
 *
 * The function to set the iterator to point to the next element of an array. On reaching end of the array,
 * the iterator is set to point to the first element in the array.
 *
 * @param iter  Input iterator
 * @return      Returns whether the iterator is still valid (has not passed end of the array)
 */
extern bool iot_data_array_iter_next (iot_data_array_iter_t * iter);

/**
 * @brief Returns whether an array iterator has a next element
 *
 * The function returns whether the iterator next function will return a value
 *
 * @param iter  Input iterator
 * @return      Whether the iterator has a next element
 */
extern bool iot_data_array_iter_has_next (const iot_data_array_iter_t * iter);
/**
 * @brief Update the iterator to point to the previous element within an array
 *
 * The function to set the iterator to point to the previous element of an array. On reaching end of the array,
 * the iterator is set to point to the last element in the array.
 *
 * @param iter  Input iterator
 * @return      Returns whether the iterator is still valid (has not passed start of the array)
 */
extern bool iot_data_array_iter_prev (iot_data_array_iter_t * iter);

/**
 * @brief Return the current array index of an array iterator
 *
 * The function returns the current array index of an array iterator.
 *
 * @param iter  Input iterator
 * @return      The current array index
 */
extern uint32_t iot_data_array_iter_index (const iot_data_array_iter_t * iter);

/**
 * @brief Get the value of the element associated with an array iterator
 *
 * The function returns the address of the array element currently referenced by the array iterator.
 *
 * @param iter  Input iterator
 * @return      Pointer to the current array element
 */
extern const void * iot_data_array_iter_value (const iot_data_array_iter_t * iter);

/**
 * @brief Initialise iterator for a map
 *
 * The function initialises an iterator to point to the beginning of a map. Note that
 * the iterator is unsafe in that the map cannot be modified when being iterated other
 * than by using the iot_data_map_iter_replace_value function.
 *
 * @param map   Input map
 * @param iter  Iterator to initialise
 */
extern void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter);

/**
 * @brief Return first element in a map or NULL if map empty
 *
 * @param  map   Input map
 * @return       First element in the map
 */
extern const iot_data_t * iot_data_map_start (iot_data_t * map);

/**
 * @brief Return the value of the first pointer element in a map or NULL if map empty
 *
 * @param  map   Input map
 * @return       Pointer from the first element in the map
 */
extern const void * iot_data_map_start_pointer (iot_data_t * map);

/**
 * @brief Return last element in a map or NULL if map empty
 *
 * @param  map   Input map
 * @return       Last element in the map
 */
extern const iot_data_t * iot_data_map_end (iot_data_t * map);

/**
 * @brief Return the value of the last pointer element in a map or NULL if map empty
 *
 * @param  map   Input map
 * @return       Pointer from the last element in the map
 */
extern const void * iot_data_map_end_pointer (iot_data_t * map);

/**
 * @brief Update the iterator to point to the next element within a map
 *
 * The function to set the iterator to point to the next element within a map. On reaching end of the map,
 * iterator is set to point to first element in the map.
 *
 * @param iter  Input iterator
 * @return      Returns whether the iterator is still valid (has not passed end of the map)
 */
extern bool iot_data_map_iter_next (iot_data_map_iter_t * iter);

/**
 * @brief Returns whether a map iterator has a next element
 *
 * The function returns whether the iterator next function will return a value
 *
 * @param iter  Input iterator
 * @return      Whether the iterator has a next element
 */
extern bool iot_data_map_iter_has_next (const iot_data_map_iter_t * iter);

/**
 * @brief Update the iterator to point to the previous element within a map
 *
 * The function to set the iterator to point to the previous element within a map. On reaching start of the map,
 * iterator is set to point to last element in the map.
 *
 * @param iter  Input iterator
 * @return      Returns whether the iterator is still valid (has not passed start of the map)
 */
extern bool iot_data_map_iter_prev (iot_data_map_iter_t * iter);

/**
 * @brief Get Key from the map referenced by an iterator
 *
 * The function to get the key in a map referenced by the iterator
 *
 * @param iter  Input iterator
 * @return      Pointer to the key of type iot_data if iter is valid, NULL otherwise
 */
extern const iot_data_t * iot_data_map_iter_key (const iot_data_map_iter_t * iter);

/**
 * @brief Get Value from the map referenced by an iterator
 *
 * The function to get the value from the map referenced by the iterator
 *
 * @param iter  Input iterator
 * @return      Pointer to the value of type iot_data if iter is valid, NULL otherwise
 */
extern const iot_data_t * iot_data_map_iter_value (const iot_data_map_iter_t * iter);

/**
 * @brief Replace Value from the map referenced by an iterator
 *
 * The function to replace the value from the map referenced by the iterator
 *
 * @param iter  Input iterator
 * @param value New value to store in the map
 * @return      Pointer to the previous value of type iot_data if iter is valid, NULL otherwise
 */
extern iot_data_t * iot_data_map_iter_replace_value (const iot_data_map_iter_t * iter, iot_data_t * value);

/**
 * @brief Get Key from the map referenced by an iterator
 *
 * The function to get string type key from the map referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      String type key if iter is valid, NULL otherwise
 */
extern const char * iot_data_map_iter_string_key (const iot_data_map_iter_t * iter);

/**
 * @brief Get string value from the map referenced by an iterator
 *
 * The function to get the string value from the map referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      String type value from the map if iter is valid, NULL otherwise
 */
extern const char * iot_data_map_iter_string_value (const iot_data_map_iter_t * iter);

/**
 * @brief Get pointer value from the map referenced by an iterator
 *
 * The function to get the pointer value from the map referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      Pointer type value from the map if iter is valid, NULL otherwise
 */
extern const void * iot_data_map_iter_pointer_value (const iot_data_map_iter_t * iter);

/**
 * @brief Get boolean value from the map referenced by an iterator
 *
 * The function to get the boolean value from the map referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      Boolean value from the map if iter is valid, FALSE otherwise
 */
extern bool iot_data_map_iter_bool_value (const iot_data_map_iter_t * iter);

/**
 * @brief Initialise iterator to the start of a vector
 *
 * The function initialises an iterator to point to the first element of a vector. Note that
 * the iterator is unsafe in that the vector cannot be modified when being iterated, other
 * than by using the iot_data_vector_iter_replace_value function.
 *
 * @param vector Input vector
 * @param iter   Input iterator
 */
extern void iot_data_vector_iter (const iot_data_t * vector, iot_data_vector_iter_t * iter);

/**
 * @brief Iterate to next vector element
 *
 * The function to set the iterator to point to the next element in an vector. On reaching the end of the vector,
 * the iterator is set to point to the first element in the vector.
 *
 * @param  iter  Input iterator
 * @return       Returns whether the iterator is still valid (has not passed end of the vector)
 */
extern bool iot_data_vector_iter_next (iot_data_vector_iter_t * iter);

/**
 * @brief Returns whether a vector iterator has a next element
 *
 * The function returns whether the iterator next function will return a value
 *
 * @param iter  Input iterator
 * @return      Whether the iterator has a next element
 */
extern bool iot_data_vector_iter_has_next (const iot_data_vector_iter_t * iter);

/**
 * @brief Iterate to previous vector element
 *
 * The function to set the iterator to point to the previous element in an vector. On reaching the start of the vector,
 * the iterator is set to point to the last element in the vector.
 *
 * @param  iter  Input iterator
 * @return       Returns whether the iterator is still valid (has not passed start of the vector)
 */
extern bool iot_data_vector_iter_prev (iot_data_vector_iter_t * iter);

/**
 * @brief Get vector index referenced by the iterator
 *
 * The function to return the index of an vector referenced by iterator
 *
 * @param  iter  Input iterator
 * @return       Index of an vector, referenced by iterator
 */
extern uint32_t iot_data_vector_iter_index (const iot_data_vector_iter_t * iter);

/**
 * @brief Get the value from the vector at an index referenced by iterator
 *
 * The function to return the value from the vector at an index referenced by iterator. If iterator index exceeds
 * size of an vector, NULL is returned
 *
 * @param  iter  Input iterator
 * @return       Pointer to a data value from the vector index pointed by iterator if valid, NULL otherwise
 */
extern const iot_data_t * iot_data_vector_iter_value (const iot_data_vector_iter_t * iter);

/**
 * @brief Get the string value from the vector referenced by an iterator
 *
 * Get string type value from the vector referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      String type value if iter is valid, NULL otherwise
 */
extern const char * iot_data_vector_iter_string_value (const iot_data_vector_iter_t * iter);

/**
 * @brief Get pointer value from the vector referenced by an iterator
 *
 * Get the pointer type value from the vector referenced by an iterator
 *
 * @param iter  Input iterator
 * @return      Pointer type value from the vector if iter is valid, NULL otherwise
 */
extern const void * iot_data_vector_iter_pointer_value (const iot_data_vector_iter_t * iter);

/**
 * @brief Replace Value from the vector at the index referenced by iterator
 *
 * The function to replace the value in the the vector at an index referenced by iterator. If iterator index exceeds
 * the size of an vector, no replacement is made and NULL is returned.
 *
 * @param iter  Input iterator
 * @param value New value to store in the vector
 * @return      Pointer to the previous value of type iot_data if iter is valid, NULL otherwise
 */

extern iot_data_t * iot_data_vector_iter_replace_value (const iot_data_vector_iter_t * iter, iot_data_t * value);

/**
 * @brief Get the value as string type from the vector at an index referenced by iterator
 *
 * The function to return the value of string type from the vector at an index referenced by iterator. If iterator index exceeds
 * size of an vector, NULL is returned
 *
 * @param  iter  Input iterator
 * @return       Value of string type from the vector index pointed by iterator if valid, NULL otherwise
 */
extern const char * iot_data_vector_iter_string (const iot_data_vector_iter_t * iter);

/**
 * @brief Find matching element in a vector using compare function
 *
 * Applies a compare function to each element in a vector until the compare
 * function returns true or the end of the vector is reached. Returns a
 * pointer to the matching element or NULL.
 *
 * @param vector  Input vector
 * @param cmp     A comparison function which takes an element and an argument and returns true or false
 * @param arg     Pointer to user supplied data that is passed to the comparison function.
 * @return        Pointer to the first element for which the comparison function return true, NULL otherwise
 */
extern const iot_data_t * iot_data_vector_find (const iot_data_t * vector, iot_data_cmp_fn cmp, const void * arg);

/**
 * @brief Initialise iterator to the start of a iterable
 *
 * The function initialises an iterator to point to the first element of an iterable. Note that
 * the iterator is unsafe in that the iterable cannot be modified when being iterated.
 *
 * @param iterable Input iterable
 * @param iter   Input iterator
 */
extern void iot_data_iter (const iot_data_t * iterable, iot_data_iter_t * iter);

/**
 * @brief Get the number of elements in an iot_data object
 *
 * This function returns the number of elements in an iot_data object.
 * This is 1 for all data types other than lists, vectors, arrays, maps and binary types where it is the number of elements in each.
 *
 * @param data Input data
 * @return Number of elements
 */
extern uint32_t iot_data_length (const iot_data_t * data);

/**
 * @brief Iterate to next iterable element
 *
 * The function to set the iterator to point to the next element in an iterable. On reaching the end of the iterable,
 * the iterator is set to point to the first element in the iterable.
 *
 * @param  iter  Input iterator
 * @return       Returns whether the iterator is still valid (has not passed end of the iterable)
 */
extern bool iot_data_iter_next (iot_data_iter_t * iter);

/**
 * @brief Returns whether a iterable iterator has a next element
 *
 * The function returns whether the iterator next function will return a value
 *
 * @param iter  Input iterator
 * @return      Whether the iterator has a next element
 */
extern bool iot_data_iter_has_next (const iot_data_iter_t * iter);

/**
 * @brief Iterate to previous iterable element
 *
 * The function to set the iterator to point to the previous element in an iterable. On reaching the start of the iterable,
 * the iterator is set to point to the last element in the iterable.
 *
 * @param  iter  Input iterator
 * @return       Returns whether the iterator is still valid (has not passed start of the iterable)
 */
extern bool iot_data_iter_prev (iot_data_iter_t * iter);

/**
 * @brief Get the value from the iterable at an index referenced by iterator
 *
 * The function to return the value from the iterable at an index referenced by iterator. If iterator index exceeds
 * size of an iterable, NULL is returned
 *
 * @param  iter  Input iterator
 * @return       Pointer to a data value from the iterable index pointed by iterator if valid, NULL otherwise
 */
extern const iot_data_t * iot_data_iter_value (const iot_data_iter_t * iter);

/**
 * @brief  Convert data to json string
 *
 * The function to convert data to a json string. If metadata has been
 * added to a map in the format generated by iot_data_from_json_with_ordering
 * then the metadata string vectors will be used to determine which map entries
 * appear in the JSON and the order in which they appear.
 *
 * @param  data  Input data
 * @return       json string
 */
extern char * iot_data_to_json (const iot_data_t * data);

/**
 * @brief  Convert data to json string with initial string size
 *
 * The function to convert data to a json string. An initial string size
 * is provided to eliminate memory reallocation as the required string
 * size increases.
 *
 * @param  data  Input data
 * @param  size  Initial size of output string
 * @return       json string
 */
extern char * iot_data_to_json_with_size (const iot_data_t * data, uint32_t size);

/**
 * @brief  Convert data to json string, writing to provided buffer
 *
 * An initial string buffer and size are provided. If the buffer is too small a new buffer will
 * be allocated, in which case the returned pointer will not be the same as the provided buffer.
 * If a stack allocated buffer is used, it must be large enough to hold the generated json with a 32 byte minimum size.
 *
 * @param  data  Input data
 * @param  buff  Output buffer
 * @param  size  Output buffer size
 * @return       Output buffer or a new buffer if provided buffer too small
 */
extern char * iot_data_to_json_with_buffer (const iot_data_t * data, char * buff, uint32_t size);

/**
 * @brief Convert json to iot_data_t type
 *
 * The function to convert input json string to iot_data
 *
 * @param  json  Input json string
 * @return       Pointer to data of type iot_data if input string is a json object, NULL otherwise
 */
extern iot_data_t * iot_data_from_json (const char * json);

/**
 * @brief Convert json to iot_data_t type with optional object ordering metadata
 *
 * The function to convert input json string to iot_data and optionally adds
 * metadata to the result. The metadata is associated with each map
 * representing JSON objects. The metadata is in the form of a map which
 * contains the key string "ordering" with a vector of strings. The vector
 * consists of the JSON object keys in the order in which they appear in
 * the JSON. See also iot_data_to_json.
 *
 * @param json    Input json string
 * @param ordered Whether returned map is ordered by position in json
 * @return        Pointer to data of type iot_data if input string is a json object, NULL otherwise
 */
extern iot_data_t * iot_data_from_json_with_ordering (const char * json, bool ordered);

/**
 * @brief Convert json to iot_data_t type with optional object ordering metadata and shared key map
 *
 * The function to convert input json string to iot_data and optionally adds
 * metadata to the result. The metadata is associated with each map
 * representing JSON objects. The metadata is in the form of a map which
 * contains the key string "ordering" with a vector of strings. The vector
 * consists of the JSON object keys in the order in which they appear in
 * the JSON. See also iot_data_to_json and iot_data_from_json_with_ordering.
 *
 * @param json    Input json string
 * @param ordered Whether returned map is ordered by position in json
 * @param cache   Optional string map used as a cache for string values, may be NULL
 * @return        Pointer to data of type iot_data if input string is a json object, NULL otherwise
 */
extern iot_data_t * iot_data_from_json_with_cache (const char * json, bool ordered, iot_data_t * cache);

#ifdef IOT_HAS_CBOR
/**
 * @brief  Convert data to CBOR block
 *
 * The function to convert data to cbor.
 *
 * @param  data  Input data
 * @return       CBOR in an IOT_DATA_BINARY
 */
extern iot_data_t * iot_data_to_cbor (const iot_data_t * data);

/**
 * @brief  Convert data to CBOR block with initial buffer size
 *
 * The function to convert data to cbor. An initial buffer size
 * is provided to eliminate memory reallocation as the required buffer
 * size increases.
 *
 * @param  data  Input data
 * @param  size  Initial size of output buffer
 * @return       CBOR in an IOT_DATA_BINARY
 */
extern iot_data_t * iot_data_to_cbor_with_size (const iot_data_t * data, uint32_t size);

/**
 * @bief Convert cbor data to iot_data_t structure
 *
 * @param data Input data
 * @param size size of input data
 * @return iot_data struct
 */
extern iot_data_t * iot_data_from_cbor (const uint8_t *data, uint32_t size);

/**
 * @bief Convert iot binary cbor data to iot_data_t structure
 *
 * @param data Input binary data, must be of type IOT_DATA_BINARY
 * @return iot_data struct
 */
extern iot_data_t * iot_data_from_iot_cbor (const iot_data_t *data);

#endif
#ifdef IOT_HAS_XML
/**
 * @brief Convert XML to iot_data_t type
 *
 * The function to convert input XML string to iot_data. The conversion results
 * in a map containing "name", "attributes", "children" and "content" entries.
 * The "children" and "content" entries will only be present if required.
 * The "children" entry will contain a recursive array of map entries
 * containing any XML child elements.
 *
 * @param  xml   Input XML string
 * @return       A iot_data map if input string is a XML string, NULL otherwise.
 */
extern iot_data_t * iot_data_from_xml (const char * xml);
#endif

#ifdef IOT_HAS_YAML
/**
 * @brief Convert YAML to iot_data_t type
 *
 * The function to convert input YAML string to iot_data. The conversion results
 * in a map, vector or simple type depending on the YAML content.
 *
 * @param  yaml       Input YAML string
 * @param  exception  If a parse error occurs, on exit this will hold a string describing the problem
 * @return            A iot_data element if input string is a YAML string, NULL otherwise.
 */
extern iot_data_t * iot_data_from_yaml (const char * yaml, iot_data_t ** exception);
#endif

/**
 * @brief Check for equality of two iot_data types. For equality both data types and values must match.
 *
 * Function to check the values of the two iot_data types and returns true if the data values and types are the same.
 *
 * @param  data1 Input data1 (can be NULL)
 * @param  data2 Input data2 (can be NULL)
 * @return       'true' if data1 & data2 are equal, 'false' otherwise
 */
extern bool iot_data_equal (const iot_data_t * data1, const iot_data_t * data2);

/**
 * @brief Check for equality of two iot_data instances. Integer or floating types will compare as equal
 *        if they have the same value.
 *
 * Function to check the values of the two iot_data instances and returns true if the data value is the same.
 *
 * @param  data1 Input data1 (can be NULL)
 * @param  data2 Input data2 (can be NULL)
 * @return       'true' if data1 & data2 are equal, 'false' otherwise
 */
extern bool iot_data_equal_value (const iot_data_t * data1, const iot_data_t * data2);

/**
 * @brief Compare two data instances, returning whether the first is less than, equal to or greater than the second.
 *        Both types must be the same for values to compare equal.
 *
 * @param  data1 Input data1 (can be NULL)
 * @param  data2 Input data2 (can be NULL)
 * @return       Returns zero if data1 equals data2, a value less than zero if data1 less than data2, a value greater than zero if data1 greater than data2
 */
extern int iot_data_compare (const iot_data_t * data1, const iot_data_t * data2);

/**
 * @brief Compare two data instances, returning whether the first is less than, equal to or greater than the second. Different integer
 * or floating types are compared by value.
 *
 * @param  data1 Input data1 (can be NULL)
 * @param  data2 Input data2 (can be NULL)
 * @return       Returns zero if data1 equals data2, a value less than zero if data1 less than data2, a value greater than zero if data1 greater than data2
 */
extern int iot_data_compare_value (const iot_data_t * data1, const iot_data_t * data2);

/**
 * @brief Copy data
 *
 * This function copies data from src and returns the pointer of the copied data.
 * Note that to maintain ownership semantics for the POINTER data type, for this type
 * the src argument is returned with it's reference count incremented.
 *
 * @param src Data to copy
 * @return    Pointer to the copied data. The caller should free memory after use
 */
extern iot_data_t * iot_data_copy (const iot_data_t * src);

/**
 * @brief Shallow copy data
 *
 * This function copies data from src and returns the pointer of the copied data.
 * For map, vector and list data a new data object of the same type is constructed
 * and the contents of the data object copied by reference into the new object.
 * All other data types are returned unchanged with the referncec count incremented.
 *
 * @param src Data to copy
 * @return    Pointer to the copied data. The caller should free memory after use
 */
extern iot_data_t * iot_data_shallow_copy (const iot_data_t * src);

/**
 * @brief Check data type matches typecode
 *
 * The function returns where a data instance matches a given typecode. Note that this will
 * return false for polymorphic data types i.e. maps, vectors or lists of differing type.
 *
 * @param data     Data to compare
 * @param typecode Typecode to compare data against
 * @return         Whether the data matches the typecode
 */
extern bool iot_data_matches (const iot_data_t * data, const iot_typecode_t * typecode);

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
 * @brief Converts a vector to an array, vector elements must be castable to the target type, vector elements
 * that cannot be cast are ignored. If no vector elements can be cast to the required type an empty array is returned.
 *
 * @param vector  The vector to transform
 * @param type    The data element type for the created array
 * @param recurse Whether to recurse to contained vectors
 * @return        The newly created array containing the vector elements, may be empty
 */
extern iot_data_t * iot_data_vector_to_array (const iot_data_t * vector, iot_data_type_t type, bool recurse);

/**
 * @brief Converts a vector to a vector, vector elements must be castable to the target type, vector elements
 * that cannot be cast are ignored. If no vector elements can be cast to the required type an empty vector is returned.
 *
 * @param vector  The vector to transform
 * @param type    The data element type for the created vector
 * @param recurse Whether to recurse to contained vectors
 * @return        The newly created vector, may be empty
 */
extern iot_data_t * iot_data_vector_to_vector (const iot_data_t * vector, iot_data_type_t type, bool recurse);

/**
 * @brief Returns the dimensions of a vector representing a multi dimensional array.
 *        Note that a vector must contain only other vectors to be considered an array slice
 *        and each non vector element, must contain the same number of elements.
 *
 * @param vector  The vector
 * @param total   Pointer to an integer set to the total number of elements in the array (multiplication of dimensions). Set to zero if vector structure invalid
 * @return        An array of IOT_DATA_UINT32 containing the array dimensions, or NULL if vector structure invalid
 */
extern iot_data_t * iot_data_vector_dimensions (const iot_data_t * vector, uint32_t * total);

/**
 * @brief Transforms one data type to another. Tf the data type is already of the required type then the same
 *        data is returned with an incremented reference count, otherwise if the data is castable to the target type
 *        a new instance of the required type is returned.
 *
 * @param data   The data to transform
 * @param type   The type for the transformed data
 * @return       The newly created data or NULL if type/value could not be cast
 */
iot_data_t * iot_data_transform (const iot_data_t * data, iot_data_type_t type);

/**
 * @brief Transforms an array of one type to an array of another. Array element values must be castable from one type/value
 * to another, or the array element is ignored. An empty array is returned if no array elements can be transformed.
 *
 * @param array  The array to transform
 * @param type   The data element type for the transformed array
 * @return       The newly created array containing the transformed array elements, may be empty
 */
extern iot_data_t * iot_data_array_transform (const iot_data_t * array, iot_data_type_t type);

/**
 * @brief Returns the size of the contained data type in bytes.
 *
 * @param type   The data type
 * @return       The size of the contained type in bytes (e.g. 4 for IOT_DATA_UINT32)
 */
extern uint32_t iot_data_type_size (iot_data_type_t type);

/**
 * @brief  Returns a data value by traversing nested maps and vectors following the keys and indexes supplied in path.
 *         Each entry in path must be of the correct type for the corresponding map or an unsigned 32 bit integer
 *         for vectors.
 * @param  data The starting data structure
 * @param  path A vector of keys and indexes. If the vector is empty the data parameter is returned
 * @return The data found by traversing the path or NULL if the path does not match the supplied data
 *
 */
extern const iot_data_t * iot_data_get_at (const iot_data_t * data, const iot_data_t * path);


/**
 * @brief  Adds a data value at a point specified by traversing nested maps and vectors following the keys and indexes
 *         supplied in path. Each entry in path must be of the correct type for the corresponding map or an unsigned
 *         32 bit integer for vectors.
 * @param  data The starting data structure
 * @param  path A vector of keys and indexes. If the vector is empty the data parameter is returned
 * @param  val  The value to add
 * @return A new data object of the same type as the data parameter. This data object will be a shallow copy of
 *         the data parameter, as will intermediate objects along the specified path.
 *
 */
extern iot_data_t * iot_data_add_at (const iot_data_t * data, const iot_data_t * path, iot_data_t * val);

/**
 * @brief  Removes a data value specified by traversing nested maps and vectors following the keys and indexes supplied in path.
 *         Each entry in path must be of the correct type for the corresponding map or an unsigned 32 bit integer
 *         for vectors.
 * @param  data The starting data structure
 * @param  path A vector of keys and indexes. If the vector is empty the data parameter is returned
 * @return A new data object of the same type as the data parameter. This data object will be a shallow copy of
 *         the data parameter, as will intermediate objects along the specified path.
 *
 */
extern iot_data_t * iot_data_remove_at (const iot_data_t * data, const iot_data_t * path);


/**
 * @brief  Updates a data value at a point specified by traversing nested maps and vectors following the keys and indexes
 *         supplied in path. Each entry in path must be of the correct type for the corresponding map key
 *         or an unsigned 32 bit integer for vectors.
 * @param  data The starting data structure
 * @param  path A vector of keys and indexes. If the vector is empty the data parameter is returned
 * @param  fn Pointer to an update function. The function will be passed the existing value specified by the path and the arg parameter.
 *         The function must return the new value.
 * @param  arg User specified pointer supplied to the update function.
 * @return A new data object of the same type as the data parameter. This data object will be a shallow copy of
 *         the data parameter, as will intermediate objects along the specified path.
 *
 */
extern iot_data_t * iot_data_update_at (const iot_data_t * data, const iot_data_t * path, iot_data_update_fn fn, void * arg);

/**
 * @brief Returns the value of a boolean user tag
 *
 * @param data   The data instance
 * @param tag    The tag identifier (two supported)
 * @return       The boolean value of the tag
 */
extern bool iot_data_get_tag (const iot_data_t * data, iot_data_tag_t tag);

/**
 * @brief Sets the value of a boolean user tag
 *
 * @param data   The data instance
 * @param tag    The tag identifier (two supported)
 * @param value  The boolean value to set the tag
 * @return       The old boolean value of the tag
 */
extern bool iot_data_set_tag (iot_data_t * data, iot_data_tag_t tag, bool value);

/**
 * @brief Returns the size in bytes of an allocated/cached memory block
 * @return       The size of a memory block
 */
extern uint32_t iot_data_block_size (void);

/**
 * @brief Returns a memory block
 * @param size   Required block size
 * @return       Pointer to a memory block, or NULL if requested size is greater than block size
 */
extern void * iot_data_block_alloc (size_t size);

/**
 * @brief Frees a memory block
 * @param ptr Pointer to the memory block to be freed
 */
extern void iot_data_block_free (void * ptr);

/**
 * @brief Get the restricted element type based on the elements it contains
 * @param data The data instance
 * @return the restricted element type. If the data contains elements with different types, IOT_DATA_MULTI will be returned
 */
extern iot_data_type_t iot_data_restricted_element_type (const iot_data_t * data);

/**
 * @brief Restrict the element type of the data based on the elements it contains
 * @param data The data instance
 */
extern void iot_data_restrict_element (iot_data_t * data);

#ifdef __cplusplus
}
#endif
#endif
