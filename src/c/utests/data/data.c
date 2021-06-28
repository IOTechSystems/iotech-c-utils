/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/time.h"
#include "data.h"
#include "CUnit.h"
#include <float.h>

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
  iot_data_free (data);
  data = iot_data_alloc_ui8 (1u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt8") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8)
  iot_data_free (data);
  data = iot_data_alloc_i16 (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int16") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT16)
  iot_data_free (data);
  data = iot_data_alloc_ui16 (2u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt16") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT16)
  iot_data_free (data);
  data = iot_data_alloc_i32 (3);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int32") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32)
  iot_data_free (data);
  data = iot_data_alloc_ui32 (3u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt32") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT32)
  iot_data_free (data);
  data = iot_data_alloc_i64 (4);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int64") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT64)
  iot_data_free (data);
  data = iot_data_alloc_ui64 (4u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt64") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT64)
  iot_data_free (data);
  data = iot_data_alloc_f32 (5.0f);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Float32") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT32)
  iot_data_free (data);
  data = iot_data_alloc_f64 (6.0);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Float64") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT64)
  iot_data_free (data);
  data = iot_data_alloc_bool (true);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Bool") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL)
  iot_data_free (data);
  data = iot_data_alloc_string ("Hello", IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  iot_data_free (data);
  data = iot_data_alloc_string ("Hello", IOT_DATA_COPY);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  iot_data_free (data);
  data = iot_data_alloc_string (strdup ("Hello"), IOT_DATA_TAKE);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  iot_data_free (data);
  data = iot_data_alloc_array (array, 4, IOT_DATA_UINT8, IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_array (calloc (1, sizeof (array)), 4, IOT_DATA_UINT8, IOT_DATA_TAKE);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_array (array, 4, IOT_DATA_UINT8, IOT_DATA_COPY);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_map (IOT_DATA_UINT32);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Map") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_MAP)
  CU_ASSERT (iot_data_map_key_type (data) == IOT_DATA_UINT32)
  iot_data_free (data);
  data = iot_data_alloc_vector (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Vector") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_VECTOR)
  iot_data_free (data);
  data = iot_data_alloc_null ();
  CU_ASSERT (strcmp (iot_data_type_name (data), "Null") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_NULL)
  iot_data_free (data);
}

static void test_data_array_key (void)
{
  uint8_t data1 [4] = { 0, 1, 2 ,3 };
  uint8_t data2 [4] = { 0, 1, 2 ,4 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_ARRAY);
  CU_ASSERT (iot_data_map_key_type (map) == IOT_DATA_ARRAY)
  iot_data_t * array1 = iot_data_alloc_array (data1, sizeof (data1), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * array2 = iot_data_alloc_array (data2, sizeof (data2), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_ui32 (66u);
  iot_data_t * duffkey = iot_data_alloc_i32 (55);
  iot_data_map_add (map, array1, val);
  const iot_data_t * ret = iot_data_map_get (map, array1);
  CU_ASSERT (ret == val)
  ret = iot_data_map_get (map, array2);
  CU_ASSERT (ret == NULL)
  ret = iot_data_map_get (map, duffkey);
  CU_ASSERT (ret == NULL)
  iot_data_free (array2);
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

  iot_data_t * array = iot_data_alloc_array (data, sizeof (data) / sizeof (bool), IOT_DATA_BOOL, IOT_DATA_REF);
  iot_data_array_iter (array, &array_iter);

  while (iot_data_array_iter_next (&array_iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&array_iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&array_iter) != NULL)
    CU_ASSERT (*((bool*)iot_data_array_iter_value (&array_iter)) == data[index])
    index++;
  }

  iot_data_free (array);
}

static void test_data_string_vector (void)
{
  const char * strs [2] = { "Test", "Tube" };
  uint32_t index = 0;
  iot_data_vector_iter_t iter;
  iot_data_t * vector = iot_data_alloc_vector (2);
  iot_data_t * str1 = iot_data_alloc_string (strs[0], IOT_DATA_REF);
  iot_data_t * str2 = iot_data_alloc_string (strs[1], IOT_DATA_REF);
  iot_data_vector_add (vector, 0, str1);
  iot_data_vector_add (vector, 1, str2);
  iot_data_vector_iter (vector, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_vector_iter_string (&iter) == strs[index])
    index++;
  }
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

static void test_data_to_json (void)
{
  char * json;
  uint8_t data [4] = { 0, 1, 2, 3 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_t * key = iot_data_alloc_string ("UInt32", IOT_DATA_REF);
  iot_data_t * array = iot_data_alloc_array (data, sizeof (data), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  val = iot_data_alloc_string ("Lilith", IOT_DATA_REF);
  key = iot_data_alloc_string ("Name", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("Data", IOT_DATA_REF);
  iot_data_map_add (map, key, array);
  key = iot_data_alloc_string ("Escaped", IOT_DATA_REF);
  val = iot_data_alloc_string ("abc\t\n123\x0b\x1fxyz", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("Boolean", IOT_DATA_REF);
  val = iot_data_alloc_bool (true);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("NULL", IOT_DATA_REF);
  val = iot_data_alloc_null ();
  iot_data_map_add (map, key, val);
  json = iot_data_to_json (map);
  CU_ASSERT (json != NULL)
  if (json)
  {
    printf ("JSON: %s\n", json);
    CU_ASSERT (strcmp (json, "{\"UInt32\":1,\"Name\":\"Lilith\",\"Data\":\"AAECAw==\",\"Escaped\":\"abc\\t\\n123\\u000b\\u001fxyz\",\"Boolean\":true,\"NULL\":null}") == 0)
  }
  free (json);
  iot_data_free (map);

  /* Test with non string key type */
  map = iot_data_alloc_map (IOT_DATA_UINT32);
  val = iot_data_alloc_string ("Cthulhu", IOT_DATA_REF);
  key = iot_data_alloc_ui32 (1u);
  iot_data_map_add (map, key, val);
  val = iot_data_alloc_string ("Rules", IOT_DATA_REF);
  key = iot_data_alloc_ui32 (2u);
  iot_data_map_add (map, key, val);
  json = iot_data_to_json (map);
  CU_ASSERT (json != NULL)
  if (json)
  {
    CU_ASSERT (strcmp (json, "{\"1\":\"Cthulhu\",\"2\":\"Rules\"}") == 0)
  }
  free (json);
  iot_data_free (map);

  /* Test with maximum float size */

  val = iot_data_alloc_f64 (DBL_MAX);
  json = iot_data_to_json (val);
  CU_ASSERT (json != NULL)
  iot_data_free (val);
  free (json);
}

static void test_data_from_json (void)
{
  static const char * config =
  "{"
    "\"Interval\":100000,"
    "\"Scheduler\":\"scheduler\","
    "\"ThreadPool\":\"pool\","
    "\"Topics\": [{\"Topic\":\"test/tube\",\"Priority\":10,\"Retain\":true}],"
    "\"Null\": null,"
    "\"Boolean\":true,"
    "\"Numbers\":{ \"One\":1, \"Two\":2, \"Three\":3 },"
    "\"Vector\":[ \"A\",\"B\"],"
    "\"DB\":0.5,"
    "\"Escaped\":\"Double \\\" Quote\""
  "}";
  bool bval = false;
  const char * sval = NULL;
  double dval = 1.0;
  int64_t ival = 0;
  bool found;
  const iot_data_t * data;

  iot_data_t * map = iot_data_from_json (config);
  CU_ASSERT (map != NULL)

  found = iot_config_bool (map, "Boolean", &bval, NULL);
  CU_ASSERT (found)
  CU_ASSERT (bval)
  found = iot_config_bool (map, "Bolean", &bval, NULL);
  CU_ASSERT (! found)

  sval = iot_config_string (map, "Scheduler", false,NULL);
  CU_ASSERT (sval != NULL)
  CU_ASSERT (strcmp (sval, "scheduler") == 0)
  sval = iot_config_string (map, "Sched", false, NULL);
  CU_ASSERT (sval == NULL)

  sval = iot_config_string (map, "Escaped", false, NULL);
  CU_ASSERT (sval != NULL)
  CU_ASSERT (strcmp (sval, "Double \" Quote") == 0)

  sval = iot_config_string_default (map, "Scheduler", "Hello", false);
  CU_ASSERT (sval != NULL)
  CU_ASSERT (strcmp (sval, "scheduler") == 0)
  sval = iot_config_string_default (map, "Nope", "Hello", true);
  CU_ASSERT (sval != NULL)
  CU_ASSERT (strcmp (sval, "Hello") == 0)
  free ((void*) sval);
  sval = iot_config_string_default (map, "Nope", NULL, true);
  CU_ASSERT (sval == NULL)

  data = iot_data_string_map_get (map, "Null");
  CU_ASSERT (iot_data_type (data) == IOT_DATA_NULL)

  found = iot_config_f64 (map, "DB", &dval, NULL);
  CU_ASSERT (found)
  CU_ASSERT (dval < 1.0)
  found = iot_config_f64 (map, "B", &dval, NULL);
  CU_ASSERT (! found)

  found = iot_config_i64 (map, "Interval", &ival, NULL);
  CU_ASSERT (found)
  CU_ASSERT (ival == 100000)
  found = iot_config_i64 (map, "Int", &ival, NULL);
  CU_ASSERT (! found)

  dval = 7.7;
  dval = iot_data_string_map_get_f64 (map, "DB", 1.0);
  CU_ASSERT (dval < 1.0)

  const iot_data_t * vector = iot_config_vector (map, "Topics", NULL);
  CU_ASSERT (vector != NULL)
  vector = iot_config_vector (map, "opics", NULL);
  CU_ASSERT (vector == NULL)

  const iot_data_t * map2 = iot_config_map (map, "Numbers", NULL);
  CU_ASSERT (map2 != NULL)
  map2 = iot_config_map (map, "umbers", NULL);
  CU_ASSERT (map2 == NULL)
  map2 = iot_data_string_map_get_map (map, "Numbers");
  CU_ASSERT (map2 != NULL)

  const iot_data_t * vec = iot_data_string_map_get_vector (map, "Vector");
  CU_ASSERT (vec != NULL)

  iot_data_free (map);

  iot_data_t * nd = iot_data_from_json (NULL);
  CU_ASSERT (nd != NULL)
  CU_ASSERT (iot_data_type (nd) == IOT_DATA_NULL)
  iot_data_free (nd);
  nd = iot_data_from_json ("");
  CU_ASSERT (nd != NULL)
  CU_ASSERT (iot_data_type (nd) == IOT_DATA_NULL)
  iot_data_free (nd);
}

#ifdef IOT_HAS_XML
static void test_data_from_xml (void)
{
  iot_data_t * xml;
  char * json;
  const char * test_xml = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n\
<busmaster xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" busId=\"main_bus\">\n\
  <deviceService name=\"virtual_device_service\" library=\"libxrt-virtual-device-service.so\" factory=\"xrt_virtual_device_service_factory\" topic=\"virtual_device_service/data\">\n\
    <device name=\"Random-Integer-Device\" profile=\"Random-Integer-Device\">\n\
      <resource name=\"RandomValue_Int8\" schedule=\"500000000\" />\n\
      <protocol name=\"Other\">\n\
        <protocolAttribute name=\"Address\" value=\"device-virtual-int-01\" />Any old rubbish\n\
      </protocol>\n\
    </device>\n\
  </deviceService>\n\
  <fubar>Some text!</fubar>\n\
  <container threads=\"4\">\n\
    <logging enable=\"true\" filename=\"/dev/null\" />\n\
  </container>\n\
</busmaster>";
  const char * expected = "{\"name\":\"busmaster\",\"attributes\":{\"xmlns:xsi\":\"http://www.w3.org/2001/XMLSchema-instance\",\"xmlns:xsd\":\"http://www.w3.org/2001/XMLSchema\",\"busId\":\"main_bus\"},\"children\":[{\"name\":\"deviceService\",\"attributes\":{\"name\":\"virtual_device_service\",\"library\":\"libxrt-virtual-device-service.so\",\"factory\":\"xrt_virtual_device_service_factory\",\"topic\":\"virtual_device_service/data\"},\"children\":[{\"name\":\"device\",\"attributes\":{\"name\":\"Random-Integer-Device\",\"profile\":\"Random-Integer-Device\"},\"children\":[{\"name\":\"resource\",\"attributes\":{\"name\":\"RandomValue_Int8\",\"schedule\":\"500000000\"}},{\"name\":\"protocol\",\"attributes\":{\"name\":\"Other\"},\"children\":[{\"name\":\"protocolAttribute\",\"attributes\":{\"name\":\"Address\",\"value\":\"device-virtual-int-01\"}}],\"content\":\"Any old rubbish\\n      \"}],\"content\":\"\\n    \"}],\"content\":\"\\n  \"},{\"name\":\"fubar\",\"attributes\":{},\"content\":\"Some text!\"},{\"name\":\"container\",\"attributes\":{\"threads\":\"4\"},\"children\":[{\"name\":\"logging\",\"attributes\":{\"enable\":\"true\",\"filename\":\"/dev/null\"}}],\"content\":\"\\n  \"}],\"content\":\"\\n\"}";


  xml = iot_data_from_xml (test_xml);
  CU_ASSERT (xml != NULL);
  json = iot_data_to_json (xml);
  CU_ASSERT (json != NULL);
  CU_ASSERT (strcmp (json, expected) == 0);
  free (json);
  iot_data_free (xml);
}
#endif

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
  CU_ASSERT (iot_data_name_type ("string") == IOT_DATA_STRING)
  CU_ASSERT (iot_data_name_type ("array") == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_name_type ("map") == IOT_DATA_MAP)
  CU_ASSERT (iot_data_name_type ("vector") == IOT_DATA_VECTOR)
  CU_ASSERT (iot_data_name_type ("null") == IOT_DATA_NULL)
  CU_ASSERT (iot_data_name_type ("dummy") == (iot_data_type_t) -1)
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
  data = iot_data_alloc_from_string (IOT_DATA_STRING, "Wibble");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_data_string (data), "Wibble") == 0)
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
  iot_data_free (data);
}

static void test_data_equal_vector_ui8 (void)
{
  uint32_t vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);

  while (vector_index < 5)
  {
    iot_data_vector_add (vector1, vector_index, iot_data_alloc_ui8 (vector_index));
    iot_data_vector_add (vector2, vector_index, iot_data_alloc_ui8 (vector_index));
    vector_index++;
  }

  CU_ASSERT (iot_data_equal (vector1, vector2))
  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_equal_vector_ui8_refcount (void)
{
  uint32_t vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);

  while (vector_index < 5)
  {
    iot_data_t *value = iot_data_alloc_ui8(vector_index);

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
    iot_data_vector_add (vector1, vector_index, iot_data_alloc_ui8(vector_index));
    iot_data_vector_add (vector2, vector_index, iot_data_alloc_ui8(vector_index+1));

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
  iot_data_t * data1 = iot_data_alloc_i8 (-128);
  CU_ASSERT (data1 != NULL)

  iot_data_t * data2 = iot_data_alloc_string ("Hello", IOT_DATA_REF);
  CU_ASSERT (data2 != NULL)

  iot_data_set_metadata (data1, data2);
  const iot_data_t * metadata1 = iot_data_get_metadata (data1);

  CU_ASSERT (metadata1 != NULL)
  CU_ASSERT (iot_data_type (metadata1) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_equal (data2, metadata1))
  CU_ASSERT (iot_data_get_metadata (NULL) == NULL)

  iot_data_free (data1);
  iot_data_free (data2);

  data1 = iot_data_alloc_ui64 (9999999999);
  CU_ASSERT (data1 != NULL)

  data2 = iot_data_alloc_i32 (2111111);
  CU_ASSERT (data2 != NULL)

  iot_data_set_metadata (data1, data2);
  metadata1 = iot_data_get_metadata (data1);

  CU_ASSERT (metadata1 != NULL)
  CU_ASSERT (iot_data_type (metadata1) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_equal (data2, metadata1))

  iot_data_free (data1);
  iot_data_free (data2);

  data1 = iot_data_alloc_f32 (1.299999f);
  CU_ASSERT (data1 != NULL)

  data2 = iot_data_alloc_string ("qwertyQWERTY", IOT_DATA_REF);
  CU_ASSERT (data2 != NULL)

  iot_data_set_metadata (data1, data2);
  metadata1 = iot_data_get_metadata (data1);

  CU_ASSERT (metadata1 != NULL)
  CU_ASSERT (iot_data_type (metadata1) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_equal (data2, metadata1))

  // When data copied, metadata should also be copied.
  iot_data_t * data3 = iot_data_copy (data1);
  const iot_data_t * metadata2 = iot_data_get_metadata (data3);

  CU_ASSERT (metadata2 != NULL)
  CU_ASSERT (iot_data_equal (metadata2, metadata1))

  iot_data_free (data1);
  iot_data_free (data2);
  iot_data_free (data3);
}

static bool string_match (const iot_data_t * data, const void * arg)
{
  const char * target = (const char *) arg;
  const char * val = iot_data_string (data);
  return strcmp (target, val) == 0;
}

static void test_data_vector_iter_next (void)
{
  uint8_t index = 0;
  uint8_t data[2] = { 0, 1 };

  iot_data_t * vector = iot_data_alloc_vector (2);
  iot_data_t * value1 = iot_data_alloc_ui8 (data[0]);
  iot_data_t * value2 = iot_data_alloc_ui8 (data[1]);

  iot_data_vector_iter_t iter;

  iot_data_vector_add (vector, 0, value1);
  iot_data_vector_add (vector, 1, value2);
  iot_data_vector_iter (vector, &iter);

  for (int i = 0; i<4; i++)
  {
    while (iot_data_vector_iter_next (&iter))
    {
      CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
      CU_ASSERT (iot_data_vector_iter_value (&iter) != NULL)
      CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == data[index])
      index++;
    }
    index = 0;

    CU_ASSERT (iot_data_vector_iter_next (&iter) == true)
    CU_ASSERT (iot_data_vector_iter_index (&iter) == index)
    CU_ASSERT (iot_data_ui8 (iot_data_vector_iter_value (&iter)) == data[index])
    index++;
  }

  iot_data_free (vector);
}

static void test_data_vector_resize (void)
{
  iot_data_t * vector = iot_data_alloc_vector (3);
  iot_data_t * elem0 = iot_data_alloc_string ("test", IOT_DATA_REF);
  iot_data_t * elem1 = iot_data_alloc_string ("test1", IOT_DATA_REF);
  iot_data_t * elem2 = iot_data_alloc_string ("test2", IOT_DATA_REF);
  iot_data_vector_add (vector, 0, elem0);
  iot_data_vector_add (vector, 1, elem1);
  iot_data_vector_add (vector, 2, elem2);
  iot_data_vector_resize (vector, 5);
  CU_ASSERT (iot_data_vector_size (vector) == 5)
  CU_ASSERT (iot_data_vector_get (vector, 0) == elem0)
  CU_ASSERT (iot_data_vector_get (vector, 1) == elem1)
  CU_ASSERT (iot_data_vector_get (vector, 2) == elem2)
  CU_ASSERT (iot_data_vector_get (vector, 3) == NULL)
  CU_ASSERT (iot_data_vector_get (vector, 4) == NULL)
  iot_data_vector_resize (vector, 2);
  CU_ASSERT (iot_data_vector_size (vector) == 2)
  CU_ASSERT (iot_data_vector_get (vector, 0) == elem0)
  CU_ASSERT (iot_data_vector_get (vector, 1) == elem1)
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
  CU_ASSERT (iot_data_vector_find (vector, string_match, "foo") == NULL)
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
    iot_data_vector_add (vector1, index, iot_data_alloc_ui8 (index));
    index++;
  }

  iot_data_t * vector2 = iot_data_copy (vector1);
  CU_ASSERT (iot_data_equal (vector1, vector2))

  //update array2
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
extern void iot_data_map_dump (iot_data_t * map);

static void test_map_size (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  CU_ASSERT (!iot_data_map_size (map))

  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_t * key = iot_data_alloc_string ("element1", IOT_DATA_REF);

  iot_data_map_add (map, key, val);
  iot_data_add_ref (key);
  iot_data_map_dump (map);

  CU_ASSERT (iot_data_map_size (map) == 1)

  /* update value for the same key */
  val = iot_data_alloc_ui32 (2u);
  iot_data_map_add (map, key, val);

  CU_ASSERT (iot_data_map_size (map) == 1)

  iot_data_string_map_add (map, "element2", iot_data_alloc_string ("data", IOT_DATA_REF));
  iot_data_map_dump (map);
  CU_ASSERT (iot_data_map_size (map) == 2)

  iot_data_free (map);
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

static void test_data_map_iter_replace (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (map, iot_data_alloc_string ("1", IOT_DATA_REF), iot_data_alloc_string ("One", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("2", IOT_DATA_REF), iot_data_alloc_string ("Two", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("3", IOT_DATA_REF), iot_data_alloc_string ("Three", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_string ("4", IOT_DATA_REF), iot_data_alloc_null ());

  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "1"), "One") == 0)
  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "3"), "Three") == 0)
  CU_ASSERT (iot_data_string_map_get_string (map, "4") == NULL)

  char * json = iot_data_to_json (map);
  printf ("JSON: %s\n", json);

  iot_data_map_iter_t it;
  iot_data_map_iter (map, &it);
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
  }

  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "1"), "Ace") == 0)
  CU_ASSERT (strcmp (iot_data_string_map_get_string (map, "3"), "Trey") == 0)

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
  CU_ASSERT (strcmp (json, "\"\"") == 0)
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
  iot_typecode_t * tc;

  tc = iot_typecode_alloc_basic (IOT_DATA_INT8);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_INT8)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Int8") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_UINT8);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_UINT8)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "UInt8") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_INT16);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_INT16)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Int16") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_UINT16);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_UINT16)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "UInt16") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_INT32);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_INT32)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Int32") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_UINT32);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_UINT32)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "UInt32") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_INT64);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_INT64)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Int64") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_UINT64);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_UINT64)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "UInt64") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_FLOAT32);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_FLOAT32)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Float32") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_FLOAT64);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_FLOAT64)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Float64") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_BOOL);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_BOOL)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Bool") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_STRING);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "String") == 0)
  iot_typecode_free (tc);
  tc = iot_typecode_alloc_basic (IOT_DATA_NULL);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_NULL)
  CU_ASSERT (strcmp (iot_typecode_type_name (tc), "Null") == 0)
  iot_typecode_free (tc);
}

static void test_data_complex_typecode (void)
{
  iot_typecode_t * tc;
  const iot_typecode_t * et;

  tc = iot_typecode_alloc_array (IOT_DATA_INT8);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_ARRAY)
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (et) == IOT_DATA_INT8)
  iot_typecode_free (tc);

  tc = iot_typecode_alloc_vector (iot_typecode_alloc_basic (IOT_DATA_STRING));
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_VECTOR)
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (et) == IOT_DATA_STRING)
  iot_typecode_free (tc);

  tc = iot_typecode_alloc_map (IOT_DATA_UINT32, iot_typecode_alloc_basic (IOT_DATA_BOOL));
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_MAP)
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (et) == IOT_DATA_BOOL)
  CU_ASSERT (iot_typecode_key_type (tc) == IOT_DATA_UINT32)
  iot_typecode_free (tc);
}

static void test_data_equal_typecode (void)
{
  iot_typecode_t * tc1;
  iot_typecode_t * tc2;
  iot_typecode_t * tc3;
  iot_typecode_t * tc4;

  tc1 = iot_typecode_alloc_basic (IOT_DATA_INT32);
  tc2 = iot_typecode_alloc_basic (IOT_DATA_UINT32);
  tc3 = iot_typecode_alloc_basic (IOT_DATA_INT32);
  tc4 = NULL;
  CU_ASSERT (iot_typecode_equal (tc1, tc1))
  CU_ASSERT (iot_typecode_equal (tc2, tc2))
  CU_ASSERT (iot_typecode_equal (tc3, tc3))
  CU_ASSERT (iot_typecode_equal (tc4, tc4))
  CU_ASSERT (iot_typecode_equal (tc1, tc3))
  CU_ASSERT (! iot_typecode_equal (tc1, tc2))
  CU_ASSERT (! iot_typecode_equal (tc1, tc4))
  iot_typecode_free (tc1);
  iot_typecode_free (tc2);
  iot_typecode_free (tc3);
  iot_typecode_free (tc4);

  tc1 = iot_typecode_alloc_array (IOT_DATA_INT8);
  tc2 = iot_typecode_alloc_array (IOT_DATA_INT8);
  tc3 = iot_typecode_alloc_array (IOT_DATA_UINT16);
  CU_ASSERT (iot_typecode_equal (tc1, tc2))
  CU_ASSERT (! iot_typecode_equal (tc1, tc3))
  iot_typecode_free (tc1);
  iot_typecode_free (tc2);
  iot_typecode_free (tc3);

  tc1 = iot_typecode_alloc_vector (iot_typecode_alloc_basic (IOT_DATA_FLOAT32));
  tc2 = iot_typecode_alloc_vector (iot_typecode_alloc_basic (IOT_DATA_FLOAT32));
  tc3 = iot_typecode_alloc_vector (iot_typecode_alloc_basic (IOT_DATA_BOOL));
  CU_ASSERT (iot_typecode_equal (tc1, tc2))
  CU_ASSERT (! iot_typecode_equal (tc1, tc3))
  iot_typecode_free (tc1);
  iot_typecode_free (tc2);
  iot_typecode_free (tc3);

  tc1 = iot_typecode_alloc_map (IOT_DATA_INT16, iot_typecode_alloc_basic (IOT_DATA_FLOAT32));
  tc2 = iot_typecode_alloc_map (IOT_DATA_INT16, iot_typecode_alloc_basic (IOT_DATA_FLOAT32));
  tc3 = iot_typecode_alloc_map (IOT_DATA_INT16, iot_typecode_alloc_basic (IOT_DATA_BOOL));
  tc4 = iot_typecode_alloc_map (IOT_DATA_STRING, iot_typecode_alloc_basic (IOT_DATA_FLOAT32));
  CU_ASSERT (iot_typecode_equal (tc1, tc2))
  CU_ASSERT (! iot_typecode_equal (tc1, tc3))
  CU_ASSERT (! iot_typecode_equal (tc1, tc4))
  iot_typecode_free (tc1);
  iot_typecode_free (tc2);
  iot_typecode_free (tc3);
  iot_typecode_free (tc4);
}

static void test_data_type_typecode (void)
{
  static uint32_t vals [4] = { 1111111, 2222222, 3333333, 4444444 };
  iot_data_t * data;
  iot_typecode_t * tc;
  const iot_typecode_t * et;

  data = iot_data_alloc_i8 (-4);
  tc = iot_data_typecode (data);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_matches (data, tc))
  iot_data_free (data);
  iot_typecode_free (tc);

  data = iot_data_alloc_array (vals, sizeof (vals) / sizeof (uint32_t), IOT_DATA_UINT32, IOT_DATA_REF);
  tc = iot_data_typecode (data);
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_typecode_type (et) == IOT_DATA_UINT32)
  CU_ASSERT (iot_data_matches (data, tc))
  iot_data_free (data);
  iot_typecode_free (tc);

  data = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (data, iot_data_alloc_string ("One", IOT_DATA_REF), iot_data_alloc_ui16 (1));
  iot_data_map_add (data, iot_data_alloc_string ("Two", IOT_DATA_REF), iot_data_alloc_ui16 (2));
  tc = iot_data_typecode (data);
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_MAP)
  CU_ASSERT (iot_typecode_type (et) == IOT_DATA_UINT16)
  CU_ASSERT (iot_typecode_key_type (tc) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_matches (data, tc))
  iot_data_free (data);
  iot_typecode_free (tc);

  data = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_map_add (data, iot_data_alloc_string ("One", IOT_DATA_REF), iot_data_alloc_ui16 (1));
  iot_data_map_add (data, iot_data_alloc_string ("Two", IOT_DATA_REF), iot_data_alloc_i16 (2));
  tc = iot_data_typecode (data);
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_MAP)
  CU_ASSERT (et == NULL)
  CU_ASSERT (iot_typecode_key_type (tc) == IOT_DATA_STRING)
  CU_ASSERT (iot_data_matches (data, tc))
  iot_data_free (data);
  iot_typecode_free (tc);

  data = iot_data_alloc_vector (2u);
  iot_data_vector_add (data, 0, iot_data_alloc_i32 (1));
  iot_data_vector_add (data, 1, iot_data_alloc_i32 (2));
  tc = iot_data_typecode (data);
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_VECTOR)
  CU_ASSERT (iot_typecode_type (et) == IOT_DATA_INT32)
  CU_ASSERT (iot_data_matches (data, tc))
  iot_data_free (data);
  iot_typecode_free (tc);

  data = iot_data_alloc_vector (2u);
  iot_data_vector_add (data, 0, iot_data_alloc_i16 (1));
  iot_data_vector_add (data, 1, iot_data_alloc_i32 (2));
  tc = iot_data_typecode (data);
  et = iot_typecode_element_type (tc);
  CU_ASSERT (iot_typecode_type (tc) == IOT_DATA_VECTOR)
  CU_ASSERT (et == NULL)
  CU_ASSERT (iot_data_matches (data, tc))
  iot_data_free (data);
  iot_typecode_free (tc);
}

static void test_data_map_perf (void)
{
  char * keys [10]= { "common_one", "common_two", "common_three", "common_four", "common_five", "common_six", "common_seven", "common_eight", "common_nine", "common_ten" };
  char * lookup_keys [10]= { "common_one", "common_two", "common_three", "common_four", "common_five", "common_six", "common_seven", "common_eight", "common_nine", "common_ten" }; // To avoid key address equality check
  uint32_t i, j;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  for (i = 0; i < 10; i++)
  {
    iot_data_string_map_add (map, keys[i], iot_data_alloc_ui32 (i));
  }
  uint64_t t1 = iot_time_msecs ();
  for (j = 0; j < 100000; j++)
  {
    for (i = 0; i < 10; i++)
    {
      iot_data_string_map_get (map, lookup_keys[i]);
    }
  }
  uint64_t t2 = iot_time_msecs ();
  // printf ("String map[10] a million lookups in %" PRIu64 " milliseconds\n", t2 - t1);
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

void cunit_data_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data", suite_init, suite_clean);

  CU_add_test (suite, "data_types", test_data_types);
  CU_add_test (suite, "data_array_key", test_data_array_key);
  CU_add_test (suite, "data_array_iter_next", test_data_array_iter_next);
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
  CU_add_test (suite, "data_map_size", test_map_size);
  CU_add_test (suite, "data_map_iter_replace", test_data_map_iter_replace);
  CU_add_test (suite, "data_map_remove", test_data_map_remove);
  CU_add_test (suite, "data_string_vector", test_data_string_vector);
  CU_add_test (suite, "data_to_json", test_data_to_json);
  CU_add_test (suite, "data_from_json", test_data_from_json);
  CU_add_test (suite, "data_address", test_data_address);
  CU_add_test (suite, "data_name_type", test_data_name_type);
  CU_add_test (suite, "data_from_string", test_data_from_string);
  CU_add_test (suite, "data_from_strings", test_data_from_strings);
  CU_add_test (suite, "data_from_base64", test_data_from_base64);
  CU_add_test (suite, "data_map_base64_to_array", test_data_map_base64_to_array);
  CU_add_test (suite, "data_increment", test_data_increment);
  CU_add_test (suite, "data_decrement", test_data_decrement);
  CU_add_test (suite, "data_vector_iter_replace", test_data_vector_iter_replace);
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
  CU_add_test (suite, "data_vector_resize", test_data_vector_resize);
  CU_add_test (suite, "data_vector_find", test_data_vector_find);
  CU_add_test (suite, "data_copy_map_base64_to_array", test_data_copy_map_base64_to_array);
  CU_add_test (suite, "data_check_equal_nested_vector", test_data_equal_nested_vector);
  CU_add_test (suite, "data_check_unequal_nested_vector", test_data_unequal_nested_vector);
  CU_add_test (suite, "data_check_equal_vector_map", test_data_equal_vector_map);
  CU_add_test (suite, "data_check_unequal_vector_map", test_data_unequal_vector_map);
  CU_add_test (suite, "data_metadata", test_data_metadata);
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
  CU_add_test (suite, "data_type_typecode", test_data_type_typecode);
  CU_add_test (suite, "data_map_perf", test_data_map_perf);
  CU_add_test (suite, "data_int_map", test_data_int_map);
  CU_add_test (suite, "data_add_ref", test_data_add_ref);
#ifdef IOT_HAS_XML
  CU_add_test (suite, "test_data_from_xml", test_data_from_xml);
#endif
}
