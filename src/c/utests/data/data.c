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
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int8") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT8);
  iot_data_free (data);
  data = iot_data_alloc_ui8 (1u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt8") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT8);
  iot_data_free (data);
  data = iot_data_alloc_i16 (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int16") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT16);
  iot_data_free (data);
  data = iot_data_alloc_ui16 (2u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt16") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT16);
  iot_data_free (data);
  data = iot_data_alloc_i32 (3);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int32") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT32);
  iot_data_free (data);
  data = iot_data_alloc_ui32 (3u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt32") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT32);
  iot_data_free (data);
  data = iot_data_alloc_i64 (4);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Int64") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_INT64);
  iot_data_free (data);
  data = iot_data_alloc_ui64 (4u);
  CU_ASSERT (strcmp (iot_data_type_name (data), "UInt64") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_UINT64);
  iot_data_free (data);
  data = iot_data_alloc_f32 (5.0);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Float32") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT32);
  iot_data_free (data);
  data = iot_data_alloc_f64 (6.0);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Float64") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_FLOAT64);
  iot_data_free (data);
  data = iot_data_alloc_bool (true);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Bool") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BOOL);
  iot_data_free (data);
  data = iot_data_alloc_string ("Hello", false);
  CU_ASSERT (strcmp (iot_data_type_name (data), "String") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_STRING);
  iot_data_free (data);
  data = iot_data_alloc_blob (blob, 4, false);
  CU_ASSERT (strcmp (iot_data_type_name (data), "BLOB") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_BLOB);
  iot_data_free (data);
  data = iot_data_alloc_map (IOT_DATA_UINT32);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Map") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_MAP);
  CU_ASSERT (iot_data_map_key_type (data) == IOT_DATA_UINT32);
  iot_data_free (data);
  data = iot_data_alloc_array (2);
  CU_ASSERT (strcmp (iot_data_type_name (data), "Array") == 0);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_ARRAY);
  iot_data_free (data);
}

static void test_data_blob_key (void)
{
  uint8_t data [4] = { 0, 1, 2 ,3 };
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_BLOB);
  CU_ASSERT (iot_data_map_key_type (map) == IOT_DATA_BLOB);
  iot_data_t * blob = iot_data_alloc_blob (data, 4, false);
  iot_data_t * val = iot_data_alloc_ui32 (66u);
  iot_data_map_add (map, blob, val);
  const iot_data_t * ret = iot_data_map_get (map, blob);
  CU_ASSERT (ret == val);
  iot_data_free (map);
}

void cunit_data_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data", suite_init, suite_clean);
  CU_add_test (suite, "data_types", test_data_types);
  CU_add_test (suite, "data_blob", test_data_blob_key);
}
