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
  data = iot_data_alloc_blob (blob, 4, IOT_DATA_REF);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_blob (calloc (1, sizeof (blob)), 4, IOT_DATA_TAKE);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  iot_data_free (data);
  data = iot_data_alloc_blob (blob, 4, IOT_DATA_COPY);
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
}

static void test_data_blob_key (void)
{
  uint8_t data1 [4] = { 0, 1, 2 ,3 };
  uint8_t data2 [4] = { 0, 1, 2 ,4 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_ARRAY);
  CU_ASSERT (iot_data_map_key_type (map) == IOT_DATA_ARRAY)
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
  uint8_t data [4] = { 0, 1, 2 , 3 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_t * key = iot_data_alloc_string ("UInt32", IOT_DATA_REF);
  iot_data_t * blob = iot_data_alloc_blob (data, sizeof (data), IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  val = iot_data_alloc_string ("Lilith", IOT_DATA_REF);
  key = iot_data_alloc_string ("Name", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("Data", IOT_DATA_REF);
  iot_data_map_add (map, key, blob);
  key = iot_data_alloc_string ("Escaped", IOT_DATA_REF);
  val = iot_data_alloc_string ("abc\t\n123\x0b\x1fxyz", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  char * json = iot_data_to_json (map, false);
  CU_ASSERT (json != NULL)
  CU_ASSERT (strcmp (json, "{\"UInt32\":1,\"Name\":\"Lilith\",\"Data\":\"AAECAw==\",\"Escaped\":\"abc\\t\\n123\\u000b\\u001fxyz\"}") == 0)
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
  CU_ASSERT (iot_data_name_type ("dummy") == (iot_data_type_t) -1)
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
  data = iot_data_alloc_blob_from_base64 ("SGVsbG8gV29ybGQhCg==");
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  CU_ASSERT (iot_data_array_type (data) == IOT_DATA_UINT8)
  bytes = iot_data_blob (data, &len);
  CU_ASSERT (len == strlen (greeting))
  CU_ASSERT (strncmp ((char *) bytes, greeting, len) == 0)
  iot_data_free (data);
}

static void test_data_map_base64_to_blob (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * key = iot_data_alloc_string ("key1", IOT_DATA_REF);
  iot_data_t * val = iot_data_alloc_string ("SGVsbG8gV29ybGQhCg==", IOT_DATA_REF);
  const iot_data_t * data;
  const uint8_t * bytes;
  uint32_t len;
  iot_data_map_add (map, key, val);
  CU_ASSERT (iot_data_map_base64_to_blob (map, key))
  data = iot_data_map_get (map, key);
  CU_ASSERT (data != NULL)
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY)
  bytes = iot_data_blob (data, &len);
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

  iot_data_free (data1);
  iot_data_free (data2);
}

static void test_data_equal_vector_ui8 (void)
{
  int vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);

  while (vector_index < 5)
  {
    iot_data_vector_add (vector1, vector_index, iot_data_alloc_ui8(vector_index));
    iot_data_vector_add (vector2, vector_index, iot_data_alloc_ui8(vector_index));
    vector_index++;
  }

  CU_ASSERT (iot_data_equal (vector1, vector2))
  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_equal_vector_ui8_refcount (void)
{
  int vector_index = 0;
  iot_data_t *vector1 = iot_data_alloc_vector (5);
  iot_data_t *vector2 = iot_data_alloc_vector (5);

  while (vector_index < 5)
  {
    iot_data_t *value = iot_data_alloc_ui8(vector_index);

    iot_data_vector_add (vector1, vector_index, value);
    iot_data_add_ref (value);

    iot_data_vector_add (vector2, vector_index, value);
    vector_index++;
  }

  CU_ASSERT (iot_data_equal (vector1, vector2))
  iot_data_free (vector1);
  iot_data_free (vector2);
}

static void test_data_unequal_vector_ui8 (void)
{
  int vector_index = 0;
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

static void test_data_equal_blob (void)
{
  uint8_t data [4] = { 0, 1, 2, 3 };
  iot_data_t * blob1 = iot_data_alloc_blob (data, sizeof (data), IOT_DATA_REF);
  iot_data_t * blob2 = iot_data_alloc_blob (data, sizeof (data), IOT_DATA_REF);

  CU_ASSERT (iot_data_equal (blob1, blob2))
  iot_data_free (blob1);
  iot_data_free (blob2);
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

  /* update the value to the same key */
  val = iot_data_alloc_ui32 (2u);
  iot_data_map_add (map, key, val);
  CU_ASSERT (iot_data_map_size (map) == 1)

  iot_data_string_map_add (map, "element2", iot_data_alloc_string ("data", IOT_DATA_REF));
  CU_ASSERT (iot_data_map_size (map) == 2)

  iot_data_free (map);
}

void cunit_data_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data", suite_init, suite_clean);

  CU_add_test (suite, "data_types", test_data_types);
  CU_add_test (suite, "data_blob_key", test_data_blob_key);
  CU_add_test (suite, "data_string_vector", test_data_string_vector);
  CU_add_test (suite, "data_to_json", test_data_to_json);
  CU_add_test (suite, "data_from_json", test_data_from_json);
  CU_add_test (suite, "data_address", test_data_address);
  CU_add_test (suite, "data_name_type", test_data_name_type);
  CU_add_test (suite, "data_from_string", test_data_from_string);
  CU_add_test (suite, "data_from_strings", test_data_from_strings);
  CU_add_test (suite, "data_from_base64", test_data_from_base64);
  CU_add_test (suite, "data_map_base64_to_blob", test_data_map_base64_to_blob);
  CU_add_test (suite, "data_increment", test_data_increment);
  CU_add_test (suite, "data_decrement", test_data_decrement);
  CU_add_test (suite, "data_map_size", test_map_size);
  CU_add_test (suite, "data_check_equal_int8", test_data_equal_int8);
  CU_add_test (suite, "data_check_equal_uint16", test_data_equal_uint16);
  CU_add_test (suite, "data_check_equal_float32", test_data_equal_float32);
  CU_add_test (suite, "data_check_equal_vector_ui8", test_data_equal_vector_ui8);
  CU_add_test (suite, "data_check_equal_vector_ui8_refcount", test_data_equal_vector_ui8_refcount);
  CU_add_test (suite, "data_check_unequal_vector_ui8", test_data_unequal_vector_ui8);
  CU_add_test (suite, "data_check_equal_vector_string", test_data_equal_vector_string);
  CU_add_test (suite, "data_check_equal_blob", test_data_equal_blob);
  CU_add_test (suite, "data_check_equal_map", test_data_equal_map);
  CU_add_test (suite, "data_check_equal_map_refcount", test_data_equal_map_refcount);
  CU_add_test (suite, "data_check_unequal_map_size", test_data_unequal_map_size);
  CU_add_test (suite, "data_check_unequal_key_map", test_data_unequal_key_map);
  CU_add_test (suite, "data_check_unequal_value_map", test_data_unequal_value_map);
  CU_add_test (suite, "data_check_equal_nested_vector", test_data_equal_nested_vector);
  CU_add_test (suite, "data_check_unequal_nested_vector", test_data_unequal_nested_vector);
  CU_add_test (suite, "data_check_equal_vector_map", test_data_equal_vector_map);
  CU_add_test (suite, "data_check_unequal_vector_map", test_data_unequal_vector_map);
}
