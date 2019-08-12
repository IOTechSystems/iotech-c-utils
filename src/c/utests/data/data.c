#include "data.h"
#include "CUnit.h"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
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
    iot_data_add_ref (map);
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

static void test_data_address (void)
{
  uint32_t * ui32ptr;
  char ** strptr;
  uint8_t buff [4] = { 0, 1, 2 ,3 };
  iot_data_t * data = iot_data_alloc_blob (buff, sizeof (buff), IOT_DATA_REF);
  CU_ASSERT (iot_data_address (data) == buff)
  iot_data_free (data);
  data = iot_data_alloc_ui32 (5u);
  ui32ptr = (uint32_t*) iot_data_address (data);
  *ui32ptr = 6u;
  CU_ASSERT (iot_data_ui32 (data) == 6u);
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
  CU_ASSERT (iot_data_name_type ("blob") == IOT_DATA_BLOB)
  CU_ASSERT (iot_data_name_type ("map") == IOT_DATA_MAP)
  CU_ASSERT (iot_data_name_type ("array") == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_name_type ("dummy") == -1)
}

static void test_data_from_string (void)
{
  iot_data_t * data;
  data = iot_data_alloc_from_string (IOT_DATA_INT8, "-6");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8)
  CU_ASSERT (iot_data_i8 (data) == -6)
  iot_data_free (data);
  data = iot_data_alloc_from_string (IOT_DATA_UINT8, "4");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8);
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
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32);
  CU_ASSERT (iot_data_i32 (data) == -2222222);
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
  data = iot_data_alloc_from_string (IOT_DATA_BLOB, "XXX");
  CU_ASSERT (data == NULL)
  data = iot_data_alloc_from_string (IOT_DATA_MAP, "XXX");
  CU_ASSERT (data == NULL)
  data = iot_data_alloc_from_string (IOT_DATA_ARRAY, "XXX");
  CU_ASSERT (data == NULL)
}

void cunit_data_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data", suite_init, suite_clean);
  CU_add_test (suite, "data_types", test_data_types);
  CU_add_test (suite, "data_blob_key", test_data_blob_key);
  CU_add_test (suite, "data_string_array", test_data_string_array);
  CU_add_test (suite, "data_to_json", test_data_to_json);
  CU_add_test (suite, "data_from_json", test_data_from_json);
  CU_add_test (suite, "data_address", test_data_address);
  CU_add_test (suite, "data_name_type", test_data_name_type);
  CU_add_test (suite, "data_from_string", test_data_from_string);
}
