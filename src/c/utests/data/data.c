/*
 * Copyright (c) 2020-2022
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/config.h"
#include "iot/logger.h"
#include "iot/time.h"
#include "iot/uuid.h"
#include "data.h"
#include "CUnit.h"
#include <float.h>
#include <math.h>


static int suite_init (void)
{
  iot_logger_start (iot_logger_default ());
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void test_data_types (void)
{
  uint8_t array [4] = { 0, 1, 2 ,3 };
  iot_data_t * data;
  data = iot_data_alloc_i8 (1);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int8") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 1u)
  iot_data_free (data);
  data = iot_data_alloc_ui8 (1u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt8") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 1u)
  iot_data_free (data);
  data = iot_data_alloc_i16 (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int16") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 2u)
  iot_data_free (data);
  data = iot_data_alloc_ui16 (2u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt16") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 2u)
  iot_data_free (data);
  data = iot_data_alloc_i32 (3);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int32") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 4u)
  iot_data_free (data);
  data = iot_data_alloc_ui32 (3u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt32") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 4u)
  iot_data_free (data);
  data = iot_data_alloc_i64 (4);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int64") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT64)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 8u)
  iot_data_free (data);
  data = iot_data_alloc_ui64 (4u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt64") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 8u)
  iot_data_free (data);
  data = iot_data_alloc_f32 (5.0f);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Float32") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT32)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 4u)
  iot_data_free (data);
  data = iot_data_alloc_f64 (6.0);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Float64") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT64)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 8u)
  iot_data_free (data);
  data = iot_data_alloc_bool (true);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Bool") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == sizeof (bool))
  iot_data_free (data);
  data = iot_data_alloc_string ("Hello", IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == sizeof (char*))
  iot_data_free (data);
  data = iot_data_alloc_string ("Hello", IOT_DATA_COPY);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  iot_data_free (data);
  data = iot_data_alloc_string (strdup ("Hello"), IOT_DATA_TAKE);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  iot_data_free (data);
  data = iot_data_alloc_string_fmt ("Legs: %d", 11);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_data_string (data), "Legs: 11") == 0)
  iot_data_free (data);
  data = iot_data_alloc_array (array, 4, IOT_DATA_UINT8, IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_binary (array, 4, IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Binary") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BINARY)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 1u)
  iot_data_free (data);
  data = iot_data_alloc_array (calloc (1, sizeof (array)), 4, IOT_DATA_UINT8, IOT_DATA_TAKE);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 0u)
  iot_data_free (data);
  data = iot_data_alloc_array (array, 4, IOT_DATA_UINT8, IOT_DATA_COPY);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_map (IOT_DATA_UINT32);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Map") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_MAP)
  CU_ASSERT (iot_data_map_key_type (data) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 0u)
  iot_data_free (data);
  data = iot_data_alloc_vector (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Vector") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_VECTOR)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 0u)
  iot_data_free (data);
  data = iot_data_alloc_null ();
  CU_ASSERT (strcmp (iot_data_type_name (data), "Null") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_NULL)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 0u)
  iot_data_free (data);
  data = iot_data_alloc_pointer (data, NULL);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Pointer") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_POINTER)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == sizeof (void*))
  iot_data_free (data);
  data = iot_data_alloc_list ();
  CU_ASSERT (strcmp (iot_data_type_name (data), "List") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_LIST)
  CU_ASSERT (iot_data_type_size (iot_data_type (data)) == 0u)
  iot_data_free (data);
}

static void test_data_array_key (void)
{
  uint8_t data1 [4] = { 0, 1, 2 ,3 };
  uint8_t data2 [4] = { 0, 1, 2 ,4 };
  uint8_t data3 [4] = { 4, 3, 2 ,1 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_ARRAY);
  CU_ASSERT (iot_data_map_key_type (map) == IOT_DATA_ARRAY)
  iot_data_t * array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * array2 = iot_data_alloc_array (data2, sizeof (data2), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * array3 = iot_data_alloc_array (data3, sizeof (data3), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_ui32 (66u);
  iot_data_t * duffkey = iot_data_alloc_i32 (55);
  iot_data_map_add (map, array1, val);
  const iot_data_t * ret = iot_data_map_get (map, array1);
  CU_ASSERT (ret == val)
  ret = iot_data_map_get (map, array2);
  CU_ASSERT (ret == NULL)
  ret = iot_data_map_get (map, duffkey);
  CU_ASSERT (ret == NULL)
  ret = iot_data_map_get (map, array3);
  CU_ASSERT (ret == NULL)
  iot_data_free (array2);
  iot_data_free (array3);
  iot_data_free (duffkey);
  iot_data_free (map);
}

static void test_data_array_iter_next (void)
{
  uint8_t index = 0;
  uint8_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (uint8_t), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);
  CU_ASSERT (iot_data_array_iter_value (&array_iter) == NULL)
  for (int i = 0; i<4; i++)
  {
    while (iot_data_array_iter_next (&array_iter))
    {
      CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
      CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
      CU_ASSERT (*((uint8_t *) iot_data_array_iter_value (&array_iter)) == data[index])
      index++;
    }
    index = 0;

    CU_ASSERT (iot_data_array_iter_next (&array_iter) == true)
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (*((uint8_t *)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }
  iot_data_free (array);
}

static void test_data_array_iter_prev (void)
{
  uint8_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (uint8_t), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  uint8_t index = 3;
  while (iot_data_array_iter_prev (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((uint8_t *) iot_data_array_iter_value (&array_iter)) == data[index])
    index--;
  }
  index = 3;
  while (iot_data_array_iter_prev (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((uint8_t *) iot_data_array_iter_value (&array_iter)) == data[index])
    index--;
  }

  iot_data_free (array);
}

static void test_data_array_iter_uint8 (void)
{
  uint8_t index = 0;
  uint8_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (uint8_t), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((uint8_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_int8 (void)
{
  uint8_t index = 0;
  int8_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (int8_t), IOT_DATA_INT8, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((int8_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_uint16 (void)
{
  uint8_t index = 0;
  uint16_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (uint16_t), IOT_DATA_UINT16, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((uint16_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_int16 (void)
{
  uint8_t index = 0;
  int16_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (int16_t), IOT_DATA_INT16, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((int16_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_int32 (void)
{
  uint8_t index = 0;
  int32_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (int32_t), IOT_DATA_INT32, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((int32_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_uint32 (void)
{
  uint8_t index = 0;
  uint32_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (uint32_t), IOT_DATA_UINT32, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((uint32_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_int64 (void)
{
  uint8_t index = 0;
  int64_t data [4] = { 0, 1, 2, 3 };

  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (int64_t), IOT_DATA_INT64, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((int64_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_uint64 (void)
{
  uint8_t index = 0;
  uint64_t data [4] = { 0, 1, 2, 3 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (uint64_t), IOT_DATA_UINT64, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  CU_ASSERT (iot_data_array_is_of_type (array, IOT_DATA_UINT64))
  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((uint64_t*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_float32 (void)
{
  uint8_t index = 0;
  float data [4] = { 0.0f, 1.0f, 2.0f, 3.0f };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (float), IOT_DATA_FLOAT32, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((float*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_float64 (void)
{
  uint8_t index = 0;
  double data [4] = { 0.0, 1.0, 2.0, 3.0 };
  iot_data_array_iter_t array_iter;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (double), IOT_DATA_FLOAT64, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((double*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_array_iter_bool (void)
{
  uint8_t index = 0;
  bool data [4] = { false, true, false, true };
  iot_data_array_iter_t array_iter;
  bool has_next = true;

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (bool), IOT_DATA_BOOL, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((bool*)iot_data_array_iter_value (&array_iter)) == data[index])
    has_next = iot_data_array_iter_has_next (&array_iter);
    index++;
  }
  CU_ASSERT (!has_next)

  iot_data_free (array);
}

static void test_data_string_vector (void)
{
  const char * strs [2] = { "Test", "Tube" };
  uint32_t index = 0;
  bool has_next = true;
  iot_data_vector_iter_t iter;
  iot_data_t * vector = iot_data_alloc_vector (2);
  iot_data_vector_iter (vector, &iter);
  CU_ASSERT (iot_data_vector_iter_has_next (&iter))
  iot_data_t * str1 = iot_data_alloc_string (strs[0], IOT_DATA_REF);
  iot_data_t * str2 = iot_data_alloc_string (strs[1], IOT_DATA_REF);
  iot_data_vector_add (vector, 0, str1);
  iot_data_vector_add (vector, 1, str2);
  CU_ASSERT (iot_data_vector_iter_has_next (&iter))
  while (iot_data_vector_iter_next (&iter))
  {
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_vector_iter_string (&iter) == strs[index])
    has_next = iot_data_vector_iter_has_next (&iter);
    index++;
  }
  CU_ASSERT (! has_next)
  CU_ASSERT (iot_data_vector_iter_has_next (&iter))
  CU_ASSERT (iot_data_vector_size (vector) == 2)
  CU_ASSERT (iot_data_vector_get (vector, 0) == str1)
  CU_ASSERT (iot_data_vector_get (vector, 1) == str2)

  int loop = 5;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_vector_add (vector, 0, iot_data_alloc_string ("first element", IOT_DATA_REF));
  while (loop--)
  {
    iot_data_string_map_add (map, "temp", iot_data_alloc_i32 (loop));
    iot_data_vector_add (vector, 1, map);
    iot_data_add_ref (map);
    assert (iot_data_vector_get (vector, 1) == map);
  }
  iot_data_free (map);
  iot_data_free (vector);
}

static const char * test_config =
  "{"
    "\"Interval\":1000,"
    "\"Scheduler\":\"scheduler\","
    "\"ThreadPool\":\"pool\","
    "\"Topics\": [{\"Topic\":\"test/tube\",\"Priority\":10,\"Retain\":true}],"
    "\"Null\": null,"
    "\"Boolean\":true,"
    "\"Numbers\":{ \"One\":1, \"Two\":2, \"Three\":3 },"
    "\"Vector\":[ \"A\",\"B\"],"
    "\"DB\":0.5,"
    "\"Escaped\":\"Double \\\" Quote\","
    "\"Unicode\":\"\\u0003HELLO\\u0006HI\""
  "}";

static void test_data_compress (void)
{
  iot_data_t * cache = iot_data_alloc_map (IOT_DATA_MULTI);
  iot_data_t * map1 = iot_data_from_json (test_config);
  iot_data_t * map2 = iot_data_from_json (test_config);
  iot_data_t * l1 = iot_data_alloc_list ();
  iot_data_list_head_push (l1, iot_data_alloc_string ("Interval", IOT_DATA_REF));
  iot_data_string_map_add (map2, "list", l1);
  iot_data_compress_with_cache (map1, cache);
  iot_data_compress_with_cache (map2, cache);
  iot_data_free (map1);
  iot_data_compress (map2);
  iot_data_free (map2);
  iot_data_free (cache);
}

static void test_data_infinite (void)
{
  iot_data_t * f = iot_data_alloc_from_string (IOT_DATA_FLOAT32, "1e500");
  iot_data_t * d = iot_data_alloc_from_string (IOT_DATA_FLOAT64, "1e500");
  char * json = iot_data_to_json (f);
  CU_ASSERT (strcmp (json, "1e400") == 0) // Use 1e400 as a representation of a float infinite value
  free (json);
  json = iot_data_to_json (d);
  CU_ASSERT (strcmp (json, "1e800") == 0) // Use 1e800 as a representation of a double infinite value
  free (json);
  iot_data_free (f);
  iot_data_free (d);
}

static void test_data_address (void)
{
  uint32_t * ui32ptr;
  char ** strptr;
  uint8_t buff [4] = { 0, 1, 2 ,3 };
  iot_data_t * data = iot_data_alloc_array (buff, sizeof (buff), IOT_DATA_UINT8, IOT_DATA_REF);
  CU_ASSERT (iot_data_address (data) == buff)
  iot_data_free (data);
  data = iot_data_alloc_ui32 (5u);
  ui32ptr = (uint32_t*) iot_data_address (data);
  *ui32ptr = 6u;
  CU_ASSERT (iot_data_ui32 (data) == 6u)
  iot_data_free (data);
  data = iot_data_alloc_string ("Hello", IOT_DATA_COPY);
  strptr = (char**) iot_data_address (data);
  *strptr[0] = 'h';
  CU_ASSERT (strcmp (iot_data_string (data), "hello") == 0)
  iot_data_free (data);
}

static void test_data_name_type (void)
{
  CU_ASSERT (iot_data_name_type ("int8") == IOT_DATA_INT8)
  CU_ASSERT (iot_data_name_type ("uint8") == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_name_type ("int16") == IOT_DATA_INT16)
  CU_ASSERT (iot_data_name_type ("uint16") == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_name_type ("int32") == IOT_DATA_INT32)
  CU_ASSERT (iot_data_name_type ("uint32") == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_name_type ("int64") == IOT_DATA_INT64)
  CU_ASSERT (iot_data_name_type ("uint64") == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_name_type ("float32") == IOT_DATA_FLOAT32)
  CU_ASSERT (iot_data_name_type ("float64") == IOT_DATA_FLOAT64)
  CU_ASSERT (iot_data_name_type ("bool") == IOT_DATA_BOOL)
  CU_ASSERT (iot_data_name_type ("pointer") == IOT_DATA_POINTER)
  CU_ASSERT (iot_data_name_type ("string") == IOT_DATA_STRING)
  CU_ASSERT (iot_data_name_type ("null") == IOT_DATA_NULL)
  CU_ASSERT (iot_data_name_type ("binary") == IOT_DATA_BINARY)
  CU_ASSERT (iot_data_name_type ("array") == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_name_type ("map") == IOT_DATA_MAP)
  CU_ASSERT (iot_data_name_type ("vector") == IOT_DATA_VECTOR)
  CU_ASSERT (iot_data_name_type ("list") == IOT_DATA_LIST)
  CU_ASSERT (iot_data_name_type ("multi") == IOT_DATA_MULTI)
  CU_ASSERT (iot_data_name_type ("invalid") == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_name_type ("dummy") == IOT_DATA_INVALID)
}

static void test_data_type_string (void)
{
  const char * str;
  str = iot_data_type_string (IOT_DATA_INT8);
  CU_ASSERT (str && strcmp (str, "Int8") == 0)
  str = iot_data_type_string (IOT_DATA_UINT8);
  CU_ASSERT (str && strcmp (str, "UInt8") == 0)
  str = iot_data_type_string (IOT_DATA_INT16);
  CU_ASSERT (str && strcmp (str, "Int16") == 0)
  str = iot_data_type_string (IOT_DATA_UINT16);
  CU_ASSERT (str && strcmp (str, "UInt16") == 0)
  str = iot_data_type_string (IOT_DATA_INT32);
  CU_ASSERT (str && strcmp (str, "Int32") == 0)
  str = iot_data_type_string (IOT_DATA_UINT32);
  CU_ASSERT (str && strcmp (str, "UInt32") == 0)
  str = iot_data_type_string (IOT_DATA_INT64);
  CU_ASSERT (str && strcmp (str, "Int64") == 0)
  str = iot_data_type_string (IOT_DATA_UINT64);
  CU_ASSERT (str && strcmp (str, "UInt64") == 0)
  str = iot_data_type_string (IOT_DATA_FLOAT32);
  CU_ASSERT (str && strcmp (str, "Float32") == 0)
  str = iot_data_type_string (IOT_DATA_FLOAT64);
  CU_ASSERT (str && strcmp (str, "Float64") == 0)
  str = iot_data_type_string (IOT_DATA_BOOL);
  CU_ASSERT (str && strcmp (str, "Bool") == 0)
  str = iot_data_type_string (IOT_DATA_STRING);
  CU_ASSERT (str && strcmp (str, "String") == 0)
  str = iot_data_type_string (IOT_DATA_NULL);
  CU_ASSERT (str && strcmp (str, "Null") == 0)
  str = iot_data_type_string (IOT_DATA_ARRAY);
  CU_ASSERT (str && strcmp (str, "Array") == 0)
  str = iot_data_type_string (IOT_DATA_MAP);
  CU_ASSERT (str && strcmp (str, "Map") == 0)
  str = iot_data_type_string (IOT_DATA_VECTOR);
  CU_ASSERT (str && strcmp (str, "Vector") == 0)
  str = iot_data_type_string (IOT_DATA_POINTER);
  CU_ASSERT (str && strcmp (str, "Pointer") == 0)
  str = iot_data_type_string (IOT_DATA_LIST);
  CU_ASSERT (str && strcmp (str, "List") == 0)
}

static void test_data_from_string (void)
{
  iot_data_t * data;
  data = iot_data_alloc_from_string (IOT_DATA_INT8, "-6");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_i8 (data) == -6)
  CU_ASSERT (iot_data_string (data) == NULL)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT8, "4");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_ui8 (data) == 4)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT16, "-22222");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_i16 (data) == -22222)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT16, "44444");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_ui16 (data) == 44444)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT32, "-2222222");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_i32 (data) == -2222222)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT32, "4444444");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_ui32 (data) == 4444444)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT64, "-22222222222222");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT64)
  CU_ASSERT (iot_data_i64 (data) == -22222222222222)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT64, "44444444444444");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_ui64 (data) == 44444444444444)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_FLOAT32, "0.2");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT32)
  CU_ASSERT (iot_data_f32 (data) < 0.200001 && iot_data_f32 (data) > 0.1999999)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_FLOAT64, "0.4");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT64)
  CU_ASSERT (iot_data_f64 (data) < 0.4000001 && iot_data_f64 (data) > 0.3999999)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_BOOL, "true");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL)
  CU_ASSERT (iot_data_bool (data))
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_BOOL, "True");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL)
  CU_ASSERT (iot_data_bool (data))
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_BOOL, "False");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL)
  CU_ASSERT (! iot_data_bool (data))
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_BOOL, "X");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL)
  CU_ASSERT (! iot_data_bool (data))
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_STRING, "Wibble");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_data_string (data), "Wibble") == 0)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_STRING, "");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_data_string (data), "") == 0)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_ARRAY, "XXX");
  CU_ASSERT (data == NULL)
  data = iot_data_alloc_from_string (IOT_DATA_MAP, "XXX");
  CU_ASSERT (data == NULL)
  data = iot_data_alloc_from_string (IOT_DATA_VECTOR, "XXX");
  CU_ASSERT (data == NULL)

  // Integer range tests
  data = iot_data_alloc_from_string (IOT_DATA_UINT8, "0");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_ui8 (data) == 0)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT8, "255");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_ui8 (data) == 255)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT8, "-128");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_i8 (data) == -128)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT8, "127");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_i8 (data) == 127)
  iot_data_free (data);

  data = iot_data_alloc_from_string (IOT_DATA_UINT16, "0");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_ui16 (data) == 0)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT16, "65535");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_ui16 (data) == 65535)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT16, "-32768");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_i16 (data) == -32768)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT16, "32767");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_i16 (data) == 32767)
  iot_data_free (data);

  data = iot_data_alloc_from_string (IOT_DATA_UINT32, "0");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_ui32 (data) == 0)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT32, "4294967295");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_ui32 (data) == 4294967295U)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT32, "-2147483648");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_i32 (data) == -2147483647-1)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT32, "2147483647");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_i32 (data) == 2147483647)
  iot_data_free (data);

  data = iot_data_alloc_from_string (IOT_DATA_UINT64, "0");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_ui64 (data) == 0)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT64, "18446744073709551615");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_ui64 (data) == 18446744073709551615ULL)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT64, "-9223372036854775808");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT64)
  CU_ASSERT (iot_data_i64 (data) == -9223372036854775807LL-1)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_INT64, "9223372036854775807");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT64)
  CU_ASSERT (iot_data_i64 (data) == 9223372036854775807LL)
  iot_data_free (data);
}

static void test_data_from_strings (void)
{
  iot_data_t * data;
  data = iot_data_alloc_from_strings ("Int8", "-6");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_i8 (data) == -6)
  iot_data_free (data);
}

static void test_data_from_base64 (void)
{
  const char * greeting = "Hello World!\n";
  iot_data_t * data;
  const uint8_t * bytes;
  uint32_t len;
  data = iot_data_alloc_array_from_base64 ("SGVsbG8gV29ybGQhCg==");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (data) == IOT_DATA_UINT8)
  bytes = iot_data_address (data);
  len = iot_data_array_length (data);
  CU_ASSERT (len == strlen (greeting))
  CU_ASSERT (strncmp ((char*) bytes, greeting, len) == 0)
  iot_data_free (data);
}

static void test_data_map_base64_to_array (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_string ("SGVsbG8gV29ybGQhCg==", IOT_DATA_REF);
  const iot_data_t * data;
  const void * bytes;
  uint32_t len;
  iot_data_map_add (map, key, val);
  CU_ASSERT (iot_data_map_base64_to_array (map, key))
  data = iot_data_map_get (map, key);
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  bytes = iot_data_address (data);
  len = iot_data_array_size (data);
  CU_ASSERT (len == 13)
  CU_ASSERT (strncmp ((char *) bytes, "Hello World!\n", len) == 0)
  iot_data_free (map);
}

static void test_data_increment (void)
{
  iot_data_t * data;
  data = iot_data_alloc_i8 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_i8 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_ui8 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_ui8 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_i16 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_i16 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_ui16 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_ui16 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_i32 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_i32 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_ui32 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_ui32 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_i64 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_i64 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_ui64 (1);
  iot_data_increment (data);
  CU_ASSERT (iot_data_ui64 (data) == 2)
  iot_data_free (data);
  data = iot_data_alloc_f32 (1.0f);
  iot_data_increment (data);
  CU_ASSERT (iot_data_f32 (data) == 2.0)
  iot_data_free (data);
  data = iot_data_alloc_f64 (1.0);
  iot_data_increment (data);
  CU_ASSERT (iot_data_f64 (data) == 2.0)
  iot_data_free (data);
  data = iot_data_alloc_vector (0);
  iot_data_increment (data);
  iot_data_free (data);
}

static void test_data_decrement (void)
{
  iot_data_t * data;
  data = iot_data_alloc_i8 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_i8 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_ui8 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_ui8 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_i16 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_i16 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_ui16 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_ui16 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_i32 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_i32 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_ui32 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_ui32 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_i64 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_i64 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_ui64 (2);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_ui64 (data) == 1)
  iot_data_free (data);
  data = iot_data_alloc_f32 (2.0f);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_f32 (data) == 1.0)
  iot_data_free (data);
  data = iot_data_alloc_f64 (2.0);
  iot_data_decrement (data);
  CU_ASSERT (iot_data_f64 (data) == 1.0)
  iot_data_free (data);
}

static void test_data_equal_int8 (void)
{
  iot_data_t * data1;
  data1 = iot_data_alloc_i8 (-1);

  iot_data_t * data2;
  data2 = iot_data_alloc_i8 (-1);

  CU_ASSERT (iot_data_equal (data1, data2))
  CU_ASSERT (iot_data_equal (data1, data1))

  iot_data_increment (data1);
  CU_ASSERT (!iot_data_equal (data1, data2))

  iot_data_free (data1);
  iot_data_free (data2);
}

static void test_data_equal_uint16 (void)
{
  iot_data_t * data1;
  data1 = iot_data_alloc_ui16 (1);

  iot_data_t * data2;
  data2 = iot_data_alloc_ui16 (1);

  CU_ASSERT (iot_data_equal (data1, data2))
  CU_ASSERT (iot_data_equal (data1, data1))

  iot_data_increment (data1);
  CU_ASSERT (!iot_data_equal (data1, data2))

  iot_data_free (data1);
  iot_data_free (data2);
}

static void test_data_equal_float32 (void)
{
  iot_data_t * data1;
  data1 = iot_data_alloc_f32 (1.0f);

  iot_data_t * data2;
  data2 = iot_data_alloc_f32 (1.0f);

  CU_ASSERT (iot_data_equal (data1, data2))
  CU_ASSERT (iot_data_equal (data1, data1))

  iot_data_increment (data1);
  CU_ASSERT (!iot_data_equal (data1, data2))
  CU_ASSERT (!iot_data_equal (data1, data2))

  iot_data_free (data1);
  iot_data_free (data2);
}

static void test_data_equal_string (void)
{
  iot_data_t * data1;
  data1 = iot_data_alloc_string ("test1", IOT_DATA_REF);

  iot_data_t * data2;
  data2 = iot_data_alloc_string ("test1", IOT_DATA_REF);

  CU_ASSERT (iot_data_equal (data1, data2))
  iot_data_free (data1);
  iot_data_free (data2);

  data1 = iot_data_alloc_string ("test2", IOT_DATA_COPY);
  data2 = iot_data_alloc_string ("test2", IOT_DATA_COPY);

  CU_ASSERT (iot_data_equal (data1, data2))

  iot_data_free (data1);
  iot_data_free (data2);
}

static void test_data_equal_null (void)
{
  iot_data_t * data = iot_data_alloc_ui8 (1u);
  CU_ASSERT (iot_data_equal (NULL, NULL))
  CU_ASSERT (iot_data_equal (data, data))
  CU_ASSERT (! iot_data_equal (data, NULL))
  CU_ASSERT (! iot_data_equal (NULL, data))

  iot_data_t * n1 = iot_data_alloc_null ();
  iot_data_t * n2 = iot_data_alloc_null ();
  CU_ASSERT (iot_data_equal (n1, n2))
  CU_ASSERT (! iot_data_equal (n1, data))
  CU_ASSERT (! iot_data_equal (n1, NULL))

  iot_data_free (data);
  iot_data_free (n1);
  iot_data_free (n2);
}

static void test_data_equal_vector_ui8 (void)
{
  uint8_t vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);
  iot_data_t *vector3 = iot_data_alloc_vector (1);


  while (vector_index < 5)
  {
    iot_data_vector_add (vector1, vector_index, iot_data_alloc_ui8 (vector_index));
    iot_data_vector_add (vector2, vector_index, iot_data_alloc_ui8 (vector_index));
    vector_index++;
  }
  iot_data_vector_add (vector3, 0, iot_data_alloc_ui8 (0));

  CU_ASSERT (iot_data_equal (vector1, vector2))
  CU_ASSERT (! iot_data_equal (vector1, vector3))
  iot_data_free (vector1);
  iot_data_free (vector2);
  iot_data_free (vector3);
}

static void test_data_equal_vector_ui8_refcount (void)
{
  uint32_t vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);

  while (vector_index < 5)
  {
    iot_data_t * value = iot_data_alloc_ui8 ((uint8_t) vector_index);

    iot_data_vector_add (vector1, vector_index, value);
    iot_data_vector_add (vector2, vector_index, iot_data_add_ref (value));
    vector_index++;
  }

  CU_ASSERT (iot_data_equal (vector1, vector2))
  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_unequal_vector_ui8 (void)
{
  uint32_t vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);

  while (vector_index < 5)
  {
    iot_data_vector_add (vector1, vector_index, iot_data_alloc_ui8 ((uint8_t) vector_index));
    iot_data_vector_add (vector2, vector_index, iot_data_alloc_ui8 ((uint8_t) (vector_index+1)));

    vector_index++;
  }

  CU_ASSERT (!iot_data_equal (vector1, vector2))
  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_equal_vector_string (void)
{
  const char *strs [2] = { "Test", "Tube" };

  iot_data_t * vector1 = iot_data_alloc_vector (2);
  iot_data_t * vector2 = iot_data_alloc_vector (2);

  iot_data_t * str1 = iot_data_alloc_string (strs[0], IOT_DATA_REF);
  iot_data_t * str2 = iot_data_alloc_string (strs[1], IOT_DATA_REF);

  iot_data_t * str3 = iot_data_alloc_string (strs[0], IOT_DATA_REF);
  iot_data_t * str4 = iot_data_alloc_string (strs[1], IOT_DATA_REF);

  iot_data_vector_add (vector1, 0, str1);
  iot_data_vector_add (vector1, 1, str2);

  iot_data_vector_add (vector2, 0, str3);
  iot_data_vector_add (vector2, 1, str4);

  CU_ASSERT (iot_data_equal (vector1, vector2))
  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_equal_array (void)
{
  uint8_t data [4] = { 0, 1, 2, 3 };
  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);

  CU_ASSERT (iot_data_equal (array1, array2))
  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_equal_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val1 = iot_data_alloc_ui32 (66u);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_t * val2 = iot_data_alloc_ui32 (66u);
  iot_data_t * key2 = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_map_add (data_map1, key1, val1);
  iot_data_map_add (data_map2, key2, val2);

  val1 = iot_data_alloc_ui32 (77u);
  key1 = iot_data_alloc_string ("key2", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, val1);

  val2 = iot_data_alloc_ui32 (77u);
  key2 = iot_data_alloc_string ("key2", IOT_DATA_REF);
  iot_data_map_add (data_map2, key2, val2);

  CU_ASSERT (iot_data_map_key_is_of_type (data_map1, IOT_DATA_STRING))
  CU_ASSERT (iot_data_is_of_type (data_map1, IOT_DATA_MAP))
  CU_ASSERT (! iot_data_is_of_type (NULL, IOT_DATA_MAP))
  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_equal_map_refcount (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val = iot_data_alloc_ui32 (66u);
  iot_data_t * key = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_map_add (data_map1, key, val);
  const iot_data_t * tmp = iot_data_add_ref (key);
  CU_ASSERT (tmp == key)
  iot_data_add_ref (val);

  iot_data_map_add (data_map2, key, val);

  val = iot_data_alloc_ui32 (77u);
  key = iot_data_alloc_string ("key2", IOT_DATA_REF);

  iot_data_map_add (data_map1, key, val);
  iot_data_map_add (data_map2, iot_data_add_ref (key), iot_data_add_ref (val));

  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_unequal_map_size (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val = iot_data_alloc_ui32 (66u);
  iot_data_t * key = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_map_add (data_map1, key, val);
  iot_data_add_ref (key);
  iot_data_add_ref (val);

  iot_data_map_add (data_map2, key, val);

  val = iot_data_alloc_ui32 (77u);
  key = iot_data_alloc_string ("key2", IOT_DATA_REF);

  iot_data_map_add (data_map1, key, val);
  iot_data_add_ref (key);
  iot_data_add_ref (val);

  iot_data_map_add (data_map2, key, val);

  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  val = iot_data_alloc_ui32 (88u);
  key = iot_data_alloc_string ("key3", IOT_DATA_REF);

  iot_data_map_add (data_map1, key, val);

  CU_ASSERT (iot_data_map_size (data_map1) == 3)
  CU_ASSERT (iot_data_map_size (data_map2) == 2)
  CU_ASSERT (!iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}


static void test_data_unequal_key_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val1 = iot_data_alloc_ui32 (66u);
  iot_data_t * val2 = iot_data_alloc_ui32 (66u);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_t * key2 = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_map_add (data_map1, key1, val1);
  iot_data_map_add (data_map2, key2, val2);

  val1 = iot_data_alloc_ui32 (77u);
  val2 = iot_data_alloc_ui32 (77u);
  key1 = iot_data_alloc_string ("key3", IOT_DATA_REF);
  key2 = iot_data_alloc_string ("key4", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, val1);
  iot_data_map_add (data_map2, key2, val2);
  
  CU_ASSERT (!iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_unequal_value_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val1 = iot_data_alloc_ui32 (66u);
  iot_data_t * val2 = iot_data_alloc_ui32 (66u);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_t * key2 = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, val1);
  iot_data_map_add (data_map2, key2, val2);

  val1 = iot_data_alloc_ui32 (77u);
  key1 = iot_data_alloc_string ("key2", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, val1);

  key2 = iot_data_alloc_string ("key2", IOT_DATA_REF);
  val2 = iot_data_alloc_ui32 (88u);
  iot_data_map_add (data_map2, key2, val2);
  
  CU_ASSERT (!iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}
	
static void test_data_equal_nested_vector (void)
{
  iot_data_t *vector1 = iot_data_alloc_vector (2);
  iot_data_t *vector2 = iot_data_alloc_vector (2);

  iot_data_t *vector3 = iot_data_alloc_vector (2);
  iot_data_t *vector4 = iot_data_alloc_vector (2);

  iot_data_t *vector5 = iot_data_alloc_vector (2);
  iot_data_t *vector6 = iot_data_alloc_vector (2);

  iot_data_vector_add (vector3, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector3, 1, iot_data_alloc_ui8(20));
  iot_data_vector_add (vector4, 0, iot_data_alloc_ui8(30));
  iot_data_vector_add (vector4, 1, iot_data_alloc_ui8(40));

  iot_data_vector_add (vector5, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector5, 1, iot_data_alloc_ui8(20));
  iot_data_vector_add (vector6, 0, iot_data_alloc_ui8(30));
  iot_data_vector_add (vector6, 1, iot_data_alloc_ui8(40));

  iot_data_vector_add (vector1, 0, vector3);
  iot_data_vector_add (vector1, 1, vector4);

  iot_data_vector_add (vector2, 0, vector5);
  iot_data_vector_add (vector2, 1, vector6);
  
  CU_ASSERT (iot_data_equal (vector1, vector2))

  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_unequal_nested_vector (void)
{
  iot_data_t *vector1 = iot_data_alloc_vector (2);
  iot_data_t *vector2 = iot_data_alloc_vector (2);

  iot_data_t *vector3 = iot_data_alloc_vector (2);
  iot_data_t *vector4 = iot_data_alloc_vector (2);

  iot_data_t *vector5 = iot_data_alloc_vector (2);
  iot_data_t *vector6 = iot_data_alloc_vector (2);

  iot_data_vector_add (vector3, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector3, 1, iot_data_alloc_ui8(20));
  iot_data_vector_add (vector4, 0, iot_data_alloc_ui8(30));
  iot_data_vector_add (vector4, 1, iot_data_alloc_ui8(40));

  iot_data_vector_add (vector5, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector5, 1, iot_data_alloc_ui8(20));
  iot_data_vector_add (vector6, 0, iot_data_alloc_ui8(30));
  iot_data_vector_add (vector6, 1, iot_data_alloc_ui8(50));

  iot_data_vector_add (vector1, 0, vector3);
  iot_data_vector_add (vector1, 1, vector4);

  iot_data_vector_add (vector2, 0, vector5);
  iot_data_vector_add (vector2, 1, vector6);
  
  CU_ASSERT (!iot_data_equal (vector1, vector2))

  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_equal_vector_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * key1 = iot_data_alloc_string ("Vector", IOT_DATA_REF);

  iot_data_t * vector1 = iot_data_alloc_vector (2);
  iot_data_vector_add (vector1, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector1, 1, iot_data_alloc_ui8(20));

  iot_data_map_add (data_map1, key1, vector1);
  iot_data_add_ref (key1);
  iot_data_add_ref (vector1);

  iot_data_t * key2 = iot_data_alloc_string ("String", IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_string ("test", IOT_DATA_REF);

  iot_data_map_add (data_map1, key2, val);
  iot_data_add_ref (key2);
  iot_data_add_ref (val);

  iot_data_map_add (data_map2, key1, vector1);
  iot_data_map_add (data_map2, key2, val);

  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_unequal_vector_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * data_map2 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * key1 = iot_data_alloc_string ("Vector", IOT_DATA_REF);

  iot_data_t * vector1 = iot_data_alloc_vector (2);
  iot_data_vector_add (vector1, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector1, 1, iot_data_alloc_ui8(20));

  iot_data_map_add (data_map1, key1, vector1);
  iot_data_add_ref (key1);
  iot_data_add_ref (vector1);

  iot_data_t * key2 = iot_data_alloc_string ("String", IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_string ("test", IOT_DATA_REF);

  iot_data_map_add (data_map1, key2, val);
  iot_data_add_ref (key2);

  iot_data_map_add (data_map2, key1, vector1);

  val = iot_data_alloc_string ("test2", IOT_DATA_REF);
  iot_data_map_add (data_map2, key2, val);

  CU_ASSERT (!iot_data_equal (data_map1, data_map2))
  CU_ASSERT (!iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_metadata (void)
{
  iot_data_t * data = iot_data_alloc_i8 (-128);
  iot_data_t * md1 = iot_data_alloc_string ("Hello", IOT_DATA_REF);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_t * md2 = iot_data_alloc_string ("John", IOT_DATA_REF);
  iot_data_t * key2 = iot_data_alloc_string ("key2", IOT_DATA_REF);

  iot_data_set_metadata (data, md1, key1);
  iot_data_set_metadata (data, md2, key2);
  const iot_data_t * metadata1 = iot_data_get_metadata (data, key1);

  CU_ASSERT (metadata1 != NULL)
  CU_ASSERT (iot_data_type (metadata1) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_equal (md1, metadata1))
  CU_ASSERT (iot_data_get_metadata (NULL, key1) == NULL)
  CU_ASSERT (iot_data_get_metadata (data, NULL) == NULL)
  CU_ASSERT (iot_data_get_metadata (NULL, NULL) == NULL)

  const iot_data_t * metadata2 = iot_data_get_metadata (data, key2);
  CU_ASSERT (metadata2 != NULL)
  CU_ASSERT (iot_data_type (metadata2) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_equal (md2, metadata2))

  iot_data_free (data);

  data = iot_data_alloc_ui64 (9999999999);
  CU_ASSERT (data != NULL)

  md1 = iot_data_alloc_i32 (2111111);
  CU_ASSERT (md1 != NULL)

  iot_data_set_metadata (data, md1, key1);
  metadata1 = iot_data_get_metadata (data, key1);

  CU_ASSERT (metadata1 != NULL)
  CU_ASSERT (iot_data_type (metadata1) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_equal (md1, metadata1))
  iot_data_free (data);

  data = iot_data_alloc_f32 (1.299999f);
  CU_ASSERT (data != NULL)

  md1 = iot_data_alloc_string ("qwertyQWERTY", IOT_DATA_REF);
  CU_ASSERT (md1 != NULL)

  iot_data_set_metadata (data, iot_data_add_ref (md1), key1);
  iot_data_set_metadata (data, iot_data_add_ref (md1), key1);
  metadata1 = iot_data_get_metadata (data, key1);

  CU_ASSERT (metadata1 != NULL)
  CU_ASSERT (iot_data_type (metadata1) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_equal (md1, metadata1))

  // When data copied, metadata should also be copied.
  iot_data_t * data3 = iot_data_copy (data);
  const iot_data_t * metadata3 = iot_data_get_metadata (data3, key1);

  CU_ASSERT (metadata3 != NULL)
  CU_ASSERT (iot_data_equal (metadata3, metadata1))

  iot_data_free (data);
  iot_data_free (md1);
  iot_data_free (data3);
  iot_data_free (key1);
  iot_data_free (key2);
}

static void test_data_multi_metadata (void)
{
  static iot_data_static_t key1s;
  static iot_data_static_t key2s;
  static const char * str = "Hello";
  static const void * ptr = &key1s;

  const iot_data_t * key1 = iot_data_alloc_const_pointer (&key1s, ptr);
  const iot_data_t * key2 = iot_data_alloc_const_string (&key2s, str);

  iot_data_t * data = iot_data_alloc_ui32 (123u);
  iot_data_t * md1 = iot_data_alloc_i32 (123);
  iot_data_t * md2 = iot_data_alloc_i16 (12);
  iot_data_set_metadata (data, md1, key1);
  iot_data_set_metadata (data, md2, key2);

  const iot_data_t * md = iot_data_get_metadata (data, key1);
  CU_ASSERT (md == md1)
  md = iot_data_get_metadata (data, key2);
  CU_ASSERT (md == md2)

  iot_data_free (data);
}

static bool string_match (const iot_data_t * data, const void * arg)
{
  const char * target = (const char *) arg;
  const char * val = iot_data_string (data);
  return strcmp (target, val) == 0;
}

static iot_data_t *  test_populate_vector (void)
{
  uint8_t data[3] = { 0, 1, 2};
  static const uint32_t size = 3;
  iot_data_t * vector = iot_data_alloc_vector (size);
  iot_data_t * value1 = iot_data_alloc_ui8 (data[0]);
  iot_data_t * value2 = iot_data_alloc_ui8 (data[1]);
  iot_data_t * value3 = iot_data_alloc_ui8 (data[2]);
  iot_data_vector_add (vector, 0u, value1);
  iot_data_vector_add (vector, 1u, value2);
  iot_data_vector_add (vector, 2u, value3);
  return vector;
}

static void test_data_vector_iter_next (void)
{
  iot_data_t * vector = test_populate_vector ();
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);

  uint8_t index = 0;
  while (iot_data_vector_iter_next (&iter))
  {
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
    index++;
  }
  index = 0;
  while (iot_data_vector_iter_next (&iter))
  {
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
    index++;
  }
  iot_data_free (vector);
}

static void test_data_vector_typed_iter (void)
{
  const char * str = "xxx";
  iot_data_t * vector = iot_data_alloc_typed_vector (2, IOT_DATA_MULTI);
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);
  iot_data_vector_add (vector, 0u, iot_data_alloc_pointer (vector, NULL));
  iot_data_vector_add (vector, 1u, iot_data_alloc_string (str, IOT_DATA_REF));
  CU_ASSERT (iot_data_vector_iter_next (&iter))
  CU_ASSERT (iot_data_vector_iter_pointer_value (&iter) == vector)
  CU_ASSERT (iot_data_vector_iter_next (&iter))
  CU_ASSERT (iot_data_vector_iter_string_value (&iter) == str)
  CU_ASSERT (iot_data_vector_type (vector) == IOT_DATA_MULTI)
  iot_data_free (vector);
}

static void test_data_vector_iter_prev (void)
{
  iot_data_t * vector = test_populate_vector ();
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);

  uint32_t index = iot_data_vector_size (vector) - 1;
  while (iot_data_vector_iter_prev (&iter))
  {
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
    index--;
  }
  index = iot_data_vector_size (vector) - 1;
  while (iot_data_vector_iter_prev (&iter))
  {
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
    index--;
  }
  iot_data_free (vector);
}

static void test_data_vector_iters (void)
{
  iot_data_t * vector = test_populate_vector ();
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);

  uint32_t index = 0;
  iot_data_vector_iter_next (&iter);
  CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
  CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
  CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
  index++;
  iot_data_vector_iter_next (&iter);
  CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
  CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
  CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
  index--;
  iot_data_vector_iter_prev (&iter);
  CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
  CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
  CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == index)
  iot_data_free (vector);
}

static void test_data_vector_resize (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3u);
  iot_data_t * elem0 = iot_data_alloc_string ("test", IOT_DATA_REF);
  iot_data_t * elem1 = iot_data_alloc_string ("test1", IOT_DATA_REF);
  iot_data_t * elem2 = iot_data_alloc_string ("test2", IOT_DATA_REF);
  iot_data_vector_add (vector, 0u, elem0);
  iot_data_vector_add (vector, 1u, elem1);
  iot_data_vector_add (vector, 2u, elem2);
  iot_data_vector_resize (vector, 5u);
  CU_ASSERT (iot_data_vector_size (vector) == 5u)
  CU_ASSERT (iot_data_vector_get (vector, 0u) == elem0)
  CU_ASSERT (iot_data_vector_get (vector, 1u) == elem1)
  CU_ASSERT (iot_data_vector_get (vector, 2u) == elem2)
  CU_ASSERT (iot_data_vector_get (vector, 3u) == NULL)
  CU_ASSERT (iot_data_vector_get (vector, 4u) == NULL)
  iot_data_vector_resize (vector, 2u);
  CU_ASSERT (iot_data_vector_size (vector) == 2u)
  CU_ASSERT (iot_data_vector_get (vector, 0u) == elem0)
  CU_ASSERT (iot_data_vector_get (vector, 1u) == elem1)
  iot_data_free (vector);
}

static void test_data_vector_compact (void)
{
  uint32_t size;
  uint32_t hash1;
  uint32_t hash2;
  iot_data_t * vector = iot_data_alloc_vector (4u);
  iot_data_t * elem0 = iot_data_alloc_ui32 (0u);
  iot_data_t * elem2 = iot_data_alloc_ui32 (2u);
  iot_data_vector_add (vector, 0u, elem0);
  iot_data_vector_add (vector, 2u, elem2);
  hash1 = iot_data_hash (vector);
  size = iot_data_vector_compact (vector);
  hash2 = iot_data_hash (vector);
  CU_ASSERT (size == 2u)
  CU_ASSERT (hash1 == hash2)
  CU_ASSERT (iot_data_vector_size (vector) == 2u)
  CU_ASSERT (iot_data_vector_get (vector, 0u) == elem0)
  CU_ASSERT (iot_data_vector_get (vector, 1u) == elem2)
  iot_data_free (vector);
  vector = iot_data_alloc_vector (4u);
  size = iot_data_vector_compact (vector);
  CU_ASSERT (size == 0u)
  CU_ASSERT (iot_data_vector_size (vector) == 0u)
  iot_data_free (vector);
}

static void test_data_vector_find (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3);
  iot_data_t * elem0 = iot_data_alloc_string ("test", IOT_DATA_REF);
  iot_data_t * elem1 = iot_data_alloc_string ("test1", IOT_DATA_REF);
  iot_data_t * elem2 = iot_data_alloc_string ("test2", IOT_DATA_REF);
  iot_data_vector_add (vector, 0, elem0);
  iot_data_vector_add (vector, 1, elem1);
  iot_data_vector_add (vector, 2, elem2);
  CU_ASSERT (iot_data_vector_find (vector, string_match, "test1") == elem1)
  CU_ASSERT (iot_data_vector_find (vector, iot_data_string_cmp, "test1") == elem1)
  CU_ASSERT (iot_data_vector_find (vector, iot_data_string_cmp, "foo") == NULL)
  CU_ASSERT (iot_data_vector_find (vector, iot_data_string_cmp, NULL) == NULL)
  iot_data_free (vector);
}

static void test_data_copy_string (void)
{
  iot_data_t * src = iot_data_alloc_string ("src", IOT_DATA_REF);
  iot_data_t * dest = iot_data_copy (src);

  iot_data_free (src);

  const char *get_src = iot_data_string (dest);
  CU_ASSERT (strcmp (get_src, "src") == 0)
  iot_data_free (dest);

  src = iot_data_alloc_string ("src", IOT_DATA_COPY);
  dest = iot_data_copy (src);

  CU_ASSERT (iot_data_equal (src, dest))

  iot_data_free (src);
  iot_data_free (dest);
}

static void test_data_copy_ui8 (void)
{
  iot_data_t * src = iot_data_alloc_ui8 (5);
  iot_data_t * dest = iot_data_copy (src);

  iot_data_free (src);

  uint8_t get_src = iot_data_ui8 (dest);
  CU_ASSERT (get_src == 5)
  iot_data_free (dest);
}

static void test_data_copy_i8 (void)
{
  iot_data_t * src = iot_data_alloc_i8 (-128);
  iot_data_t * dest = iot_data_copy (src);

  CU_ASSERT (iot_data_i8 (dest) == -128)

  iot_data_decrement (dest);
  CU_ASSERT (iot_data_i8 (dest) == 127)
  CU_ASSERT (iot_data_i8 (src) == -128)

  iot_data_free (src);
  iot_data_free (dest);
}

static void test_data_copy_uint16 (void)
{
  iot_data_t * src = iot_data_alloc_ui16 (10);
  iot_data_t * dest = iot_data_copy (src);

  CU_ASSERT (iot_data_ui16 (dest) == 10)

  iot_data_increment (dest);
  CU_ASSERT (iot_data_ui16 (dest) == 11)
  CU_ASSERT (iot_data_ui16 (src) == 10)

  iot_data_free (src);
  iot_data_free (dest);
}

static void test_data_copy_float64 (void)
{
  iot_data_t * src = iot_data_alloc_f64 (-123.45f);
  iot_data_t * dest = iot_data_copy (src);

  iot_data_free (src);

  double get_src = iot_data_f64 (dest);
  CU_ASSERT (get_src == -123.45f)
  iot_data_free (dest);
}

static void test_data_copy_null (void)
{
  CU_ASSERT (iot_data_copy (NULL) == NULL)
}

static void test_data_copy_array (void)
{
  uint8_t data [4] = { 0, 1, 2, 3 };
  uint8_t data1 [5] = { 10, 20, 30, 40, 50 };

  iot_data_t * src = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * dest = iot_data_copy (src);

  CU_ASSERT (iot_data_type (dest) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_equal (src, dest))
  iot_data_free (src);
  iot_data_free (dest);

  src = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT8, IOT_DATA_COPY);
  dest = iot_data_copy (src);

  CU_ASSERT (iot_data_type (dest) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_equal (src, dest))
  iot_data_free (src);
  iot_data_free (dest);

  src = iot_data_alloc_array (calloc (1, sizeof (data)), 4, IOT_DATA_UINT8, IOT_DATA_TAKE);
  dest = iot_data_copy (src);

  CU_ASSERT (iot_data_type (dest) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_equal (src, dest))
  iot_data_free (src);
  iot_data_free (dest);
}

static void test_data_copy_array_chars (void)
{
  uint8_t data [5] = { 'H', 'e', 'l', 'l', 'o' };

  iot_data_t * src = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * dest = iot_data_copy (src);

  CU_ASSERT (iot_data_type (dest) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_equal (src, dest))
  iot_data_free (src);
  iot_data_free (dest);

  src = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_COPY);
  dest = iot_data_copy (src);

  CU_ASSERT (iot_data_type (dest) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_equal (src, dest))
  iot_data_free (src);
  iot_data_free (dest);
}

static void test_data_copy_vector_ui8 (void)
{
  uint32_t index = 0;
  iot_data_t * vector1 = iot_data_alloc_vector (5);

  while (index < 5)
  {
    iot_data_vector_add (vector1, index, iot_data_alloc_ui8 ((uint8_t) index));
    index++;
  }

  iot_data_t * vector2 = iot_data_copy (vector1);
  CU_ASSERT (iot_data_equal (vector1, vector2))

  iot_data_vector_add (vector2, 0, iot_data_alloc_ui8 (10));
  CU_ASSERT (!iot_data_equal (vector1, vector2))

  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_copy_vector_strings (void)
{
  const char *strs [2] = { "Test", "Tube" };

  iot_data_t * vector1 = iot_data_alloc_vector (2);
  iot_data_t * str1 = iot_data_alloc_string (strs[0], IOT_DATA_REF);
  iot_data_t * str2 = iot_data_alloc_string (strs[1], IOT_DATA_REF);

  iot_data_vector_add (vector1, 0, str1);
  iot_data_vector_add (vector1, 1, str2);

  iot_data_t *vector2 = iot_data_copy (vector1);

  // vector elements should not point to same address
  CU_ASSERT (iot_data_vector_get (vector1,0) != iot_data_vector_get (vector2,0))
  CU_ASSERT (iot_data_vector_get (vector1,1) != iot_data_vector_get (vector2,1))
  CU_ASSERT (iot_data_equal (vector1, vector2))

  iot_data_vector_add (vector2, 0, iot_data_alloc_string ("change", IOT_DATA_REF));
  CU_ASSERT (iot_data_vector_get (vector1,0) != iot_data_vector_get (vector2,0))
  CU_ASSERT (!iot_data_equal (vector1, vector2))

  iot_data_free (vector1);
  iot_data_free (vector2);

  iot_data_t * vector3 = iot_data_alloc_vector (2);
  iot_data_t * str3 = iot_data_alloc_string (strs[0], IOT_DATA_COPY);
  iot_data_t * str4 = iot_data_alloc_string (strs[1], IOT_DATA_COPY);

  iot_data_vector_add (vector3, 0, str3);
  iot_data_vector_add (vector3, 1, str4);

  iot_data_t *vector4 = iot_data_copy (vector3);

  //vector elements should point to different addresses
  CU_ASSERT (iot_data_vector_get (vector3,0) != iot_data_vector_get (vector4,0))
  CU_ASSERT (iot_data_vector_get (vector3,1) != iot_data_vector_get (vector4,1))

  CU_ASSERT (iot_data_equal (vector3, vector4))

  iot_data_vector_add (vector3, 0, iot_data_alloc_string ("change2", IOT_DATA_REF));
  CU_ASSERT (!iot_data_equal (vector3, vector4))
  iot_data_free (vector4);

  vector4 = iot_data_copy (vector3);

  CU_ASSERT (iot_data_vector_get (vector3,0) != iot_data_vector_get (vector4,0))
  CU_ASSERT (iot_data_vector_get (vector3,1) != iot_data_vector_get (vector4,1))

  iot_data_free (vector3);
  iot_data_free (vector4);
}

static void test_data_copy_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val1 = iot_data_alloc_ui32 (66u);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, val1);

  val1 = iot_data_alloc_ui32 (77u);
  key1 = iot_data_alloc_string ("key2", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, val1);

  val1 = iot_data_alloc_ui32 (88u);
  key1 = iot_data_alloc_string ("key3", IOT_DATA_COPY);
  iot_data_map_add (data_map1, key1, val1);

  val1 = iot_data_alloc_ui32 (99u);
  key1 = iot_data_alloc_string ("key4-this-is-a-very-very-very-very-yes-still-some-more-long-key", IOT_DATA_COPY);
  iot_data_map_add (data_map1, key1, val1);

  iot_data_t * data_map2 = iot_data_copy (data_map1);

  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_copy_map_update (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val1 = iot_data_alloc_ui32 (66u);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_map_add (data_map1, key1, val1);

  val1 = iot_data_alloc_ui32 (77u);
  key1 = iot_data_alloc_string ("key2", IOT_DATA_REF);

  iot_data_map_add (data_map1, key1, val1);

  iot_data_t * data_map2 = iot_data_copy (data_map1);
  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  //update data_map2
  val1 = iot_data_alloc_ui32 (88u);
  key1 = iot_data_alloc_string ("key3", IOT_DATA_REF);
  iot_data_map_add (data_map2, key1, val1);

  CU_ASSERT (iot_data_map_size(data_map1) == 2)
  CU_ASSERT (iot_data_map_size(data_map2) == 3)
  CU_ASSERT (!iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_copy_map_update_value (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * val1 = iot_data_alloc_ui32 (66u);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);

  iot_data_map_add (data_map1, key1, val1);

  val1 = iot_data_alloc_ui32 (77u);
  key1 = iot_data_alloc_string ("key2", IOT_DATA_COPY);
  iot_data_add_ref (key1);

  iot_data_map_add (data_map1, key1, val1);

  iot_data_t * data_map2 = iot_data_copy (data_map1);
  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  //update the value to the existing key
  val1 = iot_data_alloc_ui32 (88u);
  iot_data_map_add (data_map2, key1, val1);

  CU_ASSERT (!iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_data_copy_nested_vector (void)
{
  iot_data_t *vector1 = iot_data_alloc_vector (2);
  iot_data_t *vector3 = iot_data_alloc_vector (2);
  iot_data_t *vector4 = iot_data_alloc_vector (2);

  iot_data_vector_add (vector3, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector3, 1, iot_data_alloc_ui8(20));
  iot_data_vector_add (vector4, 0, iot_data_alloc_ui8(30));
  iot_data_vector_add (vector4, 1, iot_data_alloc_ui8(40));

  iot_data_vector_add (vector1, 0, vector3);
  iot_data_vector_add (vector1, 1, vector4);

  iot_data_t *vector2 = iot_data_copy (vector1);

  CU_ASSERT (iot_data_hash (vector1) == iot_data_hash (vector2))
  CU_ASSERT (iot_data_equal (vector1, vector2))

  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_copy_map_base64_to_array (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_string ("SGVsbG8gV29ybGQhCg==", IOT_DATA_REF);

  const iot_data_t * data;
  const uint8_t * bytes;
  uint32_t len;
  iot_data_map_add (map, key, val);

  iot_data_t *dest_map = iot_data_copy (map);

  CU_ASSERT (iot_data_map_base64_to_array (dest_map, key))
  data = iot_data_map_get (dest_map, key);
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  bytes = iot_data_address (data);
  len = iot_data_array_size (data);
  CU_ASSERT (len == 13)
  CU_ASSERT (strncmp ((char *) bytes, "Hello World!\n", len) == 0)
  iot_data_free (map);
  iot_data_free (dest_map);
}

static void test_data_copy_vector_map (void)
{
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key1 = iot_data_alloc_string ("Array", IOT_DATA_REF);

  iot_data_t * vector1 = iot_data_alloc_vector (2);
  iot_data_vector_add (vector1, 0, iot_data_alloc_ui8(10));
  iot_data_vector_add (vector1, 1, iot_data_alloc_ui8(20));

  iot_data_map_add (data_map1, key1, vector1);

  iot_data_t * key2 = iot_data_alloc_string ("String", IOT_DATA_REF);
  iot_data_t * vector2 = iot_data_alloc_vector (3);
  iot_data_vector_add (vector2, 0, iot_data_alloc_ui8(30));
  iot_data_vector_add (vector2, 1, iot_data_alloc_ui8(40));
  iot_data_vector_add (vector2, 2, iot_data_alloc_ui8(50));

  iot_data_map_add (data_map1, key2, vector2);

  iot_data_t * data_map2 = iot_data_copy (data_map1);
  CU_ASSERT (iot_data_equal (data_map1, data_map2))

  iot_data_free (data_map1);
  iot_data_free (data_map2);
}

static void test_map_size (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  CU_ASSERT (!iot_data_map_size (map))

  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_t * key = iot_data_alloc_string ("element1", IOT_DATA_REF);

  iot_data_map_add (map, key, val);
  iot_data_add_ref (key);

  CU_ASSERT (iot_data_map_size (map) == 1)

  /* update value for the same key */
  val = iot_data_alloc_ui32 (2u);
  iot_data_map_add (map, key, val);

  CU_ASSERT (iot_data_map_size (map) == 1)

  iot_data_string_map_add (map, "element2", iot_data_alloc_string ("data", IOT_DATA_REF));
  CU_ASSERT (iot_data_map_size (map) == 2)

  iot_data_free (map);
}

static void test_map_get (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key = iot_data_alloc_string ("Key", IOT_DATA_REF);
  iot_data_map_add (map, key, iot_data_alloc_ui32 (23u));
  const iot_data_t * val = iot_data_map_get_typed (map, key, IOT_DATA_UINT32);
  CU_ASSERT (val != NULL)
  val = iot_data_map_get_typed (map, key, IOT_DATA_INT32);
  CU_ASSERT (val == NULL)
  iot_data_free (map);
}

static void test_list_size (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_t * value;

  CU_ASSERT (iot_data_list_length (list) == 0u)
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  CU_ASSERT (iot_data_list_length (list) == 1u)
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (1u));
  CU_ASSERT (iot_data_list_length (list) == 2u)
  iot_data_list_head_push (list, iot_data_alloc_ui32 (2u));
  CU_ASSERT (iot_data_list_length (list) == 3u)
  iot_data_list_head_push (list, iot_data_alloc_ui32 (3u));
  CU_ASSERT (iot_data_list_length (list) == 4u)
  value = iot_data_list_head_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 3u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 3u)
  value = iot_data_list_tail_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 1u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 2u)
  value = iot_data_list_head_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 2u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 1u)
  value = iot_data_list_tail_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 0u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 0u)
  value = iot_data_list_tail_pop (list);
  CU_ASSERT (value == NULL)

  // Repeat test swapping head/tail push/pop

  iot_data_list_head_push (list, iot_data_alloc_ui32 (0u));
  CU_ASSERT (iot_data_list_length (list) == 1u)
  iot_data_list_head_push (list, iot_data_alloc_ui32 (1u));
  CU_ASSERT (iot_data_list_length (list) == 2u)
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));
  CU_ASSERT (iot_data_list_length (list) == 3u)
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (3u));
  CU_ASSERT (iot_data_list_length (list) == 4u)
  value = iot_data_list_tail_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 3u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 3u)
  value = iot_data_list_head_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 1u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 2u)
  value = iot_data_list_tail_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 2u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 1u)
  value = iot_data_list_head_pop (list);
  CU_ASSERT (value && iot_data_ui32 (value) == 0u)
  iot_data_free (value);
  CU_ASSERT (iot_data_list_length (list) == 0u)
  value = iot_data_list_head_pop (list);
  CU_ASSERT (value == NULL)

  iot_data_free (list);
}

static void test_list_free (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  iot_data_list_head_push (list, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));
  iot_data_free (list);
}

static void test_list_iter (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_iter_t iter;
  const iot_data_t * value;
  uint32_t counter;
  bool has_next = true;

  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));

  iot_data_list_iter (list, &iter);
  CU_ASSERT (iot_data_list_iter_has_next (&iter))
  iot_data_list_iter_next (&iter);
  CU_ASSERT (iot_data_list_iter_has_next (&iter))
  iot_data_list_iter (list, &iter);

  counter = 2;
  while (iot_data_list_iter_next (&iter))
  {
    value = iot_data_list_iter_value (&iter);
    CU_ASSERT (value && iot_data_ui32 (value) == counter)
    has_next = iot_data_list_iter_has_next (&iter);
    counter--;
  }
  CU_ASSERT (! has_next)
  CU_ASSERT (iot_data_list_iter_has_next (&iter))
  counter = 2;
  while (iot_data_list_iter_next (&iter))
  {
    value = iot_data_list_iter_value (&iter);
    CU_ASSERT (value && iot_data_ui32 (value) == counter)
    counter--;
  }
  counter = 0;
  while (iot_data_list_iter_prev (&iter))
  {
    value = iot_data_list_iter_value (&iter);
    CU_ASSERT (value && iot_data_ui32 (value) == counter)
    counter++;
  }
  counter = 0;
  while (iot_data_list_iter_prev (&iter))
  {
    value = iot_data_list_iter_value (&iter);
    CU_ASSERT (value && iot_data_ui32 (value) == counter)
    counter++;
  }
  value = iot_data_list_iter_value (&iter);
  CU_ASSERT (value == NULL)
  CU_ASSERT (iot_data_list_iter_has_next (&iter))

  iot_data_free (list);
}

static void test_list_copy (void)
{
  iot_typecode_t tc;
  iot_data_t * list = iot_data_alloc_typed_list (IOT_DATA_UINT32);
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));

  iot_data_t * list_copy = iot_data_copy (list);
  CU_ASSERT_PTR_NOT_NULL (list_copy)
  CU_ASSERT_TRUE (iot_data_equal (list, list_copy))
  iot_data_typecode (list_copy, &tc);
  CU_ASSERT (tc.type == IOT_DATA_LIST)
  CU_ASSERT (tc.element_type == IOT_DATA_UINT32)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_list_type (list) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_list_type (list_copy) == IOT_DATA_UINT32)

  iot_data_free (list);
  iot_data_free (list_copy);
}

static void test_typed_list_iter (void)
{
  static const char * str = "test";
  iot_data_t * list = iot_data_alloc_typed_list (IOT_DATA_MULTI);
  iot_data_list_iter_t iter;
  iot_data_list_tail_push (list, iot_data_alloc_pointer (list, NULL));
  iot_data_list_tail_push (list, iot_data_alloc_string (str, IOT_DATA_REF));
  iot_data_list_iter (list, &iter);
  CU_ASSERT (iot_data_list_iter_next (&iter))
  CU_ASSERT (iot_data_list_iter_string_value (&iter) == str)
  CU_ASSERT (iot_data_list_iter_next (&iter))
  CU_ASSERT (iot_data_list_iter_pointer_value (&iter) == list)
  iot_data_free (list);
}

static void test_list_iter_replace (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_iter_t iter;
  const iot_data_t * value;
  iot_data_t * old;
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));
  iot_data_list_iter (list, &iter);
  iot_data_list_iter_next (&iter);
  iot_data_list_iter_next (&iter);
  old = iot_data_list_iter_replace (&iter, iot_data_alloc_ui32 (3u));
  CU_ASSERT (old && iot_data_ui32 (old) == 1u)
  iot_data_free (old);
  value = iot_data_list_iter_value (&iter);
  CU_ASSERT (value && iot_data_ui32 (value) == 3u)
  iot_data_list_iter (list, &iter);
  iot_data_t * val = iot_data_alloc_ui32 (4u);
  old = iot_data_list_iter_replace (&iter, val);
  CU_ASSERT (old == NULL)
  iot_data_free (val);
  iot_data_free (list);
}

static iot_data_t * test_util_alloc_list (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));
  return list;
}

static void test_list_iter_remove (void)
{
  iot_data_t * list = test_util_alloc_list ();
  iot_data_list_iter_t iter;
  const iot_data_t * value;

  iot_data_list_iter (list, &iter);
  CU_ASSERT (! iot_data_list_iter_remove (&iter)) // Iterator not set
  iot_data_list_iter_next (&iter); // Iterator to first element
  iot_data_list_iter_next (&iter); // Iterator to second element
  CU_ASSERT (iot_data_list_iter_remove (&iter)); // Remove second element
  CU_ASSERT (iot_data_list_length (list) == 2u)

  iot_data_list_iter (list, &iter);
  iot_data_list_iter_next (&iter);
  value = iot_data_list_iter_value (&iter);
  CU_ASSERT (iot_data_ui32 (value) == 2u)
  iot_data_list_iter_next (&iter);
  value = iot_data_list_iter_value (&iter);
  CU_ASSERT (iot_data_ui32 (value) == 0u)
  iot_data_free (list);
}

static void test_list_iter_remove_all (void)
{
  iot_data_t * list = test_util_alloc_list ();
  iot_data_list_iter_t iter;
  iot_data_list_iter (list, &iter);
  while (iot_data_list_iter_next (&iter))
  {
    CU_ASSERT (iot_data_list_iter_remove (&iter))
  }
  iot_data_free (list);
}

static bool test_list_cmp_fn (const iot_data_t * value, const void * arg)
{
  return (iot_data_ui32 (value) == *((const uint32_t*) arg));
}

static void test_list_find (void)
{
  uint32_t val0 = 0u;
  uint32_t val1 = 1u;
  uint32_t val2 = 2u;
  uint32_t val3 = 3u;
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val0));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 ( val1));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val2));
  CU_ASSERT (iot_data_list_find (list, test_list_cmp_fn, &val0) != NULL)
  CU_ASSERT (iot_data_list_find (list, test_list_cmp_fn, &val1) != NULL)
  CU_ASSERT (iot_data_list_find (list, test_list_cmp_fn, &val2) != NULL)
  CU_ASSERT (iot_data_list_find (list, test_list_cmp_fn, &val3) == NULL)
  iot_data_free (list);
}

static void test_list_remove (void)
{
  uint32_t val0 = 0u;
  uint32_t val1 = 1u;
  uint32_t val2 = 2u;
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val0));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 ( val1));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val2));
  CU_ASSERT (iot_data_list_remove (list, test_list_cmp_fn, &val1))
  CU_ASSERT (iot_data_list_length (list) == 2u)
  CU_ASSERT (iot_data_list_remove (list, test_list_cmp_fn, &val0))
  CU_ASSERT (iot_data_list_length (list) == 1u)
  CU_ASSERT (iot_data_list_remove (list, test_list_cmp_fn, &val2))
  CU_ASSERT (iot_data_list_length (list) == 0u)
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val0));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 ( val1));
  CU_ASSERT (iot_data_list_remove (list, test_list_cmp_fn, &val1))
  CU_ASSERT (iot_data_list_length (list) == 1u)
  iot_data_free (list);
}
static void test_list_equal (void)
{
  uint32_t val0 = 0u;
  uint32_t val1 = 1u;
  uint32_t val2 = 2u;
  iot_data_t * value;
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_t * list2 = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val0));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 ( val1));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (val2));
  iot_data_list_tail_push (list2, iot_data_alloc_ui32 (val0));
  iot_data_list_tail_push (list2, iot_data_alloc_ui32 ( val1));
  iot_data_list_tail_push (list2, iot_data_alloc_ui32 (val2));
  CU_ASSERT (iot_data_equal (list, list2))
  value = iot_data_list_tail_pop (list);
  iot_data_free (value);
  CU_ASSERT (! iot_data_equal (list, list2))
  value = iot_data_list_head_pop (list2);
  iot_data_free (value);
  CU_ASSERT (! iot_data_equal (list, list2))
  iot_data_free (list);
  iot_data_free (list2);
}


static void test_data_map_remove (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key2 = iot_data_alloc_string ("2", IOT_DATA_REF);
  iot_data_t * map2;
  char * str = NULL;

  iot_data_map_add (map, iot_data_alloc_string ("1", IOT_DATA_REF), iot_data_alloc_string ("One", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("2", IOT_DATA_REF), iot_data_alloc_string ("Two", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("3", IOT_DATA_REF), iot_data_alloc_string ("Three", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("4", IOT_DATA_REF), iot_data_alloc_string ("Four", IOT_DATA_REF));

  CU_ASSERT (! iot_data_string_map_remove (map, "Nope"))
  CU_ASSERT (! iot_data_string_map_remove (map, NULL))
  CU_ASSERT (! iot_data_map_remove (map, NULL))

  CU_ASSERT (iot_data_string_map_remove (map, "3"))
  CU_ASSERT (iot_data_map_size (map) == 3u)
  CU_ASSERT (iot_data_string_map_remove (map, "1"))
  CU_ASSERT (iot_data_map_size (map) == 2u)
  CU_ASSERT (iot_data_string_map_remove (map, "4"))
  CU_ASSERT (iot_data_map_size (map) == 1u)
  iot_data_map_add (map, iot_data_alloc_string ("4", IOT_DATA_REF), iot_data_alloc_string ("Four", IOT_DATA_REF));
  str = iot_data_to_json (map);
  map2 = iot_data_from_json (str);
  CU_ASSERT (iot_data_equal (map, map2))
  CU_ASSERT (iot_data_string_map_remove (map, "4"))
  CU_ASSERT (iot_data_map_remove (map, key2))
  CU_ASSERT (iot_data_map_size (map) == 0u)

  iot_data_free (key2);
  iot_data_free (map);
  iot_data_free (map2);
  free (str);
}

static void test_data_map_get_list (void)
{
  iot_data_t * list_key = iot_data_alloc_string ("list", IOT_DATA_REF);
  iot_data_t * string_key = iot_data_alloc_string ("str", IOT_DATA_REF);
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (map, string_key, iot_data_alloc_string ("string", IOT_DATA_REF));
  iot_data_map_add (map, list_key, list);
  const iot_data_t * found = iot_data_map_get_list (map, list_key);
  CU_ASSERT (found == list)
  found = iot_data_map_get_list (map, string_key);
  CU_ASSERT (found == NULL)
  iot_data_free (map);
}

static void test_data_map_get_array (void)
{
  iot_data_t * array_key = iot_data_alloc_string ("array", IOT_DATA_REF);
  iot_data_t * string_key = iot_data_alloc_string ("str", IOT_DATA_REF);
  iot_data_t * array = iot_data_alloc_array (NULL, 0u, IOT_DATA_UINT32, IOT_DATA_REF);
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (map, string_key, iot_data_alloc_string ("string", IOT_DATA_REF));
  iot_data_map_add (map, array_key, array);
  const iot_data_t * found = iot_data_map_get_array (map, array_key);
  CU_ASSERT (found == array)
  found = iot_data_map_get_array (map, string_key);
  CU_ASSERT (found == NULL)
  iot_data_free (map);
}

static void test_data_map_iter_replace (void)
{
  iot_data_map_iter_t it;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_iter (map, &it);
  CU_ASSERT (! iot_data_map_iter_has_next (&it))

  iot_data_map_add (map, iot_data_alloc_string ("1", IOT_DATA_REF), iot_data_alloc_string ("One", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("2", IOT_DATA_REF), iot_data_alloc_string ("Two", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("3", IOT_DATA_REF), iot_data_alloc_string ("Three", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("4", IOT_DATA_REF), iot_data_alloc_null ());
  CU_ASSERT (iot_data_map_iter_has_next (&it))

  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "1"), "One") == 0)
  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "3"), "Three") == 0)
  CU_ASSERT (iot_data_string_map_get_string (map, "4") == NULL)
  iot_data_t * key = iot_data_alloc_string ("2", IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_map_get_string (map, key), "Two") == 0)
  iot_data_free (key);

  // char * json = iot_data_to_json (map);
  // printf ("JSON: %s\n", json);
  // free (json);

  iot_data_map_iter (map, &it);
  bool has_next = true;
  CU_ASSERT (iot_data_map_iter_key (&it) == NULL)
  CU_ASSERT (iot_data_map_iter_value (&it) == NULL)
  CU_ASSERT (iot_data_map_iter_string_key (&it) == NULL)
  CU_ASSERT (iot_data_map_iter_string_value (&it) == NULL)
  CU_ASSERT (iot_data_map_iter_replace_value (&it, map) == NULL)
  while (iot_data_map_iter_next (&it))
  {
    if (strcmp (iot_data_map_iter_string_key (&it), "1") == 0)
    {
      iot_data_free (iot_data_map_iter_replace_value (&it, iot_data_alloc_string ("Ace", IOT_DATA_REF)));
    }
    else if (strcmp (iot_data_map_iter_string_key (&it), "3") == 0)
    {
      iot_data_free (iot_data_map_iter_replace_value (&it, iot_data_alloc_string ("Trey", IOT_DATA_REF)));
    }
    has_next = iot_data_map_iter_has_next (&it);
  }
  CU_ASSERT (! has_next)
  CU_ASSERT (iot_data_map_iter_has_next (&it))

  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "1"), "Ace") == 0)
  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "3"), "Trey") == 0)

  iot_data_free (map);
}

static void test_data_map_iter_start_end (void)
{
  iot_data_t * map = iot_data_alloc_typed_map (IOT_DATA_UINT32, IOT_DATA_POINTER);
  iot_data_t * p1 = iot_data_alloc_pointer (&map, NULL);
  iot_data_t * p2 = iot_data_alloc_pointer (&p1, NULL);
  iot_data_t * p3 = iot_data_alloc_pointer (&p2, NULL);

  iot_data_map_add (map, iot_data_alloc_ui32 (1u), p1);
  iot_data_map_add (map, iot_data_alloc_ui32 (2u), p2);
  iot_data_map_add (map, iot_data_alloc_ui32 (3u), p3);

  const iot_data_t * val = iot_data_map_start (map);
  const void * ptr = iot_data_map_start_pointer (map);
  CU_ASSERT (val == p1)
  CU_ASSERT (ptr == &map)
  val = iot_data_map_end (map);
  ptr = iot_data_map_end_pointer (map);
  CU_ASSERT (val == p3)
  CU_ASSERT (ptr == &p2)

  iot_data_free (map);
}

static void test_data_map_typed_iter (void)
{
  const char * str = "xxx";
  iot_data_t * map = iot_data_alloc_typed_map (IOT_DATA_STRING, IOT_DATA_MULTI);
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  iot_data_string_map_add (map, "A", iot_data_alloc_pointer (map, NULL));
  iot_data_string_map_add (map, "B", iot_data_alloc_string (str, IOT_DATA_REF));
  CU_ASSERT (iot_data_map_iter_next (&iter)) // Element 0
  CU_ASSERT (iot_data_map_iter_has_next (&iter))
  CU_ASSERT (iot_data_map_iter_pointer_value (&iter) == map)
  CU_ASSERT (iot_data_map_iter_next (&iter)) // Element 1
  CU_ASSERT (iot_data_map_iter_string_value (&iter) == str)
  CU_ASSERT (! iot_data_map_iter_has_next (&iter))
  CU_ASSERT (! iot_data_map_iter_next (&iter)) // End
  CU_ASSERT (iot_data_map_iter_has_next (&iter))
  CU_ASSERT (iot_data_map_iter_next (&iter)) // Element 0
  CU_ASSERT (iot_data_map_iter_has_next (&iter))
  CU_ASSERT (iot_data_map_iter_next (&iter)) // Element 1
  CU_ASSERT (! iot_data_map_iter_has_next (&iter))
  CU_ASSERT (iot_data_map_iter_prev (&iter)) // Element 0
  CU_ASSERT (iot_data_map_iter_has_next (&iter))
  CU_ASSERT (! iot_data_map_iter_prev (&iter)) // End
  CU_ASSERT (iot_data_map_iter_has_next (&iter))
  CU_ASSERT (iot_data_map_iter_next (&iter)) // Element 0

  iot_data_free (map);
}

static void test_data_vector_iter_replace (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3u);
  iot_data_vector_add (vector, 0, iot_data_alloc_i32 (0));
  iot_data_vector_add (vector, 1, iot_data_alloc_i32 (1));
  iot_data_vector_add (vector, 2, iot_data_alloc_i32 (2));

  CU_ASSERT (iot_data_i32 (iot_data_vector_get (vector, 0)) == 0)
  CU_ASSERT (iot_data_i32 (iot_data_vector_get (vector, 1)) == 1)
  CU_ASSERT (iot_data_i32 (iot_data_vector_get (vector, 2)) == 2)

  iot_data_vector_iter_t it;
  iot_data_vector_iter (vector, &it);
  while (iot_data_vector_iter_next (&it))
  {
    iot_data_free (iot_data_vector_iter_replace_value (&it, iot_data_alloc_i32 (iot_data_vector_iter_index (&it) * 6)));
  }

  CU_ASSERT (iot_data_i32 (iot_data_vector_get (vector, 0)) == 0)
  CU_ASSERT (iot_data_i32 (iot_data_vector_get (vector, 1)) == 6)
  CU_ASSERT (iot_data_i32 (iot_data_vector_get (vector, 2)) == 12)

  iot_data_free (vector);
}

static void test_data_alloc_array_i8 (void)
{
  int8_t data [4] = { -1, -2, 3, 4 };
  int8_t data1 [4] = { 10, 20, -30, -40 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT8, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT8)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT8, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_INT8, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT8)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_ui8 (void)
{
  uint8_t data [4] = { 1, 2, 3, 4 };
  uint8_t data1 [4] = { 10, 20, 30, 40 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT8)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT8, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT8)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_i16 (void)
{
  static int16_t data [4] = { -11111, -22222, 3333, 4444 };
  static int16_t data1 [4] = { -4444, -3333, 22222, 11111 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT16, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT16)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT16, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_INT16, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT16)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_ui16 (void)
{
  static uint16_t data [4] = { 11111, 22222, 44444, 55555 };
  static uint16_t data1 [4] = { 55555, 11111, 22222, 44444  };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT16, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT16)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT16, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT16, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT16)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT16)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_i32 (void)
{
  static int32_t data [4] = { -1111111, -2222222, 3333333, 4444444 };
  static int32_t data1 [4] = { -4444444, -3333333, 2222222, 1111111 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT32, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT32)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT32, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_INT32, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT32)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_ui32 (void)
{
  static uint32_t data [4] = { 1111111, 2222222, 3333333, 4444444 };
  static uint32_t data1 [4] = { 2222222, 4444444, 1111111, 3333333 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT32, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT32)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT32, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT32, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT32)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_i64 (void)
{
  static int64_t data [4] = { -444444444444444 , 222222222222222, 333333333333333, -111111111111111 };
  static int64_t data1 [4] = { -222222222222222, -333333333333333, 111111111111111, 444444444444444 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT64, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT64)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_INT64, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT64)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_INT64, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_INT64)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_INT64)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_ui64 (void)
{
  static uint64_t data [4] = { 222222222222222, 333333333333333, 444444444444444, 111111111111111 };
  static uint64_t data1 [4] = { 333333333333333, 222222222222222, 111111111111111, 444444444444444 };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT64, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT64)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT64, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT64, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT64)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_UINT64)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_f32 (void)
{
  static float_t data [4] = { 1.200001f, 2.300001f, 3.400001f, 4.500001f };
  static float_t data1 [4] = { 4.599999f, 2.300001f, 3.400001f, 1.299999f};

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_FLOAT32, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_FLOAT32)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_FLOAT32, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_FLOAT32)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_FLOAT32, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_FLOAT32)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_FLOAT32)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_f64 (void)
{
  static double_t data [4] = { 1.20000001f, 3.40000001f, 5.60000001f, 7.80000001f };
  static double_t data1 [4] = { 5.69999999f, 7.89999999f, 1.29999999f, 3.49999999f };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_FLOAT64, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_FLOAT64)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_FLOAT64, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_FLOAT64)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_FLOAT64, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_FLOAT64)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_FLOAT64)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_alloc_array_bool (void)
{
  bool data [4] = { true, false, true, false };
  bool data1 [4] = { true, true, false, false };

  iot_data_t * array1 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_BOOL, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_BOOL)

  iot_data_t * array2 = iot_data_alloc_array (data, sizeof (data), IOT_DATA_BOOL, IOT_DATA_REF);

  CU_ASSERT (array2 != NULL)
  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_BOOL)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);

  array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_BOOL, IOT_DATA_COPY);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_BOOL)

  array2 = iot_data_copy (array1);

  CU_ASSERT (iot_data_type (array2) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array2) == IOT_DATA_BOOL)
  CU_ASSERT (iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_zerolength_array (void)
{
  uint32_t empty [0];
  iot_data_t * array1 = iot_data_alloc_array (NULL, 0, IOT_DATA_BOOL, IOT_DATA_REF);

  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_BOOL)

  iot_data_t * array2 = iot_data_alloc_array (NULL, 0, IOT_DATA_BOOL, IOT_DATA_REF);
  CU_ASSERT (iot_data_equal (array1, array2))

  char * json = iot_data_to_json (array1);
  CU_ASSERT (strcmp (json, "[]") == 0)
  free (json);

  iot_data_free (array1);

  array1 = iot_data_alloc_array (empty, 0, IOT_DATA_UINT32, IOT_DATA_COPY);
  CU_ASSERT (array1 != NULL)
  CU_ASSERT (iot_data_type (array1) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (array1) == IOT_DATA_UINT32)
  CU_ASSERT (! iot_data_equal (array1, array2))

  iot_data_free (array1);
  iot_data_free (array2);
}

static void test_data_zerolength_vector (void)
{
  iot_data_t * vector1 = iot_data_alloc_vector (0);

  CU_ASSERT (iot_data_vector_size (vector1) == 0)

  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector1, &iter);

  CU_ASSERT (iot_data_vector_iter_next (&iter) == false)
  CU_ASSERT (iot_data_vector_iter_index (&iter) == 0)
  CU_ASSERT (iot_data_vector_iter_value (&iter) == NULL)
  CU_ASSERT (iot_data_vector_iter_string (&iter) == NULL)

  iot_data_free (vector1);
}

static void test_data_zerolength_vectormap (void)
{
  const iot_data_t * data = NULL;
  iot_data_t * data_map1 = iot_data_alloc_map (IOT_DATA_STRING);

  iot_data_t * vector1 = iot_data_alloc_vector (0);
  iot_data_t * key1 = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_map_add (data_map1, key1, vector1);

  data = iot_data_map_get (data_map1, key1);

  CU_ASSERT (iot_data_type (data) == IOT_DATA_VECTOR)
  CU_ASSERT (iot_data_vector_size (vector1) == 0)
  char * json = iot_data_to_json (data);
  CU_ASSERT (strcmp (json, "[]") == 0)

  free (json);
  iot_data_free (data_map1);
}

static void test_data_basic_typecode (void)
{
  iot_typecode_t tc;
  iot_data_t * data;

  data = iot_data_alloc_i8 (0);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_INT8)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_ui8 (1u);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_UINT8)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_i16 (0);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_INT16)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_ui16 (1u);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_UINT16)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_i32 (0);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_INT32)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_ui32 (1u);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_UINT32)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_i64 (0);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_INT64)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_ui64 (1u);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_UINT64)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_f32 (0.0);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_FLOAT32)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_f64 (1.0);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_FLOAT64)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_bool (true);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_BOOL)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_pointer (&tc, NULL);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_POINTER)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_string ("Hi", IOT_DATA_REF);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_STRING)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_null ();
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_NULL)
  CU_ASSERT (tc.element_type == IOT_DATA_INVALID)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
}

static void test_data_complex_typecode (void)
{
  iot_typecode_t tc;
  iot_data_t * data;
  uint8_t array[2];

  data = iot_data_alloc_binary (&array, sizeof (array), IOT_DATA_REF);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_BINARY)
  CU_ASSERT (tc.element_type == IOT_DATA_UINT8)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_array (&array, sizeof (array), IOT_DATA_INT8, IOT_DATA_REF);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_ARRAY)
  CU_ASSERT (tc.element_type == IOT_DATA_INT8)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_vector (4u);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_VECTOR)
  CU_ASSERT (tc.element_type == IOT_DATA_MULTI)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_typed_vector (4u, IOT_DATA_POINTER);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_VECTOR)
  CU_ASSERT (tc.element_type == IOT_DATA_POINTER)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_list ();
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_LIST)
  CU_ASSERT (tc.element_type == IOT_DATA_MULTI)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_typed_list (IOT_DATA_STRING);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_LIST)
  CU_ASSERT (tc.element_type == IOT_DATA_STRING)
  CU_ASSERT (tc.key_type == IOT_DATA_INVALID)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_map (IOT_DATA_FLOAT32);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_MAP)
  CU_ASSERT (tc.element_type == IOT_DATA_MULTI)
  CU_ASSERT (tc.key_type == IOT_DATA_FLOAT32)
  CU_ASSERT (iot_data_map_type (data) == IOT_DATA_MULTI)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  data = iot_data_alloc_typed_map (IOT_DATA_STRING, IOT_DATA_LIST);
  iot_data_typecode (data, &tc);
  CU_ASSERT (tc.type == IOT_DATA_MAP)
  CU_ASSERT (tc.element_type == IOT_DATA_LIST)
  CU_ASSERT (tc.key_type == IOT_DATA_STRING)
  CU_ASSERT (iot_data_map_type (data) == IOT_DATA_LIST)
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_string_map_add (data, "List", list);
  const iot_data_t * found = iot_data_string_map_get (data, "List");
  CU_ASSERT (found == list)
  iot_data_t * key = iot_data_alloc_string ("List", IOT_DATA_REF);
  found = iot_data_map_get (data, key);
  CU_ASSERT (found == list)
  iot_data_free (key);
  iot_data_free (data);
}

static void test_data_equal_typecode (void)
{
  iot_typecode_t tc = { IOT_DATA_INVALID, IOT_DATA_INVALID, IOT_DATA_INVALID };
  iot_typecode_t tcd;
  iot_data_t * data;
  int16_t array[2];

  tc.type = IOT_DATA_UINT32;
  data = iot_data_alloc_ui32 (1u);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_ARRAY;
  tc.element_type = IOT_DATA_INT16;
  data = iot_data_alloc_array (&array, 2u, IOT_DATA_INT16, IOT_DATA_REF);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_VECTOR;
  tc.element_type = IOT_DATA_MULTI;
  data = iot_data_alloc_vector (4u);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_VECTOR;
  tc.element_type = IOT_DATA_POINTER;
  data = iot_data_alloc_typed_vector (4u, IOT_DATA_POINTER);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_LIST;
  tc.element_type = IOT_DATA_MULTI;
  data = iot_data_alloc_list ();
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_LIST;
  tc.element_type = IOT_DATA_MAP;
  data = iot_data_alloc_typed_list (IOT_DATA_MAP);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_MAP;
  tc.element_type = IOT_DATA_MULTI;
  tc.key_type = IOT_DATA_INT32;
  data = iot_data_alloc_map (IOT_DATA_INT32);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
  tc.type = IOT_DATA_MAP;
  tc.element_type = IOT_DATA_STRING;
  tc.key_type = IOT_DATA_FLOAT64;
  data = iot_data_alloc_typed_map (IOT_DATA_FLOAT64, IOT_DATA_STRING);
  iot_data_typecode (data, &tcd);
  CU_ASSERT (iot_typecode_equal (&tc, &tcd))
  CU_ASSERT (iot_data_matches (data, &tc))
  iot_data_free (data);
}

static void test_data_map_perf (void)
{
  char * keys [10]= { "common_one", "common_two", "common_three", "common_four", "common_five", "common_six", "common_seven", "common_eight", "common_nine", "common_ten" };
  char * lookup_keys [10]= { "common_one", "common_two", "common_three", "common_four", "common_five", "common_six", "common_seven", "common_eight", "common_nine", "common_ten" }; // To avoid key address equality check
  uint32_t i;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  for (i = 0; i < 10; i++)
  {
    iot_data_string_map_add (map, keys[i], iot_data_alloc_ui32 (i));
  }
  uint64_t t1 = iot_time_msecs ();
  for (uint32_t j = 0; j < 100000; j++)
  {
    for (i = 0; i < 10; i++)
    {
      iot_data_string_map_get (map, lookup_keys[i]);
    }
  }
  uint64_t t2 = iot_time_msecs ();
  printf ("String map[10] a million lookups in %" PRIu64 " milliseconds\n", t2 - t1);
  CU_ASSERT (t2 > t1)
  iot_data_free (map);
}

static uint32_t test_get_string_count (iot_data_t * map, const char * str)
{
  const iot_data_t * count = iot_data_string_map_get (map, str);
  if (count)
  {
    iot_data_increment ((iot_data_t*) count);
  }
  else
  {
    count = iot_data_alloc_ui32 (1u);
    iot_data_string_map_add (map, str, (iot_data_t*) count);
  }
  return (iot_data_ui32 (count));
}

static void test_data_int_map (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  uint32_t count = test_get_string_count (map, "Key1");
  CU_ASSERT (count == 1)
  count = test_get_string_count (map, "Key2");
  CU_ASSERT (count == 1)
  count = test_get_string_count (map, "Key1");
  CU_ASSERT (count == 2)
  iot_data_free (map);
}

static void test_data_check_map_null_ret (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "V", iot_data_alloc_null ());
  const char * str = iot_data_string_map_get_string (map, "V");
  CU_ASSERT (str == NULL)
  const iot_data_t * vec = iot_data_string_map_get_vector (map, "V");
  CU_ASSERT (vec == NULL)
  const iot_data_t * mp = iot_data_string_map_get_map (map, "V");
  CU_ASSERT (mp == NULL)
  iot_data_free (map);
}

static void  test_data_add_ref (void)
{
  iot_data_t * data = iot_data_alloc_i32 (66u);
  iot_data_t * ref = iot_data_add_ref (data);
  CU_ASSERT (ref == data)
  iot_data_free (data);
  iot_data_free (ref);
  ref = iot_data_add_ref (NULL);
  CU_ASSERT (ref == NULL)
}

static void test_data_alloc_uuid (void)
{
  iot_data_t * data = iot_data_alloc_uuid_string ();
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  const char * str = iot_data_string (data);
  printf ("UUID: %s\n", str);
  CU_ASSERT (strlen (str) == UUID_STR_LEN - 1)
  iot_data_free (data);
  data = iot_data_alloc_uuid ();
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_size (data) == sizeof (iot_uuid_t))
  iot_data_free (data);
}

static void test_data_alloc_pointer (void)
{
  void * dummy = malloc (16);
  iot_data_t * data = iot_data_alloc_pointer (dummy, free);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_POINTER)
  iot_data_t * data2 = iot_data_alloc_pointer (dummy, NULL);
  CU_ASSERT (iot_data_equal (data, data2))
  CU_ASSERT (iot_data_address (data) == dummy)
  CU_ASSERT (iot_data_address (data2) == dummy)
  CU_ASSERT (iot_data_pointer (data) == dummy)
  iot_data_free (data);
  data = iot_data_alloc_pointer (NULL, NULL);
  CU_ASSERT (iot_data_address (data) == NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_POINTER)
  CU_ASSERT (! iot_data_equal (data, data2))
  iot_data_t * data3 = iot_data_copy (data2);
  CU_ASSERT (iot_data_type (data3) == IOT_DATA_POINTER)
  CU_ASSERT (iot_data_address (data3) == dummy)
  iot_data_free (data);
  data = iot_data_alloc_vector (2);
  CU_ASSERT (iot_data_address (data) == NULL)
  CU_ASSERT (iot_data_address (NULL) == NULL)
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "Pointer", iot_data_alloc_pointer (dummy, NULL));
  CU_ASSERT (iot_data_string_map_get_pointer (map, "Pointer") == dummy)
  iot_data_t * key = iot_data_alloc_string ("Pointer", IOT_DATA_REF);
  CU_ASSERT (iot_data_map_get_pointer (map, key) == dummy)
  iot_data_free (key);
  iot_data_free (map);
  iot_data_free (data);
  iot_data_free (data2);
  iot_data_free (data3);
}

static void test_data_binary (void)
{
  uint8_t buff[3] = { 1,2,3 };
  iot_data_t * bin = iot_data_alloc_binary (buff, sizeof (buff), IOT_DATA_REF);
  iot_data_array_iter_t iter;

  CU_ASSERT (iot_data_address (bin) == buff)
  CU_ASSERT (iot_data_array_is_of_type (bin, IOT_DATA_UINT8))
  CU_ASSERT (iot_data_array_length (bin) == 3u)
  CU_ASSERT (iot_data_array_size (bin) == 3u)
  iot_data_array_iter (bin, &iter);
  CU_ASSERT (iot_data_array_iter_has_next (&iter))
  iot_data_array_iter_next (&iter);
  CU_ASSERT (iot_data_array_iter_has_next (&iter))
  CU_ASSERT (iot_data_array_iter_index (&iter) == 0)
  CU_ASSERT (iot_data_array_iter_value (&iter) == &buff)
  char * json = iot_data_to_json (bin);
  CU_ASSERT (strcmp (json, "\"AQID\"") == 0) // Base64 string encoding
  free (json);
  iot_data_free (bin);
}

static void test_data_binary_from_string (void)
{
  iot_data_t * str = iot_data_alloc_string ("Hello", IOT_DATA_COPY);
  iot_data_t * bin = iot_data_binary_from_string (str);
  const char * bptr = iot_data_address (bin);
  const char * sptr = iot_data_string (str);
  CU_ASSERT (memcmp (sptr, bptr, 5u) == 0)
  iot_data_free (str);
  iot_data_free (bin);
}

static void test_data_string_from_binary (void)
{
    iot_data_t * bin = iot_data_alloc_binary ("Hello", 5u, IOT_DATA_COPY);
    iot_data_t * str = iot_data_string_from_binary (bin);
    const char * bptr = iot_data_address (bin);
    const char * sptr = iot_data_string (str);
    CU_ASSERT (memcmp (sptr, bptr, 5u) == 0)
    CU_ASSERT (strcmp (sptr, "Hello") == 0)
    iot_data_free (str);
    iot_data_free (bin);
}

static void test_data_alloc_heap (void)
{
  iot_data_alloc_heap (true);
  iot_data_t * map = iot_data_from_json (test_config);
  iot_data_free (map);
  iot_data_alloc_heap (false);
}

static void test_data_cast (void)
{
  static const int8_t i8_val = -8;
  static const uint8_t u8_val = 8;
  static const int16_t i16_val = -500;
  static const uint16_t u16_val = 500;
  static const int32_t i32_val = -90000;
  static const uint32_t u32_val = 90000;
  static const int64_t i64_val = -40000000000;
  static const uint64_t u64_val = 40000000000;
  static const float f32_val = 7.0f;
  static const double f64_val = 123.456;
  static const bool bl_val = true;

  iot_data_t * data;
  uint8_t u8 = 0;
  int8_t i8 = 0;
  uint16_t u16 = 0;
  int16_t i16 = 0;
  uint32_t u32 = 0;
  int32_t i32 = 0;
  uint64_t u64 = 0;
  int64_t i64 = 0;
  float f32 = 0.0;
  double f64 = 0.0;
  bool bl = false;

  data = iot_data_alloc_i8 (i8_val);
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (i8 == (int8_t) i8_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) i8_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) i8_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) i8_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) i8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) i8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_ui8 (u8_val);
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (i8 == (int8_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (u8 == (uint8_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (u16 == (uint16_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (u32 == (uint32_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) u8_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_i16 (i16_val);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) i16_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) i16_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) i16_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) i16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) i16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_ui16 (u16_val);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (u16 == (uint16_t) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (u32 == (uint32_t) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) u16_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_i32 (i32_val);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) i32_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) i32_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) i32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) i32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_ui32 (u32_val);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) u32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (u32 == (uint32_t) u32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) u32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) u32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) u32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) u32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_i64 (i64_val);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) i64_val)
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) i64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) i64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_ui64 (u64_val);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) u64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) u64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) u64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) u64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_f32 (f32_val);
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (i8 == (int8_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (u8 == (uint8_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (u16 == (uint16_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (u32 == (uint32_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) f32_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_f64 (f64_val);
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (i8 == (int8_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (u8 == (uint8_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (u16 == (uint16_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (u32 == (uint32_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) f64_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_BOOL, &bl))
  CU_ASSERT (bl)
  iot_data_free (data);

  data = iot_data_alloc_bool (bl_val);
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT8, &i8))
  CU_ASSERT (i8 == (int8_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (u8 == (uint8_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT16, &i16))
  CU_ASSERT (i16 == (int16_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT16, &u16))
  CU_ASSERT (u16 == (uint16_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT32, &i32))
  CU_ASSERT (i32 == (int32_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT32, &u32))
  CU_ASSERT (u32 == (uint32_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_INT64, &i64))
  CU_ASSERT (i64 == (int64_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT64, &u64))
  CU_ASSERT (u64 == (uint64_t) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT32, &f32))
  CU_ASSERT (f32 == (float) bl_val)
  CU_ASSERT (iot_data_cast (data, IOT_DATA_FLOAT64, &f64))
  CU_ASSERT (f64 == (double) bl_val)
  iot_data_free (data);

  data = iot_data_alloc_string ("hi", IOT_DATA_REF);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (! iot_data_is_static (data))
  iot_data_free (data);

  data = iot_data_alloc_null ();
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (iot_data_is_static (data))
  iot_data_free (data);

  data = iot_data_alloc_bool (true);
  CU_ASSERT (iot_data_cast (data, IOT_DATA_UINT8, &u8))
  CU_ASSERT (iot_data_is_static (data))
  iot_data_free (data);

  data = iot_data_alloc_pointer (&data, NULL);
  CU_ASSERT (! iot_data_cast (data, IOT_DATA_UINT8, &u8))
  iot_data_free (data);

  uint64_t temp_buf;
  data = iot_data_alloc_i64 (-1);
  CU_ASSERT (!iot_data_cast (data, IOT_DATA_UINT32, &temp_buf))
  iot_data_free (data);
  data = iot_data_alloc_i64 (4294967296);
  CU_ASSERT (!iot_data_cast (data, IOT_DATA_UINT32, &temp_buf))
  iot_data_free (data);
  data = iot_data_alloc_i64 (-2147483649);
  CU_ASSERT (!iot_data_cast (data, IOT_DATA_INT32, &temp_buf))
  iot_data_free (data);
  data = iot_data_alloc_i64 (2147483648);
  CU_ASSERT (!iot_data_cast (data, IOT_DATA_INT32, &temp_buf))
  iot_data_free (data);
  data = iot_data_alloc_i64 (-1);
  CU_ASSERT (!iot_data_cast (data, IOT_DATA_UINT64, &temp_buf))
  iot_data_free (data);
}

static void test_data_const_string (void)
{
  static iot_data_static_t block;
  static const char * str = "Hello";
  iot_data_t * data = iot_data_alloc_const_string (&block, str);
  CU_ASSERT (iot_data_string (data) == str)
  CU_ASSERT (data == IOT_DATA_STATIC (&block))
  CU_ASSERT (iot_data_is_static (data))
  iot_data_free (data);
  iot_data_free (data);
}

static void test_data_const_ui64 (void)
{
  static iot_data_static_t block;
  iot_data_t * data = iot_data_alloc_const_ui64 (&block, 222u);
  CU_ASSERT (iot_data_ui64 (data) == 222u)
  CU_ASSERT (data == IOT_DATA_STATIC (&block))
  CU_ASSERT (iot_data_is_static (data))
  iot_data_free (data);
  iot_data_free (data);
}

static void test_data_const_pointer (void)
{
  static iot_data_static_t block;
  static const void * ptr = &block;
  iot_data_t * data = iot_data_alloc_const_pointer (&block, ptr);
  CU_ASSERT (iot_data_pointer (data) == ptr)
  CU_ASSERT (data == IOT_DATA_STATIC (&block))
  CU_ASSERT (iot_data_is_static (data))
  iot_data_free (data);
  iot_data_free (data);
}

static void test_data_const_list (void)
{
  static iot_data_list_static_t block;
  static iot_data_static_t block2;
  static iot_data_static_t block3;
  iot_data_t * list = iot_data_alloc_const_list (&block);
  iot_data_t * ptr = iot_data_alloc_const_pointer (&block2, &block2);
  iot_data_t * str = iot_data_alloc_const_string (&block3, "test");
  CU_ASSERT (iot_data_list_length (list) == 0u)
  CU_ASSERT (iot_data_type (list) == IOT_DATA_LIST)
  CU_ASSERT (list == IOT_DATA_STATIC (&block))
  CU_ASSERT (iot_data_is_static (list))
  iot_data_list_tail_push (list, ptr);
  CU_ASSERT (iot_data_list_length (list) == 1u)
  iot_data_list_tail_push (list, str);
  CU_ASSERT (iot_data_list_length (list) == 2u)
  iot_data_list_empty (list);
  CU_ASSERT (iot_data_list_length (list) == 0u)
  iot_data_free (list);
  iot_data_free (list);

  iot_data_list_static_t block_non_static;
  iot_data_t * list_non_static = iot_data_alloc_const_list (&block_non_static);
  CU_ASSERT (iot_data_list_length (list_non_static) == 0u)
  CU_ASSERT (iot_data_type (list_non_static) == IOT_DATA_LIST)
  CU_ASSERT (list_non_static == IOT_DATA_STATIC (&block_non_static))
  CU_ASSERT (iot_data_is_static (list_non_static))
  iot_data_list_tail_push (list_non_static, str);
  CU_ASSERT (iot_data_list_length (list_non_static) == 1u)
  iot_data_list_empty (list_non_static);
  CU_ASSERT (iot_data_list_length (list_non_static) == 0u)
  iot_data_free (list_non_static);
  iot_data_free (list_non_static);
}


static void test_data_const_types (void)
{
  const iot_data_t * b1 = iot_data_alloc_bool (true);
  const iot_data_t * b2 = iot_data_alloc_bool (true);
  const iot_data_t * b3 = iot_data_alloc_bool (false);
  const iot_data_t * b4 = iot_data_alloc_bool (false);
  const iot_data_t * n1 = iot_data_alloc_null ();
  const iot_data_t * n2 = iot_data_alloc_null ();
  CU_ASSERT (iot_data_equal (b1, b2))
  CU_ASSERT (b1 == b2)
  CU_ASSERT (iot_data_equal (b3, b4))
  CU_ASSERT (b3 == b4)
  CU_ASSERT (b1 != b3)
  CU_ASSERT (iot_data_equal (n1, n2))
  CU_ASSERT (n1 == n2)
  /* No data free as are static - so should be no memory leak */
}

static void test_data_hash (void)
{
  iot_data_t * data = iot_data_alloc_bool (true);
  CU_ASSERT (iot_data_hash (data) == 0u)
  iot_data_free (data);
  CU_ASSERT (iot_data_hash (NULL) == 0u)
  data = iot_data_alloc_string ("Dummy", IOT_DATA_REF);
  CU_ASSERT (iot_data_hash (data) == 3802084562u)
  iot_data_free (data);
  data = iot_data_alloc_array ((uint8_t*) "binary", strlen ("binary"), IOT_DATA_UINT8, IOT_DATA_REF);
  CU_ASSERT (iot_data_hash (data) == 2016023253u)
  iot_data_free (data);
  data = iot_data_alloc_binary ((uint8_t*) "bool", strlen ("bool"), IOT_DATA_REF);
  CU_ASSERT (iot_data_hash (data) == 636838452u)
  iot_data_free (data);
}

static void test_data_multi_key_map (void)
{
  iot_data_t * map = iot_data_alloc_typed_map (IOT_DATA_MULTI, IOT_DATA_UINT32);
  uint32_t i = 0;
  while (i < 10)
  {
    iot_data_map_add (map, iot_data_alloc_ui32 (i), iot_data_alloc_ui32 (i));
    i++;
  }
  iot_data_map_add (map, iot_data_alloc_string ("A", IOT_DATA_REF),iot_data_alloc_ui32 (i++));
  iot_data_map_add (map, iot_data_alloc_string ("B", IOT_DATA_REF),iot_data_alloc_ui32 (i++));
  iot_data_map_add (map, iot_data_alloc_string ("C", IOT_DATA_REF),iot_data_alloc_ui32 (i++));
  CU_ASSERT (iot_data_map_size (map) == 13u)
  iot_data_t * key = iot_data_alloc_ui32 (4u);
  const iot_data_t * val = iot_data_map_get (map, key);
  CU_ASSERT (iot_data_ui32 (val) == 4u)
  iot_data_free (key);
  key = iot_data_alloc_string ("B", IOT_DATA_REF);
  val = iot_data_map_get (map, key);
  CU_ASSERT (iot_data_ui32 (val) == 11u)
  iot_data_free (key);
  iot_data_free (map);
}

static void test_data_map_empty (void)
{
  iot_data_t * map = iot_data_alloc_typed_map (IOT_DATA_UINT32, IOT_DATA_UINT32);
  uint32_t i = 0;
  while (i < 10)
  {
    iot_data_map_add (map, iot_data_alloc_ui32 (i), iot_data_alloc_ui32 (i));
    i++;
  }
  CU_ASSERT (iot_data_map_size (map) == 10u)
  iot_data_map_empty (map);
  CU_ASSERT (iot_data_map_size (map) == 0u)
  iot_data_free (map);
}

static void test_data_map_struct_key (void)
{
  iot_data_t * map = iot_data_alloc_typed_map (IOT_DATA_MAP, IOT_DATA_UINT32);
  iot_data_t * key1 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key2 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * val1 = iot_data_alloc_ui32 (11u);
  iot_data_t * val2 =  iot_data_alloc_ui32 (22u);
  const iot_data_t * ret;

  iot_data_string_map_add (key1, "Ten", iot_data_alloc_ui32 (10u));
  iot_data_string_map_add (key1, "Two", iot_data_alloc_ui32 (2u));
  iot_data_string_map_add (key2, "Two", iot_data_alloc_ui32 (2u));
  iot_data_string_map_add (key2, "Four", iot_data_alloc_ui32 (4u));
  iot_data_string_map_add (key2, "Ten", iot_data_alloc_ui32 (10u));

  uint32_t hash1 = iot_data_hash (key1);
  uint32_t hash2 = iot_data_hash (key2);
  CU_ASSERT (hash1 != hash2)

  iot_data_map_add (map, key1, val1);
  iot_data_map_add (map, key2, val2);

  ret = iot_data_map_get (map, key1);
  CU_ASSERT (ret != NULL)
  CU_ASSERT (ret == val1)
  ret = iot_data_map_get (map, key2);
  CU_ASSERT (ret == val2)

  iot_data_string_map_remove (key2, "Four");
  hash2 = iot_data_hash (key2);
  CU_ASSERT (hash1 == hash2)

  iot_data_free (map);
}

static void test_data_list_hash (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  uint32_t hash1 = iot_data_hash (list);
  CU_ASSERT (hash1 == 0)
  iot_data_list_tail_push (list, iot_data_alloc_i32 (1));
  uint32_t hash2 = iot_data_hash (list);
  CU_ASSERT (hash2 != hash1)
  iot_data_list_tail_push (list, iot_data_alloc_i32 (2));
  uint32_t hash3 = iot_data_hash (list);
  CU_ASSERT (hash3 != hash2)
  iot_data_list_tail_push (list, iot_data_alloc_i32 (3));
  uint32_t hash = iot_data_hash (list);
  CU_ASSERT (hash != hash3)
  iot_data_free (iot_data_list_tail_pop (list));
  hash = iot_data_hash (list);
  CU_ASSERT (hash == hash3)
  iot_data_free (iot_data_list_tail_pop (list));
  hash = iot_data_hash (list);
  CU_ASSERT (hash == hash2)
  iot_data_free (iot_data_list_tail_pop (list));
  hash = iot_data_hash (list);
  CU_ASSERT (hash == hash1)
  iot_data_free (list);
}

static void test_data_vector_hash (void)
{
  iot_data_t * vec = iot_data_alloc_vector (3);
  uint32_t hash1 = iot_data_hash (vec);
  CU_ASSERT (hash1 == 0)
  iot_data_vector_add (vec, 0u,iot_data_alloc_i32 (1));
  uint32_t hash2 = iot_data_hash (vec);
  CU_ASSERT (hash2 != hash1)
  iot_data_vector_add (vec, 1u, iot_data_alloc_i32 (2));
  uint32_t hash3 = iot_data_hash (vec);
  CU_ASSERT (hash3 != hash2)
  iot_data_vector_add (vec, 2u,iot_data_alloc_i32 (3));
  uint32_t hash = iot_data_hash (vec);
  CU_ASSERT (hash != hash3)
  iot_data_vector_add (vec, 2u,NULL);
  hash = iot_data_hash (vec);
  CU_ASSERT (hash == hash3)
  iot_data_vector_add (vec, 1u,NULL);
  hash = iot_data_hash (vec);
  CU_ASSERT (hash == hash2)
  iot_data_vector_add (vec, 0u,NULL);
  hash = iot_data_hash (vec);
  CU_ASSERT (hash == hash1)
  iot_data_free (vec);
}

static void data_compare_check (iot_data_t * v1, iot_data_t * v1b, iot_data_t * v2)
{
  CU_ASSERT (iot_data_compare (v1, v2) < 0)
  CU_ASSERT (iot_data_compare (v2, v1) > 0)
  CU_ASSERT (iot_data_compare (v1, v1b) == 0)
  iot_data_free (v1);
  iot_data_free (v2);
  iot_data_free (v1b);
}

static void test_data_compare (void)
{
  uint8_t bin1 [2u] = { 1u, 0u };
  uint8_t bin1b [2u] = { 1u, 0u };
  uint8_t bin2 [2u] = { 2u, 0u };
  iot_data_t * v1;
  iot_data_t * v1b;
  iot_data_t * v2;
  data_compare_check (iot_data_alloc_i8 (1), iot_data_alloc_i8 (1),iot_data_alloc_i8 (2));
  data_compare_check (iot_data_alloc_ui8 (1u), iot_data_alloc_ui8 (1u),iot_data_alloc_ui8 (2u));
  data_compare_check (iot_data_alloc_i16 (1), iot_data_alloc_i16 (1),iot_data_alloc_i16 (2));
  data_compare_check (iot_data_alloc_ui16 (1u), iot_data_alloc_ui16 (1u),iot_data_alloc_ui16 (2u));
  data_compare_check (iot_data_alloc_i32 (1), iot_data_alloc_i32 (1),iot_data_alloc_i32 (2));
  data_compare_check (iot_data_alloc_ui32 (1u), iot_data_alloc_ui32 (1u),iot_data_alloc_ui32 (2u));
  data_compare_check (iot_data_alloc_i64 (1), iot_data_alloc_i64 (1),iot_data_alloc_i64 (2));
  data_compare_check (iot_data_alloc_ui64 (1u), iot_data_alloc_ui64 (1u),iot_data_alloc_ui64 (2u));
  data_compare_check (iot_data_alloc_f32 (1.0), iot_data_alloc_f32 (1.0),iot_data_alloc_f32 (2.0));
  data_compare_check (iot_data_alloc_f64 (1.0), iot_data_alloc_f64 (1.0),iot_data_alloc_f64 (2.0));
  data_compare_check (iot_data_alloc_bool (false), iot_data_alloc_bool (false),iot_data_alloc_bool (true));
  data_compare_check (iot_data_alloc_pointer ((void*) 0, NULL), iot_data_alloc_pointer ((void*) 0, NULL),iot_data_alloc_pointer ((void*) 2, NULL));
  data_compare_check (iot_data_alloc_string ("a", IOT_DATA_COPY), iot_data_alloc_string ("a", IOT_DATA_COPY),iot_data_alloc_string ("b", IOT_DATA_COPY));
  CU_ASSERT (iot_data_compare (NULL, NULL) == 0)
  CU_ASSERT (iot_data_compare (iot_data_alloc_null (), iot_data_alloc_null ()) == 0)
  data_compare_check (NULL, NULL, iot_data_alloc_null ());
  data_compare_check (iot_data_alloc_binary (bin1, sizeof (bin1), IOT_DATA_REF), iot_data_alloc_binary (bin1b, sizeof (bin1b), IOT_DATA_REF), iot_data_alloc_binary (bin2, sizeof (bin2), IOT_DATA_REF));
  v1 = iot_data_alloc_vector (1u);
  iot_data_vector_add (v1, 0u, iot_data_alloc_null ());
  v1b = iot_data_alloc_vector (1u);
  iot_data_vector_add (v1b, 0u, iot_data_alloc_null ());
  v2 = iot_data_alloc_vector (2u);
  data_compare_check (v1, v1b, v2);
  v1 = iot_data_alloc_map (IOT_DATA_UINT32);
  v1b = iot_data_alloc_map (IOT_DATA_UINT32);
  v2 = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_map_add (v1, iot_data_alloc_ui32 (1u), iot_data_alloc_ui32 (1u));
  iot_data_map_add (v1b, iot_data_alloc_ui32 (1u), iot_data_alloc_ui32 (1u));
  iot_data_map_add (v2, iot_data_alloc_ui32 (1u), iot_data_alloc_ui32 (1u));
  iot_data_map_add (v2, iot_data_alloc_ui32 (2u), iot_data_alloc_ui32 (2u));
  data_compare_check (v1, v1b, v2);
  v1 = iot_data_alloc_list ();
  v1b = iot_data_alloc_list ();
  v2 = iot_data_alloc_list ();
  iot_data_list_tail_push (v1, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (v1b, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (v2, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (v2, iot_data_alloc_ui32 (1u));
  data_compare_check (v1, v1b, v2);
  v1 = iot_data_alloc_null ();
  v1b = iot_data_alloc_null ();
  CU_ASSERT (iot_data_compare (v1, v1b) == 0)
}

static void test_data_map_add_unused (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_string_map_add (map, "One", val);
  val = iot_data_alloc_ui32 (2u);
  iot_data_t * key = iot_data_alloc_string ("Two", IOT_DATA_REF);
  bool added = iot_data_map_add_unused (map, key, val);
  CU_ASSERT (added)
  val = iot_data_alloc_ui32 (3u);
  key = iot_data_alloc_string ("Two", IOT_DATA_REF);
  added = iot_data_map_add_unused (map, key, val);
  CU_ASSERT (! added)
  CU_ASSERT (iot_data_map_size (map) == 2)
  iot_data_free (val);
  iot_data_free (key);
  iot_data_free (map);
}

static void test_data_vector_to_array (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3u);
  iot_data_vector_add (vector, 0, iot_data_alloc_ui32 (0u));
  iot_data_vector_add (vector, 1u, iot_data_alloc_null ());
  iot_data_vector_add (vector, 2u, iot_data_alloc_ui16 (2u));
  CU_ASSERT (iot_data_vector_size (vector) == 3u)
  iot_data_t * array = iot_data_vector_to_array (vector, IOT_DATA_UINT8, false);
  CU_ASSERT (iot_data_array_size (array) == 2u)
  CU_ASSERT (iot_data_array_type (array) == IOT_DATA_UINT8)
  iot_data_array_iter_t iter;
  iot_data_array_iter (array, &iter);
  iot_data_array_iter_next (&iter);
  CU_ASSERT (*((uint8_t*) iot_data_array_iter_value (&iter)) == 0u)
  iot_data_array_iter_next (&iter);
  CU_ASSERT (*((uint8_t*) iot_data_array_iter_value (&iter)) == 2u)

  iot_data_free (vector);
  iot_data_free (array);

  vector = iot_data_alloc_vector (1u);
  iot_data_vector_add (vector, 0u, iot_data_alloc_ui32 (666u));
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT8, false);
  CU_ASSERT (iot_data_array_size (array) == 0u)

  iot_data_free (vector);
  iot_data_free (array);
}

static void test_data_vector_to_vector (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3u);
  iot_data_vector_add (vector, 0, iot_data_alloc_ui32 (0u));
  iot_data_vector_add (vector, 1u, iot_data_alloc_null ());
  iot_data_vector_add (vector, 2u, iot_data_alloc_ui16 (2u));
  CU_ASSERT (iot_data_vector_size (vector) == 3u)
  iot_data_t * out = iot_data_vector_to_vector (vector, IOT_DATA_UINT8, false);
  CU_ASSERT (iot_data_vector_size (out) == 2u)
  CU_ASSERT (iot_data_vector_type (out) == IOT_DATA_UINT8)
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (out, &iter);
  iot_data_vector_iter_next (&iter);
  const iot_data_t * val = iot_data_vector_iter_value (&iter);
  CU_ASSERT (iot_data_ui8 (val) == 0u)
  iot_data_vector_iter_next (&iter);
  val = iot_data_vector_iter_value (&iter);
  CU_ASSERT (iot_data_ui8 (val) == 2u)

  iot_data_free (vector);
  iot_data_free (out);

  vector = iot_data_alloc_vector (1u);
  iot_data_vector_add (vector, 0u, iot_data_alloc_ui32 (666u));
  out = iot_data_vector_to_vector (vector, IOT_DATA_UINT8, false);
  CU_ASSERT (iot_data_vector_size (out) == 0u)

  iot_data_free (vector);
  iot_data_free (out);
}
static void test_data_nested_vector_to_array (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3u);
  iot_data_t * vector2 = iot_data_alloc_vector (3u);
  iot_data_vector_add (vector2, 0, iot_data_alloc_ui32 (20u));
  iot_data_vector_add (vector2, 1u, iot_data_alloc_null ());
  iot_data_vector_add (vector2, 2u, iot_data_alloc_ui16 (22u));
  iot_data_vector_add (vector, 0, iot_data_alloc_ui32 (0u));
  iot_data_vector_add (vector, 1u, iot_data_alloc_null ());
  iot_data_vector_add (vector, 2u, vector2);
  CU_ASSERT (iot_data_vector_size (vector) == 3u)
  CU_ASSERT (iot_data_vector_size (vector2) == 3u)
  iot_data_t * array = iot_data_vector_to_array (vector, IOT_DATA_UINT8, true);
  CU_ASSERT (iot_data_array_size (array) == 3u)
  CU_ASSERT (iot_data_array_type (array) == IOT_DATA_UINT8)
  iot_data_array_iter_t iter;
  iot_data_array_iter (array, &iter);
  iot_data_array_iter_next (&iter);
  CU_ASSERT (*((uint8_t*) iot_data_array_iter_value (&iter)) == 0u)
  iot_data_array_iter_next (&iter);
  CU_ASSERT (*((uint8_t*) iot_data_array_iter_value (&iter)) == 20u)
  iot_data_array_iter_next (&iter);
  CU_ASSERT (*((uint8_t*) iot_data_array_iter_value (&iter)) == 22u)
  iot_data_free (vector);
  iot_data_free (array);
}

static void test_data_ref_count (void)
{
  iot_data_t * data = iot_data_alloc_ui32 (0u);
  CU_ASSERT (iot_data_ref_count (data) == 1u)
  iot_data_add_ref (data);
  CU_ASSERT (iot_data_ref_count (data) == 2u)
  iot_data_free (data);
  CU_ASSERT (iot_data_ref_count (data) == 1u)
  iot_data_free (data);
  CU_ASSERT (iot_data_ref_count (NULL) == 0u)
}

static void test_data_array_transform (void)
{
  uint32_t array [4] = { 0u, 400u, 2u , 500u };
  const uint8_t * val;
  iot_data_t * data = iot_data_alloc_array (array, 4u, IOT_DATA_UINT32, IOT_DATA_REF);
  iot_data_t * data2 = iot_data_array_transform (data, IOT_DATA_UINT8);
  CU_ASSERT (data2 != NULL)
  CU_ASSERT (iot_data_array_size (data2) == 2u)
  iot_data_array_iter_t iter;
  iot_data_array_iter (data2, &iter);
  iot_data_array_iter_next (&iter);
  val = (const uint8_t*) iot_data_array_iter_value (&iter);
  CU_ASSERT (*val == 0u)
  iot_data_array_iter_next (&iter);
  val = (const uint8_t*) iot_data_array_iter_value (&iter);
  CU_ASSERT (*val == 2u)
  iot_data_free (data);
  iot_data_free (data2);
}

static void test_data_transform (void)
{
  iot_data_t * data = iot_data_alloc_i8 (1);
  iot_data_t * xform = iot_data_transform (data, IOT_DATA_UINT8);
  CU_ASSERT (xform && iot_data_type (xform) == IOT_DATA_UINT8)
  CU_ASSERT (iot_data_ui8 (xform) == 1u)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_INT16);
  CU_ASSERT (xform && iot_data_type (xform) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_i16 (xform) == 1)
  iot_data_free (xform);
  iot_data_free (data);
  data = iot_data_alloc_i8 (-1);
  xform = iot_data_transform (data, IOT_DATA_UINT8);
  CU_ASSERT (xform == NULL)
  xform = iot_data_transform (data, IOT_DATA_INT16);
  CU_ASSERT (xform && iot_data_type (xform) == IOT_DATA_INT16)
  CU_ASSERT (iot_data_i16 (xform) == -1)
  iot_data_free (data);
  iot_data_free (xform);

  data = iot_data_alloc_ui32 (1234u);
  xform = iot_data_transform (data, IOT_DATA_UINT8);
  CU_ASSERT (xform == NULL)
  iot_data_free (data);
  data = iot_data_alloc_i64 (123);
  xform = iot_data_transform (data, IOT_DATA_INT8);
  CU_ASSERT (iot_data_i8 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_UINT8);
  CU_ASSERT (iot_data_ui8 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_INT16);
  CU_ASSERT (iot_data_i16 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_UINT16);
  CU_ASSERT (iot_data_ui16 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_INT32);
  CU_ASSERT (iot_data_i32 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_UINT32);
  CU_ASSERT (iot_data_ui32 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_INT64);
  CU_ASSERT (iot_data_i64 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_UINT64);
  CU_ASSERT (iot_data_ui64 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_FLOAT32);
  CU_ASSERT (iot_data_f32 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_FLOAT64);
  CU_ASSERT (iot_data_f64 (xform) == 123)
  iot_data_free (xform);
  xform = iot_data_transform (data, IOT_DATA_BOOL);
  CU_ASSERT (iot_data_bool (xform))
  iot_data_free (xform);
  iot_data_free (data);

  // json integer tests
  data = iot_data_alloc_i64 (UINT32_MAX);
  xform = iot_data_transform (data, IOT_DATA_UINT32);
  CU_ASSERT (iot_data_ui32 (xform) == UINT32_MAX)
  iot_data_free (xform);
  iot_data_free (data);
  data = iot_data_alloc_i64 (INT32_MIN);
  xform = iot_data_transform (data, IOT_DATA_INT32);
  CU_ASSERT (iot_data_i32 (xform) == INT32_MIN)
  iot_data_free (xform);
  iot_data_free (data);
  data = iot_data_alloc_i64 (INT32_MAX);
  xform = iot_data_transform (data, IOT_DATA_INT32);
  CU_ASSERT (iot_data_i32 (xform) == INT32_MAX)
  iot_data_free (xform);
  iot_data_free (data);
  data = iot_data_alloc_i64 (INT64_MAX);  // max uint64 integer that can be provided as json
  xform = iot_data_transform (data, IOT_DATA_UINT64);
  CU_ASSERT (iot_data_ui64 (xform) == INT64_MAX)
  iot_data_free (xform);
  iot_data_free (data);
  data = iot_data_alloc_i64 (INT64_MIN);
  xform = iot_data_transform (data, IOT_DATA_INT64);
  CU_ASSERT (iot_data_i64 (xform) == INT64_MIN)
  iot_data_free (xform);
  iot_data_free (data);
  data = iot_data_alloc_i64 (INT64_MAX);
  xform = iot_data_transform (data, IOT_DATA_INT64);
  CU_ASSERT (iot_data_i64 (xform) == INT64_MAX)
  iot_data_free (xform);
  iot_data_free (data);
}

static void test_vector_elements (void)
{
  iot_data_t * vector = iot_data_alloc_vector (4u);
  iot_data_t * vector2 = iot_data_alloc_vector (3u);
  iot_data_vector_add (vector2, 0u, iot_data_alloc_null ());
  iot_data_vector_add (vector2,1u, iot_data_alloc_ui32 (1u));
  iot_data_vector_add (vector2, 2u, iot_data_alloc_map (IOT_DATA_STRING));
  iot_data_vector_add (vector, 0u, iot_data_alloc_ui32 (0u));
  iot_data_vector_add (vector, 1u, iot_data_alloc_ui32 (1u));
  iot_data_vector_add (vector, 2u, vector2);
  iot_data_vector_add (vector, 3u, iot_data_alloc_ui16 (3u));
  CU_ASSERT (iot_data_vector_element_count (vector2, IOT_DATA_MULTI, false) == 3u)
  CU_ASSERT (iot_data_vector_element_count (vector2, IOT_DATA_NULL, false) == 1u)
  CU_ASSERT (iot_data_vector_element_count (vector2, IOT_DATA_UINT32, false) == 1u)
  CU_ASSERT (iot_data_vector_element_count (vector2, IOT_DATA_INT32, false) == 0u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_MULTI, false) == 4u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_MULTI, true) == 6u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_UINT32, true) == 3u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_UINT32, false) == 2u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_NULL, false) == 0u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_NULL, true) == 1u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_UINT16, true) == 1u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_VECTOR, false) == 1u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_VECTOR, true) == 0u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_MAP, false) == 0u)
  CU_ASSERT (iot_data_vector_element_count (vector, IOT_DATA_MAP, true) == 1u)

  uint32_t depth;
  uint32_t total;
  const uint32_t * vals;
  iot_data_t * dims;
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 1u)
  CU_ASSERT (vals[0] == 4u)
  iot_data_free (dims);

  dims = iot_data_vector_dimensions (vector2, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  CU_ASSERT (total == 3u)
  CU_ASSERT (depth == 1u)
  CU_ASSERT (vals[0] == 3u)
  iot_data_free (dims);

  iot_data_free (vector);
}

static const char * test_json1 = "[[1,2],[2,3],[3,4]]";                  // 3 by 2 integer array
static const char * test_json2 = "[[1,2,3,4]]";                          // 1 by 4 integer array
static const char * test_json3 = "[1,2,3,4]";                            // 4 by 1 integer array
static const char * test_json4 = "[[1],2,[3],[4]]";                      // 4 by 1 integer array
static const char * test_json5 = "[[[1,0]],[[2,0]],[[3,0]],[[4,0]]]";    // 4 by 1 by 2 integer array
static const char * test_json6 = "[[1,2],[3],[3,4]]";                    // Invalid array
static const char * test_json7 = "[[\"Hello\",\"I\"],[\"am\",\"mad\"]]"; // 2 by 2 string array

static void test_vector_dimensions (void)
{
  uint32_t depth;
  uint32_t total;
  const uint32_t *vals;
  iot_data_t *dims;
  iot_data_t *vector;
  iot_data_t *vec2;

  vector = iot_data_from_json (test_json1);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 6u)
  CU_ASSERT (total == 6u)
  CU_ASSERT (depth == 2u)
  CU_ASSERT (dims != NULL)
  CU_ASSERT (vals[0u] == 3u)
  CU_ASSERT (vals[1u] == 2u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (vec2);

  vector = iot_data_from_json (test_json2);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 2u)
  CU_ASSERT (dims != NULL)
  CU_ASSERT (vals[0u] == 1u)
  CU_ASSERT (vals[1u] == 4u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (vec2);

  vector = iot_data_from_json (test_json3);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 1u)
  CU_ASSERT (vals != NULL)
  CU_ASSERT (vals[0u] == 4u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (vec2);

  vector = iot_data_from_json (test_json4);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 1u)
  CU_ASSERT (vals != NULL)
  CU_ASSERT (vals[0u] == 4u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (vec2);

  vector = iot_data_from_json (test_json5);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 8u)
  CU_ASSERT (total == 8u)
  CU_ASSERT (depth == 3u)
  CU_ASSERT (vals != NULL)
  CU_ASSERT (vals[0u] == 4u)
  CU_ASSERT (vals[1u] == 1u)
  CU_ASSERT (vals[2u] == 2u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (vec2);

  vector = iot_data_from_json (test_json6);
  dims = iot_data_vector_dimensions (vector, &total);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 5u)
  CU_ASSERT (total == 0u)
  CU_ASSERT (dims == NULL)
  iot_data_free (vector);
  iot_data_free (vec2);

  vector = iot_data_from_json (test_json7);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  vec2 = iot_data_vector_to_vector (vector, IOT_DATA_STRING, true);
  CU_ASSERT (iot_data_vector_size (vec2) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 2u)
  CU_ASSERT (dims != NULL)
  CU_ASSERT (vals[0u] == 2u)
  CU_ASSERT (vals[1u] == 2u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (vec2);
}

static void test_array_dimensions (void)
{
  uint32_t total;
  uint32_t depth;
  const uint32_t * vals;
  iot_data_t * dims;
  iot_data_t * vector;
  iot_data_t * array;

  vector = iot_data_from_json (test_json1);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_array_length (array) == 6u)
  CU_ASSERT (total == 6u)
  CU_ASSERT (depth == 2u)
  CU_ASSERT (dims != NULL)
  CU_ASSERT (vals[0u] == 3u)
  CU_ASSERT (vals[1u] == 2u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (array);

  vector = iot_data_from_json (test_json2);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_array_length (array) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 2u)
  CU_ASSERT (dims != NULL)
  CU_ASSERT (vals[0u] == 1u)
  CU_ASSERT (vals[1u] == 4u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (array);

  vector = iot_data_from_json (test_json3);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_array_length (array) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 1u)
  CU_ASSERT (vals != NULL)
  CU_ASSERT (vals[0u] == 4u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (array);

  vector = iot_data_from_json (test_json4);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_array_length (array) == 4u)
  CU_ASSERT (total == 4u)
  CU_ASSERT (depth == 1u)
  CU_ASSERT (vals != NULL)
  CU_ASSERT (vals[0u] == 4u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (array);

  vector = iot_data_from_json (test_json5);
  dims = iot_data_vector_dimensions (vector, &total);
  depth = iot_data_array_length (dims);
  vals = iot_data_address (dims);
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_array_length (array) == 8u)
  CU_ASSERT (total == 8u)
  CU_ASSERT (depth == 3u)
  CU_ASSERT (vals != NULL)
  CU_ASSERT (vals[0u] == 4u)
  CU_ASSERT (vals[1u] == 1u)
  CU_ASSERT (vals[2u] == 2u)
  iot_data_free (dims);
  iot_data_free (vector);
  iot_data_free (array);

  vector = iot_data_from_json (test_json6);
  dims = iot_data_vector_dimensions (vector, &total);
  array = iot_data_vector_to_array (vector, IOT_DATA_UINT32, true);
  CU_ASSERT (iot_data_array_length (array) == 5u)
  CU_ASSERT (total == 0u)
  CU_ASSERT (dims == NULL)
  iot_data_free (vector);
  iot_data_free (array);
}

static void test_shallow_copy_map (void)
{
  iot_data_t * map = iot_data_from_json (test_config);
  iot_data_t * copy = iot_data_shallow_copy (map);
  CU_ASSERT (copy != NULL)
  CU_ASSERT (copy != map)
  CU_ASSERT (iot_data_type (copy) == IOT_DATA_MAP)
  CU_ASSERT (iot_data_equal (copy, map))
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    CU_ASSERT (iot_data_map_iter_value (&iter) == iot_data_map_get (copy, iot_data_map_iter_key (&iter)))
  }
  iot_data_free (copy);
  iot_data_free (map);
}

static void test_shallow_copy_vector (void)
{
  static const char * test_vector = "[1, \"a string\", {\"a\" : \"b\", [ 1.2, 3.5 ]} ]";
  iot_data_t * vec = iot_data_from_json (test_vector);
  iot_data_t * copy = iot_data_shallow_copy (vec);
  CU_ASSERT (copy != NULL)
  CU_ASSERT (copy != vec)
  CU_ASSERT (iot_data_type (copy) == IOT_DATA_VECTOR)
  CU_ASSERT (iot_data_equal (copy, vec))
  for (uint32_t i = 0; i < iot_data_vector_size (vec); i++)
  {
    CU_ASSERT (iot_data_vector_get (vec, i) == iot_data_vector_get (copy, i))
  }
  iot_data_free (copy);
  iot_data_free (vec);
}

static void test_shallow_copy_list (void)
{
  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (0u));
  iot_data_list_head_push (list, iot_data_alloc_ui32 (1u));
  iot_data_list_tail_push (list, iot_data_alloc_ui32 (2u));
  iot_data_t * copy = iot_data_shallow_copy (list);
  CU_ASSERT (copy != NULL)
  CU_ASSERT (copy != list)
  CU_ASSERT (iot_data_type (copy) == IOT_DATA_LIST)
  CU_ASSERT (iot_data_equal (copy, list))
  iot_data_list_iter_t iter;
  iot_data_list_iter_t iter_copy;
  iot_data_list_iter (list, &iter);
  iot_data_list_iter (copy, &iter_copy);
  while (iot_data_list_iter_next (&iter) && iot_data_list_iter_next (&iter_copy))
  {
    CU_ASSERT (iot_data_list_iter_value (&iter) == iot_data_list_iter_value (&iter_copy))
  }
  iot_data_free (copy);
  iot_data_free (list);
}

static void test_get_at (void)
{
  iot_data_t * map = iot_data_from_json (test_config);
  iot_data_t * path = iot_data_alloc_list ();
  iot_data_list_tail_push (path, iot_data_alloc_string ("Topics", IOT_DATA_REF));
  iot_data_list_tail_push (path, iot_data_alloc_ui32 (0));
  iot_data_list_tail_push (path, iot_data_alloc_string ("Priority", IOT_DATA_REF));
  const iot_data_t * d1 = iot_data_string_map_get_vector (map, "Topics");
  const iot_data_t * d2 = iot_data_vector_get (d1, 0);
  const iot_data_t * d3 = iot_data_string_map_get (d2, "Priority");
  CU_ASSERT (iot_data_get_at (map, path) == d3)
  iot_data_free (path);
  iot_data_free (map);
}

static void test_add_at (void)
{
  iot_data_t * map = iot_data_from_json (test_config);
  iot_data_t * path = iot_data_alloc_list ();
  iot_data_list_tail_push (path, iot_data_alloc_string ("Topics", IOT_DATA_REF));
  iot_data_list_tail_push (path, iot_data_alloc_ui32 (0));
  iot_data_list_tail_push (path, iot_data_alloc_string ("Priority", IOT_DATA_REF));
  iot_data_t * val = iot_data_alloc_ui32 (1);
  iot_data_t * modified = iot_data_add_at (map, path, val);
  CU_ASSERT (modified != NULL)
  CU_ASSERT (iot_data_type (modified) == IOT_DATA_MAP)
  const iot_data_t * d1 = iot_data_string_map_get_vector (modified, "Topics");
  const iot_data_t * d2 = iot_data_vector_get (d1, 0);
  const iot_data_t * d3 = iot_data_string_map_get (d2, "Priority");
  CU_ASSERT (val == d3)
  iot_data_free (modified);
  iot_data_free (path);
  iot_data_free (map);
}

static void test_remove_at (void)
{
  iot_data_t * map = iot_data_from_json (test_config);
  iot_data_t * path = iot_data_alloc_list ();
  iot_data_list_tail_push (path, iot_data_alloc_string ("Topics", IOT_DATA_REF));
  iot_data_list_tail_push (path, iot_data_alloc_ui32 (0));
  iot_data_list_tail_push (path, iot_data_alloc_string ("Priority", IOT_DATA_REF));
  iot_data_t * modified = iot_data_remove_at (map, path);
  CU_ASSERT (modified != NULL)
  CU_ASSERT (iot_data_type (modified) == IOT_DATA_MAP)
  const iot_data_t * d1 = iot_data_string_map_get_vector (modified, "Topics");
  const iot_data_t * d2 = iot_data_vector_get (d1, 0);
  const iot_data_t * d3 = iot_data_string_map_get (d2, "Priority");
  CU_ASSERT (d3 == NULL)
  iot_data_free (modified);
  iot_data_free (path);
  iot_data_free (map);
}

static iot_data_t * add_fn (const iot_data_t * data, void * arg)
{
  return iot_data_alloc_i64 (iot_data_i64 (data) + *((uint64_t *) arg));
}

static void test_update_at (void)
{
  iot_data_t * map = iot_data_from_json (test_config);
  iot_data_t * path = iot_data_alloc_list ();
  iot_data_list_tail_push (path, iot_data_alloc_string ("Topics", IOT_DATA_REF));
  iot_data_list_tail_push (path, iot_data_alloc_ui32 (0u));
  iot_data_list_tail_push (path, iot_data_alloc_string ("Priority", IOT_DATA_REF));
  uint64_t inc = 2;
  iot_data_t * modified = iot_data_update_at (map, path, add_fn, &inc);
  CU_ASSERT (modified != NULL)
  CU_ASSERT (iot_data_type (modified) == IOT_DATA_MAP)
  const iot_data_t * d1 = iot_data_string_map_get_vector (modified, "Topics");
  const iot_data_t * d2 = iot_data_vector_get (d1, 0);
  const int64_t val = iot_data_string_map_get_i64 (d2, "Priority", 0u);
  CU_ASSERT (val == 12)
  iot_data_free (modified);
  iot_data_free (path);
  iot_data_free (map);
}

static void test_data_map_number (void)
{
  bool ok;
  iot_data_t * key = iot_data_alloc_string ("Value", IOT_DATA_REF);
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (map, key, iot_data_alloc_ui16 (123u));

  int8_t i8 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_INT8, &i8);
  CU_ASSERT (ok)
  CU_ASSERT (i8 == 123u)
  uint8_t ui8 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_UINT8, &ui8);
  CU_ASSERT (ok)
  CU_ASSERT (ui8 == 123u)

  int16_t i16 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_INT16, &i16);
  CU_ASSERT (ok)
  CU_ASSERT (i16 == 123u)
  uint16_t ui16 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_UINT16, &ui16);
  CU_ASSERT (ok)
  CU_ASSERT (ui16 == 123u)

  int32_t i32 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_INT32, &i32);
  CU_ASSERT (ok)
  CU_ASSERT (i32 == 123u)
  uint32_t ui32 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_UINT32, &ui32);
  CU_ASSERT (ok)
  CU_ASSERT (ui32 == 123u)

  int64_t i64 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_INT64, &i64);
  CU_ASSERT (ok)
  CU_ASSERT (i64 == 123u)
  uint64_t ui64 = 0;
  ok = iot_data_map_get_number (map, key, IOT_DATA_UINT64, &ui64);
  CU_ASSERT (ok)
  CU_ASSERT (ui64 == 123u)

  ok = iot_data_string_map_get_number (map, "Value", IOT_DATA_INT64, &i64);
  CU_ASSERT (ok)
  CU_ASSERT (i64 == 123u)

  iot_data_free (map);
}

static void test_data_map_int (void)
{
  bool ok;
  iot_data_t * key = iot_data_alloc_string ("Value", IOT_DATA_REF);
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (map, key, iot_data_alloc_ui16 (123u));

  int i = 0;
  ok = iot_data_map_get_int (map, key, &i);
  CU_ASSERT (ok)
  CU_ASSERT (i == 123)
  i = 0;
  ok = iot_data_string_map_get_int (map, "Value", &i);
  CU_ASSERT (ok)
  CU_ASSERT (i == 123)

  iot_data_free (map);
}

static void test_data_map_merge (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * add = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "One", iot_data_alloc_ui32 (1u));
  iot_data_string_map_add (map, "Two", iot_data_alloc_ui32 (2u));
  iot_data_string_map_add (map, "Three", iot_data_alloc_ui32 (33u));
  iot_data_string_map_add (add, "Three", iot_data_alloc_ui32 (3u));
  iot_data_string_map_add (add, "Four", iot_data_alloc_ui32 (4u));
  iot_data_string_map_add (add, "Five", iot_data_alloc_ui32 (5u));

  iot_data_map_merge (map, add);
  iot_data_map_merge (map, NULL);

  CU_ASSERT (iot_data_map_size (map) == 5u)
  const iot_data_t * val = iot_data_string_map_get (map, "Three");
  CU_ASSERT (val && (iot_data_ui32 (val) == 3u))

  iot_data_free (map);
  iot_data_free (add);
}

static void test_array_to_binary (void)
{
 uint8_t data [4] = { 1, 2, 3, 4 };
 iot_data_t * array = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);
 iot_data_array_to_binary (array);
 CU_ASSERT (iot_data_type (array) == IOT_DATA_BINARY)
 iot_data_free (array);
}

static void test_binary_to_array (void)
{
  uint8_t data [4] = { 1, 2, 3, 4 };
  iot_data_t * binary = iot_data_alloc_binary (data, 4u, IOT_DATA_REF);
  iot_data_binary_to_array (binary);
  CU_ASSERT (iot_data_type (binary) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (binary) == IOT_DATA_UINT8)
  iot_data_free (binary);
}

static void test_binary_take (void)
{
  // Buffer not taken as referenced in binary
  uint32_t len;
  uint16_t buff [2u] = { 0u };
  iot_data_t * bin = iot_data_alloc_binary (buff, sizeof (buff), IOT_DATA_REF);
  void * ptr = iot_data_binary_take (bin, &len);
  printf ("Length : %d\n", len);
  CU_ASSERT (len == 4)
  CU_ASSERT (ptr != buff)
  iot_data_free (bin);
  free (ptr);

  // Buffer taken from binary
  void * alloced = malloc (5u);
  bin = iot_data_alloc_binary (alloced, 5u, IOT_DATA_TAKE);
  ptr = iot_data_binary_take (bin, &len);
  CU_ASSERT (len == 5)
  CU_ASSERT (ptr == alloced)
  iot_data_free (bin);
  free (ptr);

  // Buffer taken from binary
  bin = iot_data_alloc_binary (buff, sizeof (buff), IOT_DATA_COPY);
  alloced = (void*) iot_data_address (bin);
  ptr = iot_data_binary_take (bin, &len);
  CU_ASSERT (len == 4)
  CU_ASSERT (ptr == alloced)
  iot_data_free (bin);
  free (ptr);

  // Buffer not taken as binary ref counted
  bin = iot_data_alloc_binary (buff, sizeof (buff), IOT_DATA_COPY);
  iot_data_add_ref (bin);
  alloced = (void*) iot_data_address (bin);
  ptr = iot_data_binary_take (bin, &len);
  CU_ASSERT (len == 4)
  CU_ASSERT (ptr != alloced)
  iot_data_free (bin);
  iot_data_free (bin);
  free (ptr);
}

static void test_data_is_nan (void)
{
  iot_data_t *float_nan = iot_data_alloc_f32 (NAN);
  CU_ASSERT (iot_data_is_nan (float_nan))
  iot_data_free (float_nan);

  iot_data_t *double_nan = iot_data_alloc_f64 (NAN);
  CU_ASSERT (iot_data_is_nan (double_nan))
  iot_data_free (double_nan);

  iot_data_t *float_not_nan = iot_data_alloc_f32 (123.456f);
  CU_ASSERT_FALSE (iot_data_is_nan (float_not_nan))
  iot_data_free (float_not_nan);

  iot_data_t *double_not_nan = iot_data_alloc_f64 (123.456);
  CU_ASSERT_FALSE (iot_data_is_nan (double_not_nan))
  iot_data_free (double_not_nan);

  iot_data_t *non_float_type = iot_data_alloc_i32 (0);
  CU_ASSERT_FALSE (iot_data_is_nan (non_float_type))
  iot_data_free (non_float_type);
}

static void test_data_tags (void)
{
  bool old;
  iot_data_t * data = iot_data_alloc_ui32 (0u);
  CU_ASSERT_FALSE (iot_data_get_tag (data, IOT_DATA_TAG_USER1))
  CU_ASSERT_FALSE (iot_data_get_tag (data, IOT_DATA_TAG_USER2))
  old = iot_data_set_tag (data, IOT_DATA_TAG_USER1, true);
  CU_ASSERT (iot_data_get_tag (data, IOT_DATA_TAG_USER1))
  CU_ASSERT_FALSE (iot_data_get_tag (data, IOT_DATA_TAG_USER2))
  CU_ASSERT_FALSE (old)
  old = iot_data_set_tag (data, IOT_DATA_TAG_USER2, true);
  CU_ASSERT (iot_data_get_tag (data, IOT_DATA_TAG_USER2))
  CU_ASSERT (iot_data_get_tag (data, IOT_DATA_TAG_USER2))
  CU_ASSERT_FALSE (old)
  iot_data_free (data);
}

void cunit_data_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data", suite_init, suite_clean);

  CU_add_test (suite, "data_types", test_data_types);
  CU_add_test (suite, "data_name_type", test_data_name_type);
  CU_add_test (suite, "data_type_string", test_data_type_string);
  CU_add_test (suite, "data_array_key", test_data_array_key);
  CU_add_test (suite, "data_binary", test_data_binary);
  CU_add_test (suite, "data_binary_from_string", test_data_binary_from_string);
  CU_add_test (suite, "data_string_from_binary", test_data_string_from_binary);
  CU_add_test (suite, "data_array_iter_next", test_data_array_iter_next);
  CU_add_test (suite, "data_array_iter_prev", test_data_array_iter_prev);
  CU_add_test (suite, "data_array_iter_uint8", test_data_array_iter_uint8);
  CU_add_test (suite, "data_array_iter_int8", test_data_array_iter_int8);
  CU_add_test (suite, "data_array_iter_uint16", test_data_array_iter_uint16);
  CU_add_test (suite, "data_array_iter_int16", test_data_array_iter_int16);
  CU_add_test (suite, "data_array_iter_int32", test_data_array_iter_int32);
  CU_add_test (suite, "data_array_iter_uint32", test_data_array_iter_uint32);
  CU_add_test (suite, "data_array_iter_int64", test_data_array_iter_int64);
  CU_add_test (suite, "data_array_iter_uint64", test_data_array_iter_uint64);
  CU_add_test (suite, "data_array_iter_float32", test_data_array_iter_float32);
  CU_add_test (suite, "data_array_iter_float64", test_data_array_iter_float64);
  CU_add_test (suite, "data_array_iter_bool", test_data_array_iter_bool);
  CU_add_test (suite, "data_list_size", test_list_size);
  CU_add_test (suite, "data_list_free", test_list_free);
  CU_add_test (suite, "data_list_iter", test_list_iter);
  CU_add_test (suite, "data_list_copy", test_list_copy);
  CU_add_test (suite, "data_typed_list_iter", test_typed_list_iter);
  CU_add_test (suite, "data_list_iter_replace", test_list_iter_replace);
  CU_add_test (suite, "data_list_iter_remove", test_list_iter_remove);
  CU_add_test (suite, "data_list_iter_remove_all", test_list_iter_remove_all);
  CU_add_test (suite, "data_list_remove", test_list_remove);
  CU_add_test (suite, "data_list_find", test_list_find);
  CU_add_test (suite, "data_list_equal", test_list_equal);
  CU_add_test (suite, "data_map_size", test_map_size);
  CU_add_test (suite, "data_map_get", test_map_get);
  CU_add_test (suite, "data_map_iter_replace", test_data_map_iter_replace);
  CU_add_test (suite, "data_map_remove", test_data_map_remove);
  CU_add_test (suite, "data_map_get_list", test_data_map_get_list);
  CU_add_test (suite, "data_map_add_unused", test_data_map_add_unused);
  CU_add_test (suite, "data_map_get_array", test_data_map_get_array);
  CU_add_test (suite, "data_string_vector", test_data_string_vector);
  CU_add_test (suite, "data_address", test_data_address);
  CU_add_test (suite, "data_infinite", test_data_infinite);
  CU_add_test (suite, "data_from_string", test_data_from_string);
  CU_add_test (suite, "data_from_strings", test_data_from_strings);
  CU_add_test (suite, "data_from_base64", test_data_from_base64);
  CU_add_test (suite, "data_map_base64_to_array", test_data_map_base64_to_array);
  CU_add_test (suite, "data_increment", test_data_increment);
  CU_add_test (suite, "data_decrement", test_data_decrement);
  CU_add_test (suite, "data_vector_iter_replace", test_data_vector_iter_replace);
  CU_add_test (suite, "data_map_typed_iter", test_data_map_typed_iter);
  CU_add_test (suite, "data_check_equal_int8", test_data_equal_int8);
  CU_add_test (suite, "data_check_equal_uint16", test_data_equal_uint16);
  CU_add_test (suite, "data_check_equal_float32", test_data_equal_float32);
  CU_add_test (suite, "data_check_equal_string", test_data_equal_string);
  CU_add_test (suite, "data_check_equal_vector_ui8", test_data_equal_vector_ui8);
  CU_add_test (suite, "data_check_equal_vector_ui8_refcount", test_data_equal_vector_ui8_refcount);
  CU_add_test (suite, "data_check_unequal_vector_ui8", test_data_unequal_vector_ui8);
  CU_add_test (suite, "data_check_equal_vector_string", test_data_equal_vector_string);
  CU_add_test (suite, "data_check_equal_vector_ui8", test_data_equal_vector_ui8);
  CU_add_test (suite, "data_check_equal_null", test_data_equal_null);
  CU_add_test (suite, "data_check_equal_vector_ui8_refcount", test_data_equal_vector_ui8_refcount);
  CU_add_test (suite, "data_check_unequal_vector_ui8", test_data_unequal_vector_ui8);
  CU_add_test (suite, "data_check_equal_vector_string", test_data_equal_vector_string);
  CU_add_test (suite, "data_check_equal_array", test_data_equal_array);
  CU_add_test (suite, "data_check_equal_map", test_data_equal_map);
  CU_add_test (suite, "data_map_empty", test_data_map_empty);
  CU_add_test (suite, "data_check_map_null_ret", test_data_check_map_null_ret);
  CU_add_test (suite, "data_check_equal_map_refcount", test_data_equal_map_refcount);
  CU_add_test (suite, "data_check_unequal_map_size", test_data_unequal_map_size);
  CU_add_test (suite, "data_check_unequal_key_map", test_data_unequal_key_map);
  CU_add_test (suite, "data_check_unequal_value_map", test_data_unequal_value_map);
  CU_add_test (suite, "data_check_equal_nested_vector", test_data_equal_nested_vector);
  CU_add_test (suite, "data_check_unequal_nested_vector", test_data_unequal_nested_vector);
  CU_add_test (suite, "data_check_equal_vector_map", test_data_equal_vector_map);
  CU_add_test (suite, "data_check_unequal_vector_map", test_data_unequal_vector_map);
  CU_add_test (suite, "data_copy_null", test_data_copy_null);
  CU_add_test (suite, "data_copy_int8", test_data_copy_i8);
  CU_add_test (suite, "data_copy_ui8", test_data_copy_ui8);
  CU_add_test (suite, "data_copy_uint16", test_data_copy_uint16);
  CU_add_test (suite, "data_copy_float64", test_data_copy_float64);
  CU_add_test (suite, "data_copy_string", test_data_copy_string);
  CU_add_test (suite, "data_copy_array", test_data_copy_array);
  CU_add_test (suite, "data_copy_array_chars", test_data_copy_array_chars);
  CU_add_test (suite, "data_copy_vector_ui8", test_data_copy_vector_ui8);
  CU_add_test (suite, "data_copy_vector_strings", test_data_copy_vector_strings);
  CU_add_test (suite, "data_copy_nested_vector", test_data_copy_nested_vector);
  CU_add_test (suite, "data_copy_map", test_data_copy_map);
  CU_add_test (suite, "data_copy_map_update", test_data_copy_map_update);
  CU_add_test (suite, "data_copy_map_update_value", test_data_copy_map_update_value);
  CU_add_test (suite, "data_copy_vector_map", test_data_copy_vector_map);
  CU_add_test (suite, "data_vector_iter_next", test_data_vector_iter_next);
  CU_add_test (suite, "data_vector_typed_iter", test_data_vector_typed_iter);
  CU_add_test (suite, "data_vector_iter_prev", test_data_vector_iter_prev);
  CU_add_test (suite, "data_vector_iters", test_data_vector_iters);
  CU_add_test (suite, "data_vector_resize", test_data_vector_resize);
  CU_add_test (suite, "data_vector_compact", test_data_vector_compact);
  CU_add_test (suite, "data_vector_find", test_data_vector_find);
  CU_add_test (suite, "data_copy_map_base64_to_array", test_data_copy_map_base64_to_array);
  CU_add_test (suite, "data_check_equal_nested_vector", test_data_equal_nested_vector);
  CU_add_test (suite, "data_check_unequal_nested_vector", test_data_unequal_nested_vector);
  CU_add_test (suite, "data_check_equal_vector_map", test_data_equal_vector_map);
  CU_add_test (suite, "data_check_unequal_vector_map", test_data_unequal_vector_map);
  CU_add_test (suite, "data_metadata", test_data_metadata);
  CU_add_test (suite, "data_multi_metadata", test_data_multi_metadata);
  CU_add_test (suite, "data_alloc_array_int8", test_data_alloc_array_i8);
  CU_add_test (suite, "data_alloc_array_uint8", test_data_alloc_array_ui8);
  CU_add_test (suite, "data_alloc_array_int16", test_data_alloc_array_i16);
  CU_add_test (suite, "data_alloc_array_uint16", test_data_alloc_array_ui16);
  CU_add_test (suite, "data_alloc_array_int32", test_data_alloc_array_i32);
  CU_add_test (suite, "data_alloc_array_uint32", test_data_alloc_array_ui32);
  CU_add_test (suite, "data_alloc_array_int64", test_data_alloc_array_i64);
  CU_add_test (suite, "data_alloc_array_uint64", test_data_alloc_array_ui64);
  CU_add_test (suite, "data_alloc_array_float32", test_data_alloc_array_f32);
  CU_add_test (suite, "data_alloc_array_float64", test_data_alloc_array_f64);
  CU_add_test (suite, "data_alloc_array_bool", test_data_alloc_array_bool);
  CU_add_test (suite, "data_alloc_zerolength_array", test_data_zerolength_array);
  CU_add_test (suite, "data_alloc_zerolength_vector", test_data_zerolength_vector);
  CU_add_test (suite, "data_alloc_zerolength_vectormap", test_data_zerolength_vectormap);
  CU_add_test (suite, "data_basic_typecode", test_data_basic_typecode);
  CU_add_test (suite, "data_complex_typecode", test_data_complex_typecode);
  CU_add_test (suite, "data_equal_typecode", test_data_equal_typecode);
  CU_add_test (suite, "data_map_perf", test_data_map_perf);
  CU_add_test (suite, "data_int_map", test_data_int_map);
  CU_add_test (suite, "data_add_ref", test_data_add_ref);
  CU_add_test (suite, "data_alloc_uuid", test_data_alloc_uuid);
  CU_add_test (suite, "data_alloc_pointer", test_data_alloc_pointer);
  CU_add_test (suite, "data_alloc_heap", test_data_alloc_heap);
  CU_add_test (suite, "data_cast", test_data_cast);
  CU_add_test (suite, "data_const_string", test_data_const_string);
  CU_add_test (suite, "data_const_ui64", test_data_const_ui64);
  CU_add_test (suite, "data_const_pointer", test_data_const_pointer);
  CU_add_test (suite, "data_const_list", test_data_const_list);
  CU_add_test (suite, "data_const_types", test_data_const_types);
  CU_add_test (suite, "data_hash", test_data_hash);
  CU_add_test (suite, "data_multi_key_map", test_data_multi_key_map);
  CU_add_test (suite, "data_map_struct_key", test_data_map_struct_key);
  CU_add_test (suite, "data_list_hash", test_data_list_hash);
  CU_add_test (suite, "data_vector_hash", test_data_vector_hash);
  CU_add_test (suite, "data_compare", test_data_compare);
  CU_add_test (suite, "data_compress", test_data_compress);
  CU_add_test (suite, "data_map_number", test_data_map_number);
  CU_add_test (suite, "data_map_int", test_data_map_int);
  CU_add_test (suite, "data_map_merge", test_data_map_merge);
  CU_add_test (suite, "data_vector_to_array", test_data_vector_to_array);
  CU_add_test (suite, "data_vector_to_vector", test_data_vector_to_vector);
  CU_add_test (suite, "data_nested_vector_to_array", test_data_nested_vector_to_array);
  CU_add_test (suite, "data_ref_count", test_data_ref_count);
  CU_add_test (suite, "data_array_transform", test_data_array_transform);
  CU_add_test (suite, "data_transform", test_data_transform);
  CU_add_test (suite, "vector_elements", test_vector_elements);
  CU_add_test (suite, "array_dimensions", test_array_dimensions);
  CU_add_test (suite, "vector_dimensions", test_vector_dimensions);
  CU_add_test (suite, "data_map_iter_start_end", test_data_map_iter_start_end);
  CU_add_test (suite, "shallow_copy_map", test_shallow_copy_map);
  CU_add_test (suite, "shallow_copy_vector", test_shallow_copy_vector);
  CU_add_test (suite, "shallow_copy_list", test_shallow_copy_list);
  CU_add_test (suite, "get_at", test_get_at);
  CU_add_test (suite, "add_at", test_add_at);
  CU_add_test (suite, "remove_at", test_remove_at);
  CU_add_test (suite, "update_at", test_update_at);
  CU_add_test (suite, "array_to_binary", test_array_to_binary);
  CU_add_test (suite, "binary_to_array", test_binary_to_array);
  CU_add_test (suite, "binary_take", test_binary_take);
  CU_add_test (suite, "data_is_nan", test_data_is_nan);
  CU_add_test (suite, "data_tags", test_data_tags);
}
