//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_DATA_H_
#define _IOT_DATA_H_

/**
 * @file
 * @brief IOTech Data representation API
 */

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum iot_data_type_t
{
  IOT_DATA_INT8 = 0,
  IOT_DATA_UINT8 = 1,
  IOT_DATA_INT16 = 2,
  IOT_DATA_UINT16 = 3,
  IOT_DATA_INT32 = 4,
  IOT_DATA_UINT32 = 5,
  IOT_DATA_INT64 = 6,
  IOT_DATA_UINT64 = 7,
  IOT_DATA_FLOAT32 = 8,
  IOT_DATA_FLOAT64 = 9,
  IOT_DATA_BOOL = 10,
  IOT_DATA_STRING = 11,
  IOT_DATA_BLOB = 12,
  IOT_DATA_MAP = 13,
  IOT_DATA_ARRAY = 14
} iot_data_type_t;

typedef enum iot_data_ownership_t
{
  IOT_DATA_COPY,  /* Data is copied and copy freed when no longer used */
  IOT_DATA_TAKE,  /* Data is taken and freed when no longer used */
  IOT_DATA_REF    /* Data is referenced and never freed */
} iot_data_ownership_t;

typedef struct iot_data_t iot_data_t;

typedef struct iot_data_map_iter_t
{
  struct iot_data_map_t * map;
  struct iot_data_pair_t * pair;
} iot_data_map_iter_t;

typedef struct iot_data_array_iter_t
{
  struct iot_data_array_t * array;
  uint32_t index;
} iot_data_array_iter_t ;

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
 * @brief Get data_type
 *
 * The function to return data_type for the data
 *
 * @param data  Pointer to data
 * @return      Datatype from the enum defined in iot_data_type_t
 */
extern iot_data_type_t iot_data_type (const iot_data_t * data);

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
 * @return       String that indicates the data_type
 */
extern const char * iot_data_type_name (const iot_data_t * data);

/**
 * @brief Return address allocated for data
 *
 * The function to return the address allocated for data
 *
 * @param data  Pointer to data
 * @return      Pointer that holds the address of data
 */
extern void * iot_data_address (const iot_data_t * data); /* Not for Maps or Arrays */

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
 * @brief Allocate memory for a blob
 *
 * The function to allocate memory for a blob of size passed.
 *
 * @param data       Blob data
 * @param size       Size of the blob to allocate
 * @param ownership  If the ownership is set to IOT_DATA_COPY, a new allocation is made and data is copied to the allocated
 *                   memory, else the address of the data passed is stored
 * @return           Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_blob (uint8_t * data, uint32_t size, iot_data_ownership_t ownership);

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
 * @brief Allocate memory for an array
 *
 * The function to allocate memory for an array type
 *
 * @param size  Length of the array for allocation
 * @return      Pointer to the allocated memory
 */
extern iot_data_t * iot_data_alloc_array (uint32_t size);

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
 * The function to allocate memory of data_type blob for base64 decoded data
 *
 * @param value  Data to decode from base64
 * @return       Pointer to the allocated memory of blob data_type with the base64 decoded value, NULL on error
 */
extern iot_data_t * iot_data_alloc_blob_from_base64 (const char * value);

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
 * @brief Get value of type uint16 stored in data
 *
 * The function to get the value stored in data
 *
 * @param data  Data pointer to retrieve value
 * @return      Value stored in the address pointed by data
 */
extern const uint8_t * iot_data_blob (const iot_data_t * data, uint32_t * size);

/**
 * @brief Add key-value pair to a map
 *
 * The function to add a key-value pair to a map
 *
 * @param map  Map to add a new key-value pair
 * @param key  Input key
 * @param val  Input value
 * Note: The ownership of key and value supplied to a map is owned by the map and cannot be reused, unless reference counted
 */
extern void iot_data_map_add (iot_data_t * map, iot_data_t * key, iot_data_t * val);

/**
 * @brief Get the size of a map
 *
 * The function to get the map size
 *
 * @param map Get the size of the map
 * @return    map size
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
 * Note: The ownership of key and value supplied to a map is owned by the map and cannot be reused, unless reference counted
 */
extern void iot_data_string_map_add (iot_data_t * map, const char * key, iot_data_t * val);

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
 * @brief Get string value from the map for a key provided
 *
 * The function to get the string value from the map for an input key provided as a string
 *
 * @param map  Map to get the value
 * @param key  Input key of type String
 * @return     String value corresponding to the key
 */
extern const char * iot_data_string_map_get_string (const iot_data_t * map, const char * key);

/**
 * @brief Get value corresponding to key from the map, if the datatype of the value is IOT_DATA_INT64
 *
 * The function to get int64 value corresponding to key from the map, if the value type is IOT_DATA_INT64, else return default_val
 *
 * @param map          Map to get the value
 * @param key          Key to get the corresponding value
 * @param default_val  Default Value
 * @return             Value corresponding to the key if the datatype is int64, else default_val
 */
extern int64_t iot_data_string_map_get_i64 (const iot_data_t * map, const char * key, int64_t default_val);

/**
 * @brief Get value corresponding to key from the map, if the datatype of the value is IOT_DATA_BOOL
 *
 * The function to get bool value corresponding to key from the map, if the value type is IOT_DATA_BOOL, else return default_val
 *
 * @param map          Map to get the value
 * @param key          Key to get the corresponding value
 * @param default_val  Default Value
 * @return             Value corresponding to the key if the datatype is bool, else default_val
 */
extern bool iot_data_string_map_get_bool (const iot_data_t * map, const char * key, bool default_val);

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
 * @brief  Decode base64 value and store it as a blob in a map for a given key
 *
 * The function to decode base64 value in a map store it as a blob for an input key
 *
 * @param map  Input map
 * @param key  Input key
 * @return     'true' on Success, 'false' on Failure
 */
extern bool iot_data_map_base64_to_blob (iot_data_t * map, const iot_data_t * key);

/**
 * @brief Add an element to an array at index
 *
 * The function to add a value to an array at index
 *
 * @param array  Input array to add an element
 * @param index  Index in an array
 * @param val    Pointer to a value of type iot_data to add
 * Note: The ownership of the value supplied to array is owned by the array and cannot be reused, unless reference counted
 */
extern void iot_data_array_add (iot_data_t * array, uint32_t index, iot_data_t * val);

/**
 * @brief Get a value at an index from then array
 *
 * The function to get the value from then array at a given index
 *
 * @param array  Input array
 * @param index  Index of an array to get the value
 * @return       Pointer to a value at the index
 */
extern const iot_data_t * iot_data_array_get (const iot_data_t * array, uint32_t index);

/**
 * @brief Get the array size
 *
 * The function to get the length of the input array
 *
 * @param array  Input array
 * @return       Size of an array
 */
extern uint32_t iot_data_array_size (const iot_data_t * array);

/**
 * @brief Initialise iterator for a map
 *
 * The function initialises an iterator to point to the beginning of a map. Note that
 * the iterator is unsafe in that the map cannot be modified when being iterated.
 *
 * @param map   Input map
 * @param iter  Iterator to initialise
 */
extern void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter);

/**
 * @brief Update the iterator to point to a next element within a map
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
 * @brief Initialise iterator to an array
 *
 * The function initialises an iterator to to point to an array. Note that
 * the iterator is unsafe in that the array cannot be modified when being iterated.
 *
 * @param array  Input array
 * @param iter   Input iterator
 */
extern void iot_data_array_iter (const iot_data_t * array, iot_data_array_iter_t * iter);

/**
 * @brief Iterate to next array element
 *
 * The function to set the iterator to point to the next element in an array
 *
 * @param  iter  Input iterator
 * @return       'true' if the iterator index is <= array length, 'false' otherwise
 */
extern bool iot_data_array_iter_next (iot_data_array_iter_t * iter);

/**
 * @brief Get array index referenced by the iterator
 *
 * The function to return the index of an array referenced by iterator
 *
 * @param  iter  Input iterator
 * @return       Index of an array, referenced by iterator
 */
extern uint32_t iot_data_array_iter_index (const iot_data_array_iter_t * iter);

/**
 * @brief Get the value from the array at an index referenced by iterator
 *
 * The function to return the value from the array at an index referenced by iterator. If iterator index exceeds
 * size of an array, NULL is returned
 *
 * @param  iter  Input iterator
 * @return       Pointer to a data value from the array index pointed by iterator if valid, NULL otherwise
 */
extern const iot_data_t * iot_data_array_iter_value (const iot_data_array_iter_t * iter);

/**
 * @brief Get the value as string type from the array at an index referenced by iterator
 *
 * The function to return the value of string type from the array at an index referenced by iterator. If iterator index exceeds
 * size of an array, NULL is returned
 *
 * @param  iter  Input iterator
 * @return       Value of string type from the array index pointed by iterator if valid, NULL otherwise
 */
extern const char * iot_data_array_iter_string (const iot_data_array_iter_t * iter);

/**
 * @brief  Convert data to json string
 *
 * The function to convert data to a json string
 *
 * @param  data  Input data
 * @param  wrap  wrap data for basic types if set
 * @return       json string
 */
extern char * iot_data_to_json (const iot_data_t * data, bool wrap);

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
 * @brief Check for equality of 2 iot_data types
 *
 * The function to check the values of the 2 iot_data types and return true if the data is same
 *
 * @param  data1 Input data1
 * @param  data2 Input data2
 * @return       'true' if data1 & data2 are equal, 'false' otherwise
 */
extern bool iot_data_equal (const iot_data_t * data1, const iot_data_t * data2);

#ifdef __cplusplus
}
#endif
#endif
