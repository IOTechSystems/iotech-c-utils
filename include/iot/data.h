//
// Copyright (c) 2019-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_DATA_H_
#define _IOT_DATA_H_

/**
 * @file
 * @brief IOTech Data Representation API
 */

#include "iot/os.h"
#include "iot/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Basic type string names
*/
#define IOT_DATA_TYPENAME_INT8 "int8"
#define IOT_DATA_TYPENAME_UINT8 "uint8"
#define IOT_DATA_TYPENAME_INT16 "int16"
#define IOT_DATA_TYPENAME_UINT16 "uint16"
#define IOT_DATA_TYPENAME_INT32 "int32"
#define IOT_DATA_TYPENAME_UINT32 "uint32"
#define IOT_DATA_TYPENAME_INT64 "int64"
#define IOT_DATA_TYPENAME_UINT64 "uint64"
#define IOT_DATA_TYPENAME_FLOAT32 "float32"
#define IOT_DATA_TYPENAME_FLOAT64 "float64"
#define IOT_DATA_TYPENAME_BOOL "bool"
#define IOT_DATA_TYPENAME_STRING "string"
#define IOT_DATA_TYPENAME_ARRAY "array"
#define IOT_DATA_TYPENAME_MAP "map"
#define IOT_DATA_TYPENAME_VECTOR "vector"

/**
 * Alias for data type enumeration
 */
typedef enum iot_data_type_t
{
  IOT_DATA_INT8 = 0,    /**< Signed 8bit integer */
  IOT_DATA_UINT8 = 1,   /**< Unsigned 8bit integer */
  IOT_DATA_INT16 = 2,   /**< Signed 16bit integer */
  IOT_DATA_UINT16 = 3,  /**< Unsigned 16bit integer */
  IOT_DATA_INT32 = 4,   /**< Signed 32bit integer */
  IOT_DATA_UINT32 = 5,  /**< Unsigned 32bit integer */
  IOT_DATA_INT64 = 6,   /**< Signed 64bit integer */
  IOT_DATA_UINT64 = 7,  /**< Unsigned 64bit integer */
  IOT_DATA_FLOAT32 = 8, /**< 32bit float */
  IOT_DATA_FLOAT64 = 9, /**< 64bit float */
  IOT_DATA_BOOL = 10,   /**< Boolean */
  IOT_DATA_STRING = 11, /**< String */
  IOT_DATA_ARRAY = 12,  /**< Array */
  IOT_DATA_MAP = 13,    /**< Map */
  IOT_DATA_VECTOR = 14  /**< Vector */
} iot_data_type_t;

/**
 * Alias for data ownership enumeration
 */
typedef enum iot_data_ownership_t
{
  IOT_DATA_COPY = 0u,  /**< Data is copied and copy freed when no longer used */
  IOT_DATA_TAKE = 1u,  /**< Data is taken and freed when no longer used */
  IOT_DATA_REF = 2u    /**< Data is referenced and never freed */
} iot_data_ownership_t;

/** Alias for iot data structure */
typedef struct iot_data_t iot_data_t;

/** Alias for iot typecode structure */
typedef struct iot_typecode_t iot_typecode_t;

/**
 * Alias for data map iterator structure
 */
typedef struct iot_data_map_iter_t
{
  struct iot_data_map_t * map;   /**< Pointer to data map structure */
  struct iot_data_pair_t * pair; /**< Pointer to data pair structure */
} iot_data_map_iter_t;

/**
 * Alias for data vector iterator structure
 */
typedef struct iot_data_vector_iter_t
{
  struct iot_data_vector_t * vector;  /**< Pointer to data vector structure */
  uint32_t index;                     /**< Index of the given vector */
} iot_data_vector_iter_t;

/**
 * Alias for data array iterator structure
 */
typedef struct iot_data_array_iter_t
{
  struct iot_data_array_t * array;  /**< Pointer to data array structure */
  uint32_t index;                   /**< Index of the given data array */
} iot_data_array_iter_t;

/** Alias for data comparison function pointer */
typedef bool (*iot_data_cmp_fn) (const iot_data_t * data, const void * arg);

/**
 * @brief Increment the data reference count
 *
 * The function to increment reference count of data by 1
 *
 * @param data  Pointer to data
 */
extern void iot_data_add_ref (iot_data_t * data);

/**
 * @brief Free memory allocated to data
 *
 * The function to decrement reference count by 1 and free the allocated memory only if the reference count is <=1
 *
 * @param data  Pointer to data
 */
extern void iot_data_free (iot_data_t * data);

/**
 * @brief Get core data type
 *
 * The function to return the basic data type for the data
 *
 * @param data  Pointer to data
 * @return      Datatype from the enum defined in iot_data_type_t
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
 * @brief Get data type code
 *
 * The function to return the type code for the data
 *
 * @param data  Pointer to data
 * @return      Creates and returns a type code representing the data type
 */
extern iot_typecode_t * iot_data_typecode (const iot_data_t * data);

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
 * Set metadata on a data item. Meta data elements are reference counted by the associated data element.
 *
 * @param  data  Pointer to data
 * @param  metadata  Pointer to metadata
 */
extern void iot_data_set_metadata (iot_data_t * data, iot_data_t * metadata);

/**
 * @brief Get metadata for data
 *
 * Gets the metadata associated with a data item.
 *
 * @param  data  Pointer to data
 * @return       Pointer to metadata (NULL if not set)
 */
extern const iot_data_t * iot_data_get_metadata (const iot_data_t * data);

/**
 * @brief Return address allocated for data
 *
 * The function to return the address allocated for data
 *
 * @param data  Pointer to data
 * @return      Pointer that holds the address of data
 */
extern void * iot_data_address (const iot_data_t * data); /* Not for Maps or Vectors */

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
 * @brief Allocate memory for a string
 *
 * The function to allocate memory for a string
 *
 * @param val        String
 * @param ownership  If the ownership is set to IOT_DATA_COPY, input val is duplicated with the address allocated, else
 *                   the address passed is stored
 * @return           Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_string (const char * val, iot_data_ownership_t ownership);

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
 * @brief Find array type
 *
 * Function to return the type of an Array.
 *
 * @param array      Array
 * @return           Type of array data
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
 * @brief  Allocate memory for a map
 *
 * The function to allocate memory for a map with a key type
 *
 * @param key_type  Datatype of the key associated with the map
 * @return          Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_map (iot_data_type_t key_type);

/**
 * @brief Allocate memory for an vector
 *
 * The function to allocate memory for an vector type
 *
 * @param size  Length of the vector for allocation, could be zero to create zero length vector
 * @return      Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_vector (uint32_t size);

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
 * @brief Increment data value by 1
 *
 * The function to increment the value by 1
 *
 * @param data  Value to increment. Applicable for integer and float data_types
 */
extern void iot_data_increment (iot_data_t * data);

/**
 * @brief Decrement data value by 1
 *
 * The function to decrement the value by 1
 *
 * @param data  Value to decrement. Applicable for integer and float data_types
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
 * @return      Value stored in the address pointed by data
 */
extern const char * iot_data_string (const iot_data_t * data);

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
 * @brief Add key-value pair to a map
 *
 * The function to add a key-value pair to a map where a key is of string type
 *
 * @param map  Map to add a new key-value pair
 * @param key  Input key of string type
 * @param val  Input value
 * Note: The ownership of key and value passed is owned by the map and cannot be reused, unless reference counted
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
 * @brief Get int64_t value corresponding to key from a map
 *
 * The function to get int64 value corresponding to key from the map, if the value type is IOT_DATA_INT64, else return default_val
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @param default_val  Default int64 value
 * @return             Int64_t value corresponding to the key, else default_val
 */
extern int64_t iot_data_string_map_get_i64 (const iot_data_t * map, const char * key, int64_t default_val);

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
 * @return             Vector found by key in the map. NULL if not found or not a vector.
 */
extern const iot_data_t * iot_data_string_map_get_vector (const iot_data_t * map, const char * key);

/**
 * @brief Get map corresponding to key from a string map
 *
 * The function to get a map corresponding to key from the map, if the type value is not IOT_DATA_MAP then NULL is returned
 *
 * @param map          Map from which get a value
 * @param key          String key for the value
 * @return             Map found by key in the map. NULL if not found or not a map.
 */
extern const iot_data_t * iot_data_string_map_get_map (const iot_data_t * map, const char * key);
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
extern bool iot_data_map_base64_to_array (iot_data_t * map, const iot_data_t * key);

/**
 * @brief Add an element to an vector at index
 *
 * The function to add a value to an vector at index
 *
 * @param vector  Input vector to add an element
 * @param index  Index in an vector
 * @param val    Pointer to a value of type iot_data to add
 * Note: The ownership of the value passed is owned by the vector and cannot be reused, unless reference counted
 */
extern void iot_data_vector_add (iot_data_t * vector, uint32_t index, iot_data_t * val);

/**
 * @brief Get a value at an index from then vector
 *
 * The function to get the value from then vector at a given index
 *
 * @param vector  Input vector
 * @param index  Index of an vector to get the value
 * @return       Pointer to a value at the index
 */
extern const iot_data_t * iot_data_vector_get (const iot_data_t * vector, uint32_t index);

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
 * @brief Get the vector size
 *
 * The function to get the length of the input vector
 *
 * @param vector  Input vector
 * @return       Size of an vector
 */
extern uint32_t iot_data_vector_size (const iot_data_t * vector);

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
 * iterator is set to point to first element in the array.
 *
 * @param iter  Input iterator
 * @return      'true' on Success, 'false' when the iterator reaches end of the array
 */
extern bool iot_data_array_iter_next (iot_data_array_iter_t * iter);

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
 * than by using the iot_data_map_iter_replace_value() function.
 *
 * @param map   Input map
 * @param iter  Iterator to initialise
 */
extern void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter);

/**
 * @brief Update the iterator to point to the next element within a map
 *
 * The function to set the iterator to point to the next element within a map. On reaching end of the map,
 * iterator is set to point to first element in the map.
 *
 * @param iter  Input iterator
 * @return      'true' on Success, 'false' when the iterator reaches end of the map
 */
extern bool iot_data_map_iter_next (iot_data_map_iter_t * iter);

/**
 * @brief Get Key from the map referenced by an input iterator
 *
 * The function to get the key in a map referenced by the iterator
 *
 * @param iter  Input iterator
 * @return      Pointer to the key of type iot_data if iter is valid, NULL otherwise
 */
extern const iot_data_t * iot_data_map_iter_key (const iot_data_map_iter_t * iter);

/**
 * @brief Get Value from the map referenced by an input iterator
 *
 * The function to get the value from the map referenced by the iterator
 *
 * @param iter  Input iterator
 * @return      Pointer to the value of type iot_data if iter is valid, NULL otherwise
 */
extern const iot_data_t * iot_data_map_iter_value (const iot_data_map_iter_t * iter);

/**
 * @brief Replace Value from the map referenced by an input iterator
 *
 * The function to replace the value from the map referenced by the iterator
 *
 * @param iter  Input iterator
 * @param value New value to store in the map
 * @return      Pointer to the previous value of type iot_data if iter is valid, NULL otherwise
 */

extern iot_data_t * iot_data_map_iter_replace_value (iot_data_map_iter_t * iter, iot_data_t *value);

/**
 * @brief Get Key from the map referenced by an input iterator
 *
 * The function to get string type key from the map referenced by an input iterator
 *
 * @param iter  Input iterator
 * @return      String type key if iter is valid, NULL otherwise
 */
extern const char * iot_data_map_iter_string_key (const iot_data_map_iter_t * iter);

/**
 * @brief Get Value from the map referenced by an input iterator
 *
 * The function to get the string type value from the map referenced by an input iterator
 *
 * @param iter  Input iterator
 * @return      String type value from the map if iter is valid, NULL otherwise
 */
extern const char * iot_data_map_iter_string_value (const iot_data_map_iter_t * iter);

/**
 * @brief Initialise iterator to an vector
 *
 * The function initialises an iterator to to point to an vector. Note that
 * the iterator is unsafe in that the vector cannot be modified when being iterated, other
 * than by using the iot_data_vector_iter_replace_value() function.
 *
 * @param vector  Input vector
 * @param iter   Input iterator
 */
extern void iot_data_vector_iter (const iot_data_t * vector, iot_data_vector_iter_t * iter);

/**
 * @brief Iterate to next vector element
 *
 * The function to set the iterator to point to the next element in an vector. On reaching end of the vector,
 * iterator is set to point to first element in the vector.
 *
 * @param  iter  Input iterator
 * @return       'true' if the iterator index is <= vector length, 'false' otherwise
 */
extern bool iot_data_vector_iter_next (iot_data_vector_iter_t * iter);

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
 * @brief Replace Value from the vector at the index referenced by iterator
 *
 * The function to replace the value in the the vector at an index referenced by iterator. If iterator index exceeds
 * the size of an vector, no replacement is made and NULL is returned.
 *
 * @param iter  Input iterator
 * @param value New value to store in the vector
 * @return      Pointer to the previous value of type iot_data if iter is valid, NULL otherwise
 */

extern iot_data_t * iot_data_vector_iter_replace_value (iot_data_vector_iter_t * iter, iot_data_t *value);

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
 * @param cmp     A comparison function which takes an element and an arg and return true or false
 * @param arg     Pointer to user supplied data that is passed to the comparison function.
 * @return        Pointer to the first element for which the comparison function return true, NULL otherwise
 */

extern const iot_data_t * iot_data_vector_find (const iot_data_t * vector, iot_data_cmp_fn cmp, const void * arg);

/**
 * @brief  Convert data to json string
 *
 * The function to convert data to a json string
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
 * @brief Convert json to iot_data_t type
 *
 * The function to convert input json string to iot_data
 *
 * @param  json  Input json string
 * @return       Pointer to data of type iot_data if input string is a json object, NULL otherwise
 */
extern iot_data_t * iot_data_from_json (const char * json);

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

/**
 * @brief Check for equality of 2 iot_data types
 *
 * The function to check the values of the 2 iot_data types and return true if the data is same
 *
 * @param  data1 Input data1
 * @param  data2 Input data2
 * @return       'true' if data1 & data2 are equal, 'false' otherwise
 */
extern bool iot_data_equal (const iot_data_t * data1, const iot_data_t * data2);

/**
 * @brief Copy data
 *
 * The function to copy data from src and return the pointer of the copied data
 *
 * @param src Data to copy
 * @return    Pointer to the copied data. The caller should free memory after use
 */
extern iot_data_t * iot_data_copy (const iot_data_t * src);

/**
 * @brief Check data type matches typecode
 *
 * The function returns where a data instance matches a given typecode. Not that this will
 * return false for polymorphic data types i.e. maps or vectors of differing type.
 *
 * @param data     Data to compare
 * @param typecode Typecode to compare dat against
 * @return         Whether the data matches the typecode
 */
extern bool iot_data_matches (const iot_data_t * data, const iot_typecode_t * typecode);

#ifdef __cplusplus
}
#endif
#endif
