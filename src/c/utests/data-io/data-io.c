/*
 * Copyright (c) 2023
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/logger.h"
#include "iot/config.h"
#include "iot/data.h"
#include "data-io.h"
#include "CUnit.h"
#include <float.h>
#include "limits.h"

#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

static int suite_init (void)
{
  iot_logger_start (iot_logger_default ());
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static uint8_t test_sample_array [4] = { 0, 1, 2, 3 };

static iot_data_t *test_sample_map1 (void)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * val = iot_data_alloc_ui32 (1u);
  iot_data_t * key = iot_data_alloc_string ("UInt32", IOT_DATA_REF);
  iot_data_t * array = iot_data_alloc_array (test_sample_array, sizeof (test_sample_array), IOT_DATA_UINT8, IOT_DATA_REF);
  iot_data_t * binary = iot_data_alloc_binary (test_sample_array, sizeof (test_sample_array), IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  val = iot_data_alloc_string ("Lilith", IOT_DATA_REF);
  key = iot_data_alloc_string ("Name", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("Array", IOT_DATA_REF);
  iot_data_map_add (map, key, array);
  key = iot_data_alloc_string ("Binary", IOT_DATA_REF);
  iot_data_map_add (map, key, binary);
  key = iot_data_alloc_string ("Escaped", IOT_DATA_REF);
  val = iot_data_alloc_string ("abc\t\n123\x0b\x1fxyz", IOT_DATA_REF);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("Boolean", IOT_DATA_REF);
  val = iot_data_alloc_bool (true);
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("NULL", IOT_DATA_REF);
  val = iot_data_alloc_null ();
  iot_data_map_add (map, key, val);
  return map;
}

static iot_data_t *test_sample_map2 (void)
{
  /* non-string key type */
  iot_data_t *map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t *val = iot_data_alloc_string ("Cthulhu", IOT_DATA_REF);
  iot_data_t *key = iot_data_alloc_ui32 (1u);
  iot_data_map_add (map, key, val);
  val = iot_data_alloc_string ("Rules", IOT_DATA_REF);
  key = iot_data_alloc_ui32 (2u);
  iot_data_map_add (map, key, val);
  return map;
}

static void test_data_to_json (void)
{
  char * json;
  iot_data_t *val;
  iot_data_t *map = test_sample_map1 ();
  json = iot_data_to_json (map);
  CU_ASSERT (json != NULL)
  if (json)
  {
    // printf ("JSON: %s\n", json);
    CU_ASSERT (strcmp (json, "{\"Array\":[0,1,2,3],\"Binary\":\"AAECAw==\",\"Boolean\":true,\"Escaped\":\"abc\\t\\n123\\u000b\\u001fxyz\",\"NULL\":null,\"Name\":\"Lilith\",\"UInt32\":1}") == 0)
  }
  iot_data_free (map);

  map = iot_data_from_json_with_ordering (json, true);
  CU_ASSERT (map != NULL)
  free (json);
  iot_data_free (map);

  map = test_sample_map2 ();
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

  char buff[32];
  val = iot_data_alloc_bool (true);
  json = iot_data_to_json_with_buffer (val, buff, sizeof (buff));
  CU_ASSERT (json == buff)
  CU_ASSERT (strcmp (buff, "true") == 0)
  iot_data_free (val);

  char * dbuff = malloc (10);
  val = iot_data_alloc_f64 (DBL_MAX);
  json = iot_data_to_json_with_buffer (val, dbuff, 10);
  CU_ASSERT (json != buff)
  iot_data_free (val);
  free (json);
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
    "\"Unicode\":\"\\u0003HELLO\\u0006HI\","
    "\"Unicode2\":\"\\u003cAngled\\u003e\""
  "}";

static void test_data_from_json (void)
{
  static const char * config2 =
  "{"
    "\"Interval\":10000,"
    "\"UINT64_MAX\":18446744073709551615,"
    "\"Scheduler\":\"scheds\""
  "}";
  bool bval = false;
  const char * sval = NULL;
  double dval = 1.0;
  int64_t ival64 = 0;
  uint64_t uival64 = 0;
  int32_t ival32 = 0;
  uint32_t uival32 = 0;
  bool found;
  const iot_data_t * data;
  iot_data_t * key;

  iot_data_t * map = iot_data_from_json (test_config);
  CU_ASSERT (map != NULL)

  found = iot_config_bool (map, "Boolean", &bval, NULL);
  CU_ASSERT (found)
  CU_ASSERT (bval)
  found = iot_config_bool (map, "Bolean", &bval, NULL);
  CU_ASSERT (! found)
  key = iot_data_alloc_string ("Boolean", IOT_DATA_REF);
  bval = iot_data_map_get_bool (map, key, false);
  CU_ASSERT (bval)
  iot_data_free (key);

  sval = iot_config_string (map, "Scheduler", false,NULL);
  CU_ASSERT (sval != NULL)
  if (sval) CU_ASSERT (strcmp (sval, "scheduler") == 0)
  sval = iot_config_string (map, "Sched", false, NULL);
  CU_ASSERT (sval == NULL)

  sval = iot_config_string (map, "Escaped", false, NULL);
  CU_ASSERT (sval != NULL)
  if (sval) CU_ASSERT (strcmp (sval, "Double \" Quote") == 0)

  sval = iot_config_string_default (map, "Scheduler", "Hello", false);
  CU_ASSERT (sval != NULL)
  if (sval) CU_ASSERT (strcmp (sval, "scheduler") == 0)
  sval = iot_config_string_default (map, "Nope", "Hello", true);
  CU_ASSERT (sval != NULL)
  if (sval) CU_ASSERT (strcmp (sval, "Hello") == 0)
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

  found = iot_config_ui64 (map, "Interval", &uival64, NULL);
  CU_ASSERT (found)
  CU_ASSERT (uival64 == 1000)
  found = iot_config_i64 (map, "Int", &ival64, NULL);
  CU_ASSERT (! found)
  found = iot_config_i32 (map, "Interval", &ival32, NULL);
  CU_ASSERT (found)
  CU_ASSERT (ival32 == 1000)
  found = iot_config_ui32 (map, "Interval", &uival32, NULL);
  CU_ASSERT (found)
  CU_ASSERT (uival32 == 1000)
  key = iot_data_alloc_string ("Interval", IOT_DATA_REF);
  ival64 = iot_data_map_get_i64 (map, key, 666);
  CU_ASSERT (ival64 == 1000)
  iot_data_free (key);

  dval = 7.7;
  dval = iot_data_string_map_get_f64 (map, "DB", 1.0);
  CU_ASSERT (dval < 1.0)
  key = iot_data_alloc_string ("DB", IOT_DATA_REF);
  dval = iot_data_map_get_f64 (map, key, 1.0);
  CU_ASSERT (dval < 1.0)
  iot_data_free (key);

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
  key = iot_data_alloc_string ("Numbers", IOT_DATA_REF);
  map2 = iot_data_map_get_map (map, key);
  CU_ASSERT (map2 != NULL)
  iot_data_free (key);

  const iot_data_t * vec = iot_data_string_map_get_vector (map, "Vector");
  CU_ASSERT (vec != NULL)
  key = iot_data_alloc_string ("Vector", IOT_DATA_REF);
  vec = iot_data_map_get_vector (map, key);
  CU_ASSERT (vec!= NULL)
  iot_data_free (key);

  const char * ustr = iot_data_string_map_get_string (map, "Unicode");
  CU_ASSERT (strcmp (ustr, "\003HELLO\006HI") == 0)
  const char *u2str = iot_data_string_map_get_string (map, "Unicode2");
  CU_ASSERT (strcmp (u2str, "<Angled>") == 0)
  iot_data_free (map);

  iot_data_t * cache = iot_data_alloc_map (IOT_DATA_STRING);
  map = iot_data_from_json_with_cache (test_config, false, cache);
  iot_data_t * map3 = iot_data_from_json_with_cache (config2, false, cache);
  iot_data_map_iter_t iter;
  iot_data_map_iter (cache, &iter);
  printf ("Keys: ");
  while (iot_data_map_iter_next (&iter))
  {
    printf ("%s ", iot_data_map_iter_string_key (&iter));
  }
  uival64 = iot_data_string_map_get_ui64 (map3, "UINT64_MAX", 0u);
  CU_ASSERT (uival64 == UINT64_MAX)
  uival64 = 0u;
  found = iot_config_ui64 (map3, "UINT64_MAX", &uival64, NULL);
  CU_ASSERT (found)
  CU_ASSERT (uival64 == UINT64_MAX)
  iot_data_free (map);
  iot_data_free (map3);
  iot_data_free (cache);

  iot_data_t * nd = iot_data_from_json (NULL);
  CU_ASSERT (nd != NULL)
  CU_ASSERT (iot_data_type (nd) == IOT_DATA_NULL)
  iot_data_free (nd);
  nd = iot_data_from_json ("");
  CU_ASSERT (nd != NULL)
  CU_ASSERT (iot_data_type (nd) == IOT_DATA_NULL)
  iot_data_free (nd);
}

static void test_data_from_json2 (void)
{
  static const char * config =
  "{"
    "\"client\":\"client1\","
    "\"request_id\":\"138094\","
    "\"op\":\"schedule:list\","
    "\"0\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"100\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"200\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"300\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"400\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"500\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"600\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"700\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"800\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"900\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1000\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1100\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1200\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1300\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1400\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1500\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1600\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1700\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1800\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"1900\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"2000\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"2100\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"2200\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"2300\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\","
    "\"2400\":\"t\\\\est\\/tube\\/test\\/tub\\\\e\""
  "}";

  iot_data_t * map = iot_data_from_json (config);
  CU_ASSERT (map != NULL)
  iot_data_free (map);
}

static void test_data_from_json3 (void)
{
  static const char * original_json = "{\"int64_max\":9223372036854775807,\"int64_min\":-9223372036854775808}";
  iot_data_t * iot_map = iot_data_from_json (original_json);
  CU_ASSERT (iot_map != NULL)
  char *new_json = iot_data_to_json (iot_map);
  CU_ASSERT (new_json != NULL)
  CU_ASSERT (strcmp (new_json, original_json) == 0)
  iot_data_free (iot_map);
  free (new_json);
}

#ifdef IOT_HAS_XML
static void test_data_from_xml (void)
{
  iot_data_t * xml;
  char * json;
  const char * test_xml = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
  "<busmaster xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" busId=\"main_bus\">\n"
  "  <deviceService name=\"virtual_device_service\" library=\"libxrt-virtual-device-service.so\" factory=\"xrt_virtual_device_service_factory\" topic=\"virtual_device_service/data\">\n"
  "    <device name=\"Random-Integer-Device\" profile=\"Random-Integer-Device\">\n"
  "      <resource name=\"RandomValue_Int8\" schedule=\"500000000\" />\n"
  "      <protocol name=\"Other\">\n"
  "        <protocolAttribute name=\"Address\" value=\"device-virtual-int-01\" />Any old rubbish\n"
  "      </protocol>\n"
  "    </device>\n"
  "  </deviceService>\n"
  "  <fubar>Some text!</fubar>\n"
  "  <container threads=\"4\">\n"
  "    <logging enable=\"true\" filename=\"/dev/null\" />\n"
  "  </container>\n"
  "</busmaster>";
  const char * expected = "{\"attributes\":{\"busId\":\"main_bus\",\"xmlns:xsd\":\"http://www.w3.org/2001/XMLSchema\",\"xmlns:xsi\":\"http://www.w3.org/2001/XMLSchema-instance\"},\"children\":[{\"attributes\":{\"factory\":\"xrt_virtual_device_service_factory\",\"library\":\"libxrt-virtual-device-service.so\",\"name\":\"virtual_device_service\",\"topic\":\"virtual_device_service/data\"},\"children\":[{\"attributes\":{\"name\":\"Random-Integer-Device\",\"profile\":\"Random-Integer-Device\"},\"children\":[{\"attributes\":{\"name\":\"RandomValue_Int8\",\"schedule\":\"500000000\"},\"name\":\"resource\"},{\"attributes\":{\"name\":\"Other\"},\"children\":[{\"attributes\":{\"name\":\"Address\",\"value\":\"device-virtual-int-01\"},\"name\":\"protocolAttribute\"}],\"content\":\"Any old rubbish\\n      \",\"name\":\"protocol\"}],\"content\":\"\\n    \",\"name\":\"device\"}],\"content\":\"\\n  \",\"name\":\"deviceService\"},{\"attributes\":{},\"content\":\"Some text!\",\"name\":\"fubar\"},{\"attributes\":{\"threads\":\"4\"},\"children\":[{\"attributes\":{\"enable\":\"true\",\"filename\":\"/dev/null\"},\"name\":\"logging\"}],\"content\":\"\\n  \",\"name\":\"container\"}],\"content\":\"\\n\",\"name\":\"busmaster\"}";

  xml = iot_data_from_xml (test_xml);
  CU_ASSERT (xml != NULL)
  json = iot_data_to_json (xml);
  CU_ASSERT (json != NULL)
//  printf ("\nXML: %s\n", json);
//  printf ("\nEXP: %s\n", expected);
  if (json) CU_ASSERT (strcmp (json, expected) == 0)
  free (json);
  iot_data_free (xml);
}
#endif

#ifdef IOT_HAS_CBOR
static void test_data_to_cbor (void)
{
  iot_data_t *map = test_sample_map1 ();
  iot_data_t *cbor = iot_data_to_cbor (map);
  CU_ASSERT (cbor != NULL)
  if (cbor)
  {
    // printf ("CBOR: %s\n", iot_data_to_json (cbor));
    // printf ("CBOR hash: %u\n", iot_data_hash (cbor));
    CU_ASSERT (iot_data_hash (cbor) == 2529945693U)
  }
  iot_data_free (cbor);
  iot_data_free (map);

  map = test_sample_map2 ();
  cbor = iot_data_to_cbor (map);
  CU_ASSERT (cbor != NULL)
  if (cbor)
  {
    // printf ("CBOR: %s\n", iot_data_to_json (cbor));
    // printf ("CBOR hash: %u\n", iot_data_hash (cbor));
    CU_ASSERT (iot_data_hash (cbor) == 2695702783U)
  }
  iot_data_free (cbor);
  iot_data_free (map);

  // Test different int sizes - CBOR uses variable length encoding

  map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "1", iot_data_alloc_ui64 (12));
  iot_data_string_map_add (map, "2", iot_data_alloc_ui64 (123));
  iot_data_string_map_add (map, "3", iot_data_alloc_ui64 (1234));
  iot_data_string_map_add (map, "4", iot_data_alloc_ui64 (123456));
  iot_data_string_map_add (map, "5", iot_data_alloc_ui64 (12345678901));
  iot_data_string_map_add (map, "6", iot_data_alloc_i64 (12));
  iot_data_string_map_add (map, "7", iot_data_alloc_i64 (123));
  iot_data_string_map_add (map, "8", iot_data_alloc_i64 (1234));
  iot_data_string_map_add (map, "9", iot_data_alloc_i64 (123456));
  iot_data_string_map_add (map, "10", iot_data_alloc_i64 (12345678901));
  iot_data_string_map_add (map, "11", iot_data_alloc_i64 (-12));
  iot_data_string_map_add (map, "12", iot_data_alloc_i64 (-123));
  iot_data_string_map_add (map, "13", iot_data_alloc_i64 (-1234));
  iot_data_string_map_add (map, "14", iot_data_alloc_i64 (-123456));
  iot_data_string_map_add (map, "15", iot_data_alloc_i64 (-12345678901));
  cbor = iot_data_to_cbor (map);
  CU_ASSERT (cbor != NULL)
  if (cbor)
  {
    // printf ("CBOR: %s\n", iot_data_to_json (cbor));
    // printf ("CBOR hash: %u\n", iot_data_hash (cbor));
    CU_ASSERT (iot_data_hash (cbor) == 2072572302U)
  }
  iot_data_free (cbor);
  iot_data_free (map);

  //neg
  unsigned char negative_i16_expected_data[] = {0x39, 0x01, 0xf4 };
  iot_data_t *neg_i16 = iot_data_alloc_i16 (-501);
  cbor = iot_data_to_cbor (neg_i16);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor)
  CU_ASSERT_TRUE (ARRAY_SIZE(negative_i16_expected_data) == iot_data_array_length(cbor))
  CU_ASSERT_TRUE (memcmp (negative_i16_expected_data, iot_data_address (cbor), iot_data_array_length (cbor)) == 0);
  iot_data_free (neg_i16);
  iot_data_free (cbor);
}


static void test_cbor_to_data (void)
{
  iot_data_t *from_cbor = NULL;
  iot_data_t *cbor = NULL;

  //positive very short int
  iot_data_t *uint8_value = iot_data_alloc_ui8 (UINT8_MAX);
  cbor = iot_data_to_cbor (uint8_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (uint8_value, from_cbor))
  iot_data_free (uint8_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //negative very short int
  iot_data_t *int8_value = iot_data_alloc_i8 (INT8_MIN);
  cbor = iot_data_to_cbor (int8_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (int8_value, from_cbor));
  iot_data_free (int8_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //positive short int
  iot_data_t *uint16_value = iot_data_alloc_ui16 (UINT16_MAX);
  cbor = iot_data_to_cbor (uint16_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (uint16_value, from_cbor));
  iot_data_free (uint16_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //negative short int
  iot_data_t *int16_value = iot_data_alloc_i16 (INT16_MIN);
  cbor = iot_data_to_cbor (int16_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (int16_value, from_cbor));
  iot_data_free (int16_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //positive int
  iot_data_t *uint32_value = iot_data_alloc_ui32(UINT32_MAX);
  cbor = iot_data_to_cbor (uint32_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (uint32_value, from_cbor));
  iot_data_free (uint32_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //negative int
  iot_data_t *int32_value = iot_data_alloc_i32 (INT32_MIN);
  cbor = iot_data_to_cbor (int32_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (int32_value, from_cbor))
  iot_data_free (int32_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  unsigned char negative_half_int_data[] = {0x39, 0x01, 0xf4, 0xFF};
  iot_data_t *expected_val = iot_data_alloc_i16 (-501);
  from_cbor = iot_data_from_cbor (negative_half_int_data, ARRAY_SIZE(negative_half_int_data));
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  CU_ASSERT_TRUE (iot_data_equal_value (from_cbor, expected_val))
  iot_data_free (from_cbor);
  iot_data_free (expected_val);

  //positive long int
  iot_data_t *uint64_value = iot_data_alloc_ui64(UINT64_MAX);
  cbor = iot_data_to_cbor (uint64_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (uint64_value, from_cbor));
  iot_data_free (uint64_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //negative int
  iot_data_t *int64_value = iot_data_alloc_i64 (INT64_MIN);
  cbor = iot_data_to_cbor (int64_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal_value (int64_value, from_cbor));
  iot_data_free (int64_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //float
  iot_data_t *float_value = iot_data_alloc_f32 (123.456f);
  cbor = iot_data_to_cbor (float_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal (float_value, from_cbor));
  iot_data_free (float_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //double
  iot_data_t *double_value = iot_data_alloc_f64 (1239999999999999999999.456);
  cbor = iot_data_to_cbor (double_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal (double_value, from_cbor));
  iot_data_free (double_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //bool
  iot_data_t *bool_value = iot_data_alloc_bool (true);
  cbor = iot_data_to_cbor (bool_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal (bool_value, from_cbor));
  iot_data_free (bool_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //string
  iot_data_t *string_value = iot_data_alloc_string ("test_string", IOT_DATA_REF);
  cbor = iot_data_to_cbor (string_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor);
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE(iot_data_equal (string_value, from_cbor))
  iot_data_free (string_value);
  iot_data_free (cbor);
  iot_data_free (from_cbor);

  //indefinite string
  unsigned char indef_string_data[] = {0x7F, 0x64, 't', 'e', 's', 't', 0x63 , '1', '2', '3', 0x63, '4', '5', '6', 0x64, 't', 'e', 's', 't', 0xFF};
  from_cbor = iot_data_from_cbor (indef_string_data, ARRAY_SIZE(indef_string_data));
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor);
  CU_ASSERT_TRUE (strcmp(iot_data_string (from_cbor),"test123456test") == 0)
  iot_data_free (from_cbor);

  //bytestring
  char test_data[] = "test12345test";
  iot_data_t *bytestring_value = iot_data_alloc_binary(test_data, ARRAY_SIZE(test_data), IOT_DATA_REF);
  cbor = iot_data_to_cbor (bytestring_value);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor)
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  CU_ASSERT_TRUE(iot_data_equal (bytestring_value, from_cbor))
  iot_data_free (cbor);
  iot_data_free (from_cbor);
  iot_data_free (bytestring_value);

  //indefinite bytestring
  unsigned char binary_data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x99};
  unsigned char indef_binary_data[] = {0x5F, 0x44, 0xaa, 0xbb, 0xcc, 0xdd, 0x43, 0xee, 0xff, 0x99, 0xFF};
  from_cbor = iot_data_from_cbor (indef_binary_data, ARRAY_SIZE(indef_binary_data));
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  bytestring_value = iot_data_alloc_binary(binary_data, ARRAY_SIZE(binary_data), IOT_DATA_REF);
  CU_ASSERT_TRUE(iot_data_equal (bytestring_value, from_cbor))
  iot_data_free (from_cbor);
  iot_data_free (bytestring_value);

  //array
  iot_data_t *iot_array = iot_data_alloc_vector (4);
  iot_data_vector_add (iot_array, 0, iot_data_alloc_string ("string_value", IOT_DATA_REF));
  iot_data_vector_add (iot_array, 1, iot_data_alloc_i8 (-42));
  iot_data_vector_add (iot_array, 2, iot_data_alloc_ui32 (UINT32_MAX));
  iot_data_vector_add (iot_array, 3, iot_data_alloc_binary(binary_data, ARRAY_SIZE(binary_data), IOT_DATA_REF));
  cbor = iot_data_to_cbor (iot_array);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor)
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  CU_ASSERT_TRUE(iot_data_equal_value (iot_array, from_cbor))
  iot_data_free (cbor);
  iot_data_free (from_cbor);
  iot_data_free (iot_array);

  //map
  iot_data_t *iot_map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (iot_map, "one", iot_data_alloc_string ("one value", IOT_DATA_REF));
  iot_data_string_map_add (iot_map, "two", iot_data_alloc_ui32 (UINT32_MAX));
  iot_data_string_map_add (iot_map, "array", iot_data_alloc_binary(binary_data, ARRAY_SIZE(binary_data), IOT_DATA_REF));
  cbor = iot_data_to_cbor (iot_map);
  CU_ASSERT_PTR_NOT_NULL_FATAL (cbor)
  from_cbor = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  CU_ASSERT_TRUE(iot_data_equal_value (iot_map, from_cbor))
  iot_data_free (cbor);
  iot_data_free (from_cbor);
  iot_data_free (iot_map);

  //tag
  /* Tag 255 << 8 + uint 2 */
  unsigned char tag_binary_data[] = {0xD9, 0xFF, 0x00, 0x02};
  expected_val = iot_data_alloc_ui8 (2);
  from_cbor = iot_data_from_cbor (tag_binary_data, ARRAY_SIZE(tag_binary_data));
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  CU_ASSERT_TRUE (iot_data_equal (from_cbor, expected_val))
  iot_data_free (from_cbor);
  iot_data_free (expected_val);
}

#endif

#ifdef IOT_HAS_YAML
static void test_data_from_yaml (void)
{
  iot_data_t * yaml;
  iot_data_t * ex;
  char * json;
  const char * test_yaml = "name: \"Example Sensor\"\n"
    "manufacturer: \"IoTechSystems\"\n"
    "labels:\n"
    "  - \"sensor\"\n\n"
    "resources:\n"
    "  -\n"
    "    name: Switch\n"
    "    get:\n"
    "    - { index: 1, operation: \"get\", object: \"Switch\", parameter: \"Switch\", property: \"value\" }\n"
    "    set:\n"
    "    - { index: 1, operation: \"set\", object: \"Switch\", parameter: \"Switch\", property: \"value\" }\n\n"
    "commands:\n"
    "  -\n"
    "    name: SensorOne\n"
    "    isVisible: true\n"
    "    get:\n"
    "        path: \"/api/v1/device/{deviceId}/SensorOne\"\n"
    "        responses:\n"
    "          -\n"
    "            code: 200\n"
    "            description: \"Get the SensorOne reading.\"\n"
    "            expectedValues: [\"SensorOne\"]\n"
    "          -\n"
    "            code: 503\n"
    "            description: \"service unavailable\"\n"
    "            expectedValues: []";
  const char * expected = "{\"commands\":[{\"get\":{\"path\":\"/api/v1/device/{deviceId}/SensorOne\",\"responses\":[{\"code\":200,\"description\":\"Get the SensorOne reading.\",\"expectedValues\":[\"SensorOne\"]},{\"code\":503,\"description\":\"service unavailable\",\"expectedValues\":[]}]},\"isVisible\":true,\"name\":\"SensorOne\"}],\"labels\":[\"sensor\"],\"manufacturer\":\"IoTechSystems\",\"name\":\"Example Sensor\",\"resources\":[{\"get\":[{\"index\":1,\"object\":\"Switch\",\"operation\":\"get\",\"parameter\":\"Switch\",\"property\":\"value\"}],\"name\":\"Switch\",\"set\":[{\"index\":1,\"object\":\"Switch\",\"operation\":\"set\",\"parameter\":\"Switch\",\"property\":\"value\"}]}]}";

  yaml = iot_data_from_yaml (test_yaml, &ex);
  CU_ASSERT (yaml != NULL)
  json = iot_data_to_json (yaml);
  CU_ASSERT (json != NULL)
  if (json) CU_ASSERT (strcmp (json, expected) == 0)
  free (json);
  iot_data_free (yaml);
}
#endif

void cunit_data_io_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("data-io", suite_init, suite_clean);

  CU_add_test (suite, "data_to_json", test_data_to_json);
  CU_add_test (suite, "data_from_json", test_data_from_json);
  CU_add_test (suite, "data_from_json2", test_data_from_json2);
  CU_add_test (suite, "data_from_json3", test_data_from_json3);
#ifdef IOT_HAS_XML
  CU_add_test (suite, "data_from_xml", test_data_from_xml);
#endif
#ifdef IOT_HAS_CBOR
  CU_add_test (suite, "data_to_cbor", test_data_to_cbor);
  CU_add_test (suite, "cbor_to_data", test_cbor_to_data);
#endif
#ifdef IOT_HAS_YAML
  CU_add_test (suite, "data_from_yaml", test_data_from_yaml);
#endif
}
