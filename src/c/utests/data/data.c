#include "data.h"
#include "CUnit.h"

static int suite_init (void)
{
  iot_data_init ();
  return 0;
}

static int suite_clean (void)
{
  iot_data_fini ();
  return 0;
}

static void test_data_types (void)
{
  uint8_t blob [4] = { 0, 1, 2 ,3 };
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
  data = iot_data_alloc_f32 (5.0);
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
  data = iot_data_alloc_blob (blob, 4, IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "BLOB") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BLOB)
  iot_data_free (data);
  data = iot_data_alloc_blob (calloc (1, sizeof (blob)), 4, IOT_DATA_TAKE);
  CU_ASSERT (strcmp (iot_data_type_name (data), "BLOB") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BLOB)
  iot_data_free (data);
  data = iot_data_alloc_blob (blob, 4, IOT_DATA_COPY);
  CU_ASSERT (strcmp (iot_data_type_name (data), "BLOB") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BLOB)
  iot_data_free (data);
  data = iot_data_alloc_map (IOT_DATA_UINT32);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Map") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_MAP)
  CU_ASSERT (iot_data_map_key_type (data) == IOT_DATA_UINT32)
  iot_data_free (data);
  data = iot_data_alloc_array (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
}

static void test_data_blob_key (void)
{
  uint8_t data1 [4] = { 0, 1, 2 ,3 };
  uint8_t data2 [4] = { 0, 1, 2 ,4 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_BLOB);
  CU_ASSERT (iot_data_map_key_type (map) == IOT_DATA_BLOB)
  iot_data_t * blob1 = iot_data_alloc_blob (data1, sizeof (data1), IOT_DATA_REF);
  iot_data_t * blob2 = iot_data_alloc_blob (data2, sizeof (data2), IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_ui32 (66u);
  iot_data_t * duffkey = iot_data_alloc_i32 (55);
  iot_data_map_add (map, blob1, val);
  const iot_data_t * ret = iot_data_map_get (map, blob1);
  CU_ASSERT (ret == val)
  ret = iot_data_map_get (map, blob2);
  CU_ASSERT (ret == NULL)
  ret = iot_data_map_get (map, duffkey);
  CU_ASSERT (ret == NULL)
  iot_data_free (blob2);
  iot_data_free (duffkey);
  iot_data_free (map);
}

static void test_data_string_array (void)
{
  const char * strs [2] = { "Test", "Tube" };
  uint32_t index = 0;
  iot_data_array_iter_t iter;
  iot_data_t * array = iot_data_alloc_array (2);
  iot_data_t * str1 = iot_data_alloc_string (strs[0], IOT_DATA_REF);
  iot_data_t * str2 = iot_data_alloc_string (strs[1], IOT_DATA_REF);
  iot_data_array_add (array, 0, str1);
  iot_data_array_add (array, 1, str2);
  iot_data_array_iter (array, &iter);
  while (iot_data_array_iter_next (&iter))
  {
    CU_ASSERT (iot_data_array_iter_index (&iter) == index)
    CU_ASSERT (iot_data_array_iter_value (&iter) != NULL)
    CU_ASSERT (iot_data_array_iter_string (&iter) == strs[index])
    index++;
  }
  CU_ASSERT (iot_data_array_size (array) == 2)
  CU_ASSERT (iot_data_array_get (array, 0) == str1)
  CU_ASSERT (iot_data_array_get (array, 1) == str2)

  int loop = 5;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_array_add (array, 0, iot_data_alloc_string ("first element", IOT_DATA_REF));
  while (loop--)
  {
    iot_data_string_map_add (map, "temp", iot_data_alloc_i32 (loop));
    iot_data_array_add (array, 1, map);
    iot_data_addref (map);
    assert (iot_data_array_get (array, 1) == map);
  }
  iot_data_free (map);
  iot_data_free (array);
}

static void test_data_to_json (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_t * key = iot_data_alloc_string ("UInt32", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  val = iot_data_alloc_string ("Lilith", IOT_DATA_REF);
  key = iot_data_alloc_string ("Name",IOT_DATA_REF );
  iot_data_map_add (map, key, val);
  char * json = iot_data_to_json (map, false);
  CU_ASSERT (json != NULL)
  printf (" %s ", json);
  free (json);
  iot_data_free (map);
}

static void test_data_from_json (void)
{
  static const char * bus_config =
  "{"
    "\"Interval\":100000,"
    "\"Scheduler\":\"scheduler\","
    "\"ThreadPool\":\"pool\","
    "\"Topics\": [{\"Topic\":\"test/tube\",\"Priority\":10,\"Retain\":true}],"
    "\"Dummy\": null"
  "}";
  iot_data_t * map = iot_data_from_json (bus_config);
  CU_ASSERT (map != NULL)
  iot_data_free (map);
}

void cunit_data_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data", suite_init, suite_clean);
  CU_add_test (suite, "data_types", test_data_types);
  CU_add_test (suite, "data_blob_key", test_data_blob_key);
  CU_add_test (suite, "data_string_array", test_data_string_array);
  CU_add_test (suite, "data_to_json", test_data_to_json);
  CU_add_test (suite, "data_from_json", test_data_from_json);
}
