/*
 * Copyright (c) 2023-2026
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
#include "iot/json.h"

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
  val = iot_data_true;
  iot_data_map_add (map, key, val);
  key = iot_data_alloc_string ("NULL", IOT_DATA_REF);
  val = iot_data_null;
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
  val = iot_data_true;
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
  const char * expected = "{\"attributes\":{\"busId\":\"main_bus\",\"xmlns:xsd\":\"http://www.w3.org/2001/XMLSchema\",\"xmlns:xsi\":\"http://www.w3.org/2001/XMLSchema-instance\"},\"children\":[{\"attributes\":{\"factory\":\"xrt_virtual_device_service_factory\",\"library\":\"libxrt-virtual-device-service.so\",\"name\":\"virtual_device_service\",\"topic\":\"virtual_device_service/data\"},\"children\":[{\"attributes\":{\"name\":\"Random-Integer-Device\",\"profile\":\"Random-Integer-Device\"},\"children\":[{\"attributes\":{\"name\":\"RandomValue_Int8\",\"schedule\":\"500000000\"},\"name\":\"resource\"},{\"attributes\":{\"name\":\"Other\"},\"children\":[{\"attributes\":{\"name\":\"Address\",\"value\":\"device-virtual-int-01\"},\"name\":\"protocolAttribute\"}],\"content\":\"\\n        Any old rubbish\\n      \",\"name\":\"protocol\"}],\"content\":\"\\n      \\n      \\n    \",\"name\":\"device\"}],\"content\":\"\\n    \\n  \",\"name\":\"deviceService\"},{\"attributes\":{},\"content\":\"Some text!\",\"name\":\"fubar\"},{\"attributes\":{\"threads\":\"4\"},\"children\":[{\"attributes\":{\"enable\":\"true\",\"filename\":\"/dev/null\"},\"name\":\"logging\"}],\"content\":\"\\n    \\n  \",\"name\":\"container\"}],\"content\":\"\\n  \\n  \\n  \\n\",\"name\":\"busmaster\"}";

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

static void test_data_from_xml_invalid (void)
{
  // Empty, truncated, mismatched and trailing content is rejected
  static const char * invalid[] =
  {
    "", "  ", "<?xml version=\"1.0\"?>", "<a>", "<a", "<a b=\"1\">text", "<a><b></b>", "<a><b/>", "<a></b>", "<a/>junk", "<a/><b/>", "<a>&bad;</a>"
  };
  for (uint32_t i = 0; i < sizeof (invalid) / sizeof (invalid[0]); i++)
  {
    iot_data_t * data = iot_data_from_xml (invalid[i]);
    CU_ASSERT (data == NULL)
    iot_data_free (data);
  }
  // Trailing whitespace and comments are allowed
  iot_data_t * data = iot_data_from_xml ("<a/>\n<!-- comment -->\n");
  CU_ASSERT (data != NULL)
  iot_data_free (data);
}

static void test_data_from_xml_content (void)
{
  // Content either side of child elements is kept
  static const struct { const char * in; const char * out; } cases[] =
  {
    { "<a>x<b>y</b>z</a>", "{\"attributes\":{},\"children\":[{\"attributes\":{},\"content\":\"y\",\"name\":\"b\"}],\"content\":\"xz\",\"name\":\"a\"}" },
    { "<a k=\"v\">x<b/><c/>z</a>", "{\"attributes\":{\"k\":\"v\"},\"children\":[{\"attributes\":{},\"name\":\"b\"},{\"attributes\":{},\"name\":\"c\"}],\"content\":\"xz\",\"name\":\"a\"}" },
    { "<a>&lt;&#65;</a>", "{\"attributes\":{},\"content\":\"<A\",\"name\":\"a\"}" }
  };
  for (uint32_t i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
  {
    iot_data_t * data = iot_data_from_xml (cases[i].in);
    char * json = iot_data_to_json (data);
    CU_ASSERT (strcmp (json, cases[i].out) == 0)
    free (json);
    iot_data_free (data);
  }
}
#endif

static void test_json_invalid (void)
{
  // Malformed and invalid JSON is rejected (returning null), without reading outside the input (checked by valgrind)
  static const char * invalid[] =
  {
    "{\"a\"}", "{\"a\",\"b\",\"c\",\"d\"}", "{\"a\":{}:1}", "{\"a\":}", "{\"a\" \"b\"}", "{\"a\":1 \"b\":2}", "{a:1}", "{1:2}",
    "[\"a\":1]", "[}", "{]", "[1,2", "{\"a\":1}x", "1 2", "{\"enabled\":tomato}", "[nothing]", "[010]", "[1.]", "[-]",
    "[.5]", "[1e]", "[\"a\x01" "b\"]", "[\"a\\u0000b\"]", "[\"\\x\"]", "[\"\\u12\"]", "\"abc"
  };
  for (uint32_t i = 0; i < sizeof (invalid) / sizeof (invalid[0]); i++)
  {
    iot_data_t * data = iot_data_from_json (invalid[i]);
    CU_ASSERT (iot_data_type (data) == IOT_DATA_NULL)
    iot_data_free (data);
  }
}

static void test_json_values (void)
{
  // Valid JSON values, including numbers outside 64 bit range, NaN as written by iot_data_to_json and escapes
  static const struct { const char * in; const char * out; } cases[] =
  {
    { "[true,false,null]", "[true,false,null]" },
    { "[1,-2,0,-0,9223372036854775807,-9223372036854775808,18446744073709551615]", "[1,-2,0,0,9223372036854775807,-9223372036854775808,18446744073709551615]" },
    { "[99999999999999999999]", "[1.0000000000000000e+20]" },
    { "[2.5e2]", "[2.5000000000000000e+02]" },
    { "{\"a\":[1,{\"b\":null}],\"c\":\"d\\n\\t\\\"\\\\\\/\"}", "{\"a\":[1,{\"b\":null}],\"c\":\"d\\n\\t\\\"\\\\/\"}" },
    { "[\"\\u00e9\\u20ac\"]", "[\"\xc3\xa9\xe2\x82\xac\"]" },
    { "[\"\\ud83d\\ude00\"]", "[\"\xf0\x9f\x98\x80\"]" },
    { "[\"\\ud800x\",\"\\udc00\"]", "[\"\xef\xbf\xbdx\",\"\xef\xbf\xbd\"]" }, // Unpaired surrogates replaced
    { "[1,]", "[1]" } // Trailing comma tolerated
  };
  for (uint32_t i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
  {
    iot_data_t * data = iot_data_from_json (cases[i].in);
    char * json = iot_data_to_json (data);
    CU_ASSERT (strcmp (json, cases[i].out) == 0)
    free (json);
    iot_data_free (data);
  }
  iot_data_t * data = iot_data_from_json ("[nan,-nan]");
  CU_ASSERT (isnan (iot_data_f64 (iot_data_vector_get (data, 0u))) && isnan (iot_data_f64 (iot_data_vector_get (data, 1u))))
  iot_data_free (data);
}

static char * test_json_nested (uint32_t depth)
{
  char * json = malloc (2u * depth + 1u);
  memset (json, '[', depth);
  memset (json + depth, ']', depth);
  json[2u * depth] = '\0';
  return json;
}

static void test_json_depth (void)
{
  // Nesting depth is limited, so deeply nested input cannot exhaust the stack (or take quadratic time)
  char * json = test_json_nested (256u);
  iot_data_t * data = iot_data_from_json (json);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_VECTOR)
  iot_data_free (data);
  free (json);
  json = test_json_nested (257u);
  data = iot_data_from_json (json);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_NULL)
  iot_data_free (data);
  free (json);
  json = test_json_nested (500000u);
  data = iot_data_from_json (json);
  CU_ASSERT (iot_data_type (data) == IOT_DATA_NULL)
  iot_data_free (data);
  free (json);
}

static void test_data_to_json_large (void)
{
  // Output well beyond initial buffer size must be complete, and parse back to the same data
  iot_data_t * list = iot_data_alloc_list ();
  char buf[64];
  for (uint32_t i = 0; i < 20000u; i++)
  {
    snprintf (buf, sizeof (buf), "value \"%u\"\n", i); // Escaped characters change output length
    iot_data_list_tail_push (list, iot_data_alloc_string (buf, IOT_DATA_COPY));
  }
  char * json = iot_data_to_json (list);
  CU_ASSERT (strlen (json) > 100000u)
  iot_data_t * vector = iot_data_from_json (json);
  CU_ASSERT (iot_data_vector_size (vector) == 20000u)
  char * json2 = iot_data_to_json (vector); // Lists and vectors are both output as JSON arrays
  CU_ASSERT (strcmp (json, json2) == 0)
  free (json2);
  iot_data_free (vector);
  free (json);
  iot_data_free (list);
}

#ifdef IOT_HAS_CBOR
static void test_cbor_decode_robust (void)
{
  // Decoding must not assert or leak on undefined, unsupported or invalid input
  static const struct { const uint8_t bytes[8]; uint32_t len; const char * json; } cases[] =
  {
    { { 0xf7 }, 1, "null" }, { { 0x82, 0x01, 0xf7 }, 3, "[1,null]" }, { { 0xa1, 0x61, 0x61, 0xf7 }, 4, "{\"a\":null}" },
    { { 0x40 }, 1, "\"\"" }, { { 0x60 }, 1, "\"\"" }, { { 0x82, 0x01 }, 2, NULL }, { { 0xf8, 0x20 }, 2, NULL },
    { { 0x82, 0x01, 0xf8, 0x20 }, 4, NULL }, { { 0xa1, 0x01, 0xf8, 0x20 }, 4, NULL }, { { 0xa1, 0xf8, 0x20, 0x01 }, 4, NULL }
  };
  for (uint32_t i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
  {
    iot_data_t * data = iot_data_from_cbor (cases[i].bytes, cases[i].len);
    CU_ASSERT ((data == NULL) == (cases[i].json == NULL))
    if (data && cases[i].json)
    {
      char * json = iot_data_to_json (data);
      CU_ASSERT (strcmp (json, cases[i].json) == 0)
      free (json);
    }
    iot_data_free (data);
  }
}

static void test_cbor_float_round_trip (void)
{
  // Floats must be encoded at full width, as zero or small values previously encoded as other types
  static const double vals[] = { 0.0, -0.0, 1.0e-320, 1.5, -2.25e10 };
  for (uint32_t i = 0; i < sizeof (vals) / sizeof (vals[0]); i++)
  {
    iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
    iot_data_string_map_add (map, "f32", iot_data_alloc_f32 ((float) vals[i]));
    iot_data_string_map_add (map, "f64", iot_data_alloc_f64 (vals[i]));
    float farr[] = { (float) vals[i] };
    double darr[] = { vals[i] };
    iot_data_string_map_add (map, "a32", iot_data_alloc_array (farr, 1u, IOT_DATA_FLOAT32, IOT_DATA_COPY));
    iot_data_string_map_add (map, "a64", iot_data_alloc_array (darr, 1u, IOT_DATA_FLOAT64, IOT_DATA_COPY));
    iot_data_t * cbor = iot_data_to_cbor (map);
    iot_data_t * res = iot_data_from_cbor (iot_data_address (cbor), iot_data_array_length (cbor));
    CU_ASSERT (res != NULL)
    if (res)
    {
      const iot_data_t * f32 = iot_data_string_map_get (res, "f32");
      const iot_data_t * f64 = iot_data_string_map_get (res, "f64");
      CU_ASSERT (iot_data_type (f32) == IOT_DATA_FLOAT32 && iot_data_f32 (f32) == (float) vals[i])
      CU_ASSERT (iot_data_type (f64) == IOT_DATA_FLOAT64 && iot_data_f64 (f64) == vals[i])
      const iot_data_t * a32 = iot_data_string_map_get (res, "a32");
      const iot_data_t * a64 = iot_data_string_map_get (res, "a64");
      CU_ASSERT (iot_data_type (a32) == IOT_DATA_VECTOR && iot_data_f32 (iot_data_vector_get (a32, 0u)) == (float) vals[i])
      CU_ASSERT (iot_data_type (a64) == IOT_DATA_VECTOR && iot_data_f64 (iot_data_vector_get (a64, 0u)) == vals[i])
    }
    iot_data_free (res);
    iot_data_free (cbor);
    iot_data_free (map);
  }
}

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
    CU_ASSERT (iot_data_hash (cbor) == 1151834325U)
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

  unsigned char negative_short_int_data[] = {0x39, 0x01, 0xf4, 0xFF};
  iot_data_t *expected_val = iot_data_alloc_i16 (-501);
  from_cbor = iot_data_from_cbor (negative_short_int_data, ARRAY_SIZE(negative_short_int_data));
  CU_ASSERT_PTR_NOT_NULL_FATAL (from_cbor)
  CU_ASSERT_TRUE (iot_data_equal_value (from_cbor, expected_val))
  iot_data_free (from_cbor);
  iot_data_free (expected_val);

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

  unsigned char negative_int_data[] = {0x3a, 0xa5, 0xf7, 0x02, 0xb3, 0xFF};
  expected_val = iot_data_alloc_i64 (-2784428724);
  from_cbor = iot_data_from_cbor (negative_int_data, ARRAY_SIZE(negative_int_data));
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
  iot_data_t *bool_value = iot_data_true;
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

  //indefinite strings and bytestrings, empty and with empty chunks
  unsigned char indef_empty_string[] = {0x7F, 0xFF};
  unsigned char indef_empty_chunk_string[] = {0x7F, 0x60, 0x62, 'a', 'b', 0x60, 0xFF};
  unsigned char indef_empty_binary[] = {0x5F, 0xFF};
  unsigned char indef_empty_chunk_binary[] = {0x5F, 0x40, 0x42, 0xaa, 0xbb, 0x40, 0xFF};
  from_cbor = iot_data_from_cbor (indef_empty_string, ARRAY_SIZE (indef_empty_string));
  CU_ASSERT_FATAL (from_cbor && iot_data_type (from_cbor) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_data_string (from_cbor), "") == 0)
  iot_data_free (from_cbor);
  from_cbor = iot_data_from_cbor (indef_empty_chunk_string, ARRAY_SIZE (indef_empty_chunk_string));
  CU_ASSERT_FATAL (from_cbor && iot_data_type (from_cbor) == IOT_DATA_STRING)
  CU_ASSERT (strcmp (iot_data_string (from_cbor), "ab") == 0)
  iot_data_free (from_cbor);
  from_cbor = iot_data_from_cbor (indef_empty_binary, ARRAY_SIZE (indef_empty_binary));
  CU_ASSERT_FATAL (from_cbor && iot_data_type (from_cbor) == IOT_DATA_BINARY)
  CU_ASSERT (iot_data_array_length (from_cbor) == 0u)
  iot_data_free (from_cbor);
  from_cbor = iot_data_from_cbor (indef_empty_chunk_binary, ARRAY_SIZE (indef_empty_chunk_binary));
  CU_ASSERT_FATAL (from_cbor && iot_data_type (from_cbor) == IOT_DATA_BINARY)
  CU_ASSERT (iot_data_array_length (from_cbor) == 2u)
  CU_ASSERT (memcmp (iot_data_address (from_cbor), binary_data, 2u) == 0)
  iot_data_free (from_cbor);

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
static void test_data_from_yaml_depth (void)
{
  // Nesting depth is limited, so deeply nested input cannot exhaust the stack
  char * yaml = malloc (100001u);
  memset (yaml, '[', 100000u);
  yaml[100000u] = '\0';
  iot_data_t * exception = NULL;
  iot_data_t * data = iot_data_from_yaml (yaml, &exception);
  CU_ASSERT (data == NULL)
  CU_ASSERT (exception != NULL && strstr (iot_data_string (exception), "depth") != NULL)
  iot_data_free (exception);
  free (yaml);
  exception = NULL;
  data = iot_data_from_yaml ("[[[1]]]", &exception);
  CU_ASSERT (data != NULL && exception == NULL)
  iot_data_free (data);
}

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

static void test_data_from_yaml_values (void)
{
  // Scalars are typed using the YAML 1.2 core schema, tags force or check the type
  static const struct { const char * in; const char * out; } cases[] =
  {
    { "[true, True, TRUE, false, null, Null, ~, yes, no, on]", "[true,true,true,false,null,null,null,\"yes\",\"no\",\"on\"]" },
    { "[010, 0o10, 0x1F, 0xff, +5, -0, 007]", "[10,8,31,255,5,0,7]" },
    { "[9223372036854775807, 9223372036854775808, 18446744073709551615, -9223372036854775808]", "[9223372036854775807,9223372036854775808,18446744073709551615,-9223372036854775808]" },
    { "[18446744073709551616, -9223372036854775809]", "[1.8446744073709552e+19,-9.2233720368547758e+18]" },
    { "[1.5, .5, 1., 2e3, -1.5E-1]", "[1.5000000000000000e+00,5.0000000000000000e-01,1.0000000000000000e+00,2.0000000000000000e+03,-1.4999999999999999e-01]" },
    { "[0x, 0o8, 0x10000000000000000, 1_000, 1e, inf, infinity, 0x1p3, \"12\", '12', 1.2.3, -, 12abc]", "[\"0x\",\"0o8\",\"0x10000000000000000\",\"1_000\",\"1e\",\"inf\",\"infinity\",\"0x1p3\",\"12\",\"12\",\"1.2.3\",\"-\",\"12abc\"]" },
    { "[!!str 12, !!str true, !!int 12, !!float 12, !!bool false, !!null null, !custom 12, ! 12]", "[\"12\",\"true\",12,1.2000000000000000e+01,false,null,\"12\",\"12\"]" },
    { "{200: ok, \"a\": [], b: {}}", "{\"200\":\"ok\",\"a\":[],\"b\":{}}" },
    { "---\n42\n...\n", "42" }
  };
  for (uint32_t i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
  {
    iot_data_t * ex = NULL;
    iot_data_t * data = iot_data_from_yaml (cases[i].in, &ex);
    CU_ASSERT (ex == NULL)
    char * json = iot_data_to_json (data);
    if (strcmp (json, cases[i].out) != 0) printf ("\nYAML: %s\nGOT: %s\nEXP: %s\n", cases[i].in, json, cases[i].out);
    CU_ASSERT (strcmp (json, cases[i].out) == 0)
    free (json);
    iot_data_free (data);
    iot_data_free (ex);
  }
  iot_data_t * ex = NULL;
  iot_data_t * data = iot_data_from_yaml ("[.inf, -.Inf, +.INF, .nan, .NaN, nan, -nan]", &ex);
  CU_ASSERT_FATAL (data != NULL)
  CU_ASSERT (isinf (iot_data_f64 (iot_data_vector_get (data, 0u))) && iot_data_f64 (iot_data_vector_get (data, 0u)) > 0.0)
  CU_ASSERT (isinf (iot_data_f64 (iot_data_vector_get (data, 1u))) && iot_data_f64 (iot_data_vector_get (data, 1u)) < 0.0)
  CU_ASSERT (isinf (iot_data_f64 (iot_data_vector_get (data, 2u))) && iot_data_f64 (iot_data_vector_get (data, 2u)) > 0.0)
  for (uint32_t i = 3u; i < 7u; i++) CU_ASSERT (isnan (iot_data_f64 (iot_data_vector_get (data, i))))
  iot_data_free (data);
}

static void test_data_from_yaml_alias (void)
{
  // Aliases share the anchored node, merge keys add entries not already present
  static const struct { const char * in; const char * out; } cases[] =
  {
    { "a: &x 1\nb: *x\nc: 2", "{\"a\":1,\"b\":1,\"c\":2}" },
    { "base: &b {x: 1, y: 2}\ncopy: *b\nlist: [*b, *b]", "{\"base\":{\"x\":1,\"y\":2},\"copy\":{\"x\":1,\"y\":2},\"list\":[{\"x\":1,\"y\":2},{\"x\":1,\"y\":2}]}" },
    { "base: &b {x: 1, y: 2}\nderived:\n  y: 3\n  <<: *b\n  z: 4", "{\"base\":{\"x\":1,\"y\":2},\"derived\":{\"x\":1,\"y\":3,\"z\":4}}" },
    { "a: &a {x: 1}\nb: &b {x: 2, y: 2}\nc:\n  <<: [*a, *b]", "{\"a\":{\"x\":1},\"b\":{\"x\":2,\"y\":2},\"c\":{\"x\":1,\"y\":2}}" },
    { "\"<<\": 1", "{\"<<\":1}" },
    { "&k key: v\n*k : w", "{\"key\":\"w\"}" },
    { "s: &s [1, 2]\nt: *s", "{\"s\":[1,2],\"t\":[1,2]}" }
  };
  for (uint32_t i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
  {
    iot_data_t * ex = NULL;
    iot_data_t * data = iot_data_from_yaml (cases[i].in, &ex);
    CU_ASSERT (ex == NULL)
    char * json = iot_data_to_json (data);
    if (strcmp (json, cases[i].out) != 0) printf ("\nYAML: %s\nGOT: %s\nEXP: %s\n", cases[i].in, json, cases[i].out);
    CU_ASSERT (strcmp (json, cases[i].out) == 0)
    free (json);
    iot_data_free (data);
    iot_data_free (ex);
  }
}

static void test_data_from_yaml_invalid (void)
{
  // Undefined aliases, invalid tagged values, unsupported keys, multiple documents and trailing content are rejected
  static const char * invalid[] =
  {
    "a: *x", "[*x]", "a: &x [*x]", "!!int abc", "[!!bool 1]", "!!null 0", "!!float abc", "{[1]: 2}", "{{a: 1}: 2}",
    "a: &x 1\n*x : 2", "a: 1\n<<: 2", "a: 1\n<<: [1]", "a: 1\n---\nb: 2", "[1, 2]]", "{a: 1}}", "a: [1, 2", "a: b: c"
  };
  for (uint32_t i = 0; i < sizeof (invalid) / sizeof (invalid[0]); i++)
  {
    iot_data_t * ex = NULL;
    iot_data_t * data = iot_data_from_yaml (invalid[i], &ex);
    if (data || ex == NULL) printf ("\nAccepted YAML: %s\n", invalid[i]);
    CU_ASSERT (data == NULL)
    CU_ASSERT (iot_data_type (ex) == IOT_DATA_STRING)
    iot_data_free (data);
    iot_data_free (ex);
  }
}
#endif

#ifdef IOT_HAS_CBOR
static void test_cbor_round_trip_types (void)
{
  // Arrays of each integer type and booleans, and lists, written to CBOR and read back
  int8_t i8[] = { -1, 2 };
  uint8_t u8[] = { 200, 3 };
  int16_t i16[] = { -300, 4 };
  uint16_t u16[] = { 60000, 5 };
  int32_t i32[] = { -70000, 6 };
  uint32_t u32[] = { 4000000000u, 7 };
  int64_t i64[] = { -5000000000, 8 };
  uint64_t u64[] = { 10000000000u, 9 };
  bool bl[] = { true, false };
  const struct { iot_data_type_t type; void * data; } arrays[] =
  {
    { IOT_DATA_INT8, i8 }, { IOT_DATA_UINT8, u8 }, { IOT_DATA_INT16, i16 }, { IOT_DATA_UINT16, u16 }, { IOT_DATA_INT32, i32 },
    { IOT_DATA_UINT32, u32 }, { IOT_DATA_INT64, i64 }, { IOT_DATA_UINT64, u64 }, { IOT_DATA_BOOL, bl }
  };
  for (unsigned i = 0; i < ARRAY_SIZE (arrays); i++)
  {
    iot_data_t * array = iot_data_alloc_array (arrays[i].data, 2u, arrays[i].type, IOT_DATA_REF);
    iot_data_t * cbor = iot_data_to_cbor (array);
    iot_data_t * read = iot_data_from_iot_cbor (cbor);
    CU_ASSERT (read && iot_data_type (read) == IOT_DATA_VECTOR && iot_data_vector_size (read) == 2u) // CBOR arrays read as vectors
    char * json1 = iot_data_to_json (array);
    char * json2 = iot_data_to_json (read);
    CU_ASSERT (strcmp (json1, json2) == 0) // Same values
    free (json1);
    free (json2);
    iot_data_free (read);
    iot_data_free (cbor);
    iot_data_free (array);
  }

  iot_data_t * list = iot_data_alloc_list ();
  iot_data_list_tail_push (list, iot_data_alloc_string ("one", IOT_DATA_REF));
  iot_data_list_tail_push (list, iot_data_alloc_i32 (2));
  iot_data_list_tail_push (list, iot_data_alloc_null ());
  iot_data_t * cbor = iot_data_to_cbor (list);
  iot_data_t * read = iot_data_from_iot_cbor (cbor);
  CU_ASSERT (read && iot_data_type (read) == IOT_DATA_VECTOR && iot_data_vector_size (read) == 3u) // Lists read as vectors
  char * json1 = iot_data_to_json (list);
  char * json2 = iot_data_to_json (read);
  CU_ASSERT (strcmp (json1, json2) == 0) // Same values, in list iteration order
  free (json1);
  free (json2);
  iot_data_free (read);
  iot_data_free (cbor);
  iot_data_free (list);

  // Output larger than initial size
  char long_str[1000];
  memset (long_str, 'x', sizeof (long_str) - 1u);
  long_str[sizeof (long_str) - 1u] = '\0';
  iot_data_t * str = iot_data_alloc_string (long_str, IOT_DATA_REF);
  cbor = iot_data_to_cbor_with_size (str, 4u);
  read = iot_data_from_iot_cbor (cbor);
  CU_ASSERT (read && iot_data_equal (read, str))
  iot_data_free (read);
  iot_data_free (cbor);
  iot_data_free (str);
}

static void test_cbor_decode_cases (void)
{
  // Half precision float read as float32
  uint8_t half[] = { 0xF9, 0x3C, 0x00 }; // 1.0
  iot_data_t * data = iot_data_from_cbor (half, sizeof (half));
  CU_ASSERT (data && iot_data_type (data) == IOT_DATA_FLOAT32 && iot_data_f32 (data) == 1.0f)
  iot_data_free (data);

  // Undefined read as null
  uint8_t undefined[] = { 0xF7 };
  data = iot_data_from_cbor (undefined, sizeof (undefined));
  CU_ASSERT (data && iot_data_type (data) == IOT_DATA_NULL)
  iot_data_free (data);

  // Unsupported simple values, alone and in arrays or maps, are not read
  uint8_t simple[] = { 0xF0 }; // Simple value 16
  uint8_t array_simple[] = { 0x82, 0x01, 0xF0 };
  uint8_t map_simple[] = { 0xA1, 0x61, 'a', 0xF0 };
  uint8_t map_key_simple[] = { 0xA1, 0xF0, 0x01 };
  CU_ASSERT (iot_data_from_cbor (simple, sizeof (simple)) == NULL)
  CU_ASSERT (iot_data_from_cbor (array_simple, sizeof (array_simple)) == NULL)
  CU_ASSERT (iot_data_from_cbor (map_simple, sizeof (map_simple)) == NULL)
  CU_ASSERT (iot_data_from_cbor (map_key_simple, sizeof (map_key_simple)) == NULL)
}
#endif

static void test_pointer_output (void)
{
  // Pointer values written as null, so JSON and CBOR output is valid and read back
  static int x;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "p", iot_data_alloc_pointer (&x, NULL));
  iot_data_string_map_add (map, "q", iot_data_alloc_ui32 (1u));
  iot_data_t * vector = iot_data_alloc_vector (2u);
  iot_data_vector_add (vector, 0u, iot_data_alloc_pointer (&x, NULL));
  iot_data_vector_add (vector, 1u, iot_data_alloc_ui32 (2u));
  iot_data_string_map_add (map, "v", vector);
  char * json = iot_data_to_json (map);
  CU_ASSERT (strcmp (json, "{\"p\":null,\"q\":1,\"v\":[null,2]}") == 0)
  iot_data_t * read = iot_data_from_json (json);
  CU_ASSERT (iot_data_type (read) == IOT_DATA_MAP && iot_data_type (iot_data_string_map_get (read, "p")) == IOT_DATA_NULL)
  iot_data_free (read);
  free (json);

#ifdef IOT_HAS_CBOR
  iot_data_t * cbor = iot_data_to_cbor (map);
  read = iot_data_from_iot_cbor (cbor);
  CU_ASSERT_FATAL (read != NULL)
  json = iot_data_to_json (read);
  CU_ASSERT (strcmp (json, "{\"p\":null,\"q\":1,\"v\":[null,2]}") == 0)
  free (json);
  iot_data_free (read);
  iot_data_free (cbor);
#endif
  iot_data_free (map);
}

static void test_json_parse_cases (void)
{
  // String escapes, including upper case unicode hex digits
  iot_data_t * data = iot_data_from_json ("\"\\b\\f\\r\\u00E9\"");
  CU_ASSERT (data && strcmp (iot_data_string (data), "\b\f\r\xc3\xa9") == 0)
  iot_data_free (data);
  CU_ASSERT (iot_data_type (iot_data_from_json ("\"\\u00G9\"")) == IOT_DATA_NULL) // Invalid hex digit, so not valid JSON

  // Tokeniser without token storage counts tokens. Invalid input rejected
  iot_json_parser parser;
  iot_json_init (&parser);
  const char * json = "{\"a\":[1,\"b\"]}";
  CU_ASSERT (iot_json_parse (&parser, json, strlen (json), NULL, 0) == 5)
  iot_json_tok_t tokens[8];
  const char * invalid[] = { "[tr\x01ue]", "]", "{\"a\"]", "\"a\"]" };
  for (unsigned i = 0; i < ARRAY_SIZE (invalid); i++)
  {
    iot_json_init (&parser);
    CU_ASSERT (iot_json_parse (&parser, invalid[i], strlen (invalid[i]), tokens, 8) == IOT_JSON_ERROR_INVAL)
  }
}

#ifdef IOT_HAS_XML
static void test_xml_many_children (void)
{
  // Elements with more children than initially allocated
  char xml[2048] = "<root>";
  for (int i = 0; i < 20; i++) snprintf (xml + strlen (xml), sizeof (xml) - strlen (xml), "<child n=\"%d\"/>", i);
  strcat (xml, "</root>");
  iot_data_t * data = iot_data_from_xml (xml);
  CU_ASSERT_FATAL (data != NULL)
  const iot_data_t * children = iot_data_string_map_get (data, "children");
  CU_ASSERT (children && iot_data_vector_size (children) == 20u)
  iot_data_free (data);
}
#endif

#ifdef IOT_HAS_YAML
static void test_yaml_other_cases (void)
{
  // Value with an unrecognised tag kept as string. Map key followed by invalid YAML rejected
  iot_data_t * exception = NULL;
  iot_data_t * data = iot_data_from_yaml ("a: !custom value", &exception);
  CU_ASSERT (data && exception == NULL)
  CU_ASSERT (data && strcmp (iot_data_string_map_get_string (data, "a"), "value") == 0)
  iot_data_free (data);
  data = iot_data_from_yaml ("a: !!timestamp 2001-12-14", &exception); // Standard tag for unsupported type
  CU_ASSERT (data && exception == NULL)
  CU_ASSERT (data && strcmp (iot_data_string_map_get_string (data, "a"), "2001-12-14") == 0)
  iot_data_free (data);
  const char * invalid[] = { "a: [1, 2", "a:\n\t- x", "{a: ]" };
  for (unsigned i = 0; i < ARRAY_SIZE (invalid); i++)
  {
    exception = NULL;
    data = iot_data_from_yaml (invalid[i], &exception);
    CU_ASSERT (data == NULL && exception != NULL)
    iot_data_free (exception);
  }
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
  CU_add_test (suite, "data_from_xml_invalid", test_data_from_xml_invalid);
  CU_add_test (suite, "data_from_xml_content", test_data_from_xml_content);
  CU_add_test (suite, "xml_many_children", test_xml_many_children);
#endif
#ifdef IOT_HAS_CBOR
  CU_add_test (suite, "data_to_cbor", test_data_to_cbor);
  CU_add_test (suite, "cbor_to_data", test_cbor_to_data);
  CU_add_test (suite, "cbor_float_round_trip", test_cbor_float_round_trip);
  CU_add_test (suite, "cbor_decode_robust", test_cbor_decode_robust);
  CU_add_test (suite, "cbor_round_trip_types", test_cbor_round_trip_types);
  CU_add_test (suite, "cbor_decode_cases", test_cbor_decode_cases);
#endif
  CU_add_test (suite, "data_to_json_large", test_data_to_json_large);
  CU_add_test (suite, "json_invalid", test_json_invalid);
  CU_add_test (suite, "json_values", test_json_values);
  CU_add_test (suite, "json_depth", test_json_depth);
  CU_add_test (suite, "json_parse_cases", test_json_parse_cases);
  CU_add_test (suite, "pointer_output", test_pointer_output);
#ifdef IOT_HAS_YAML
  CU_add_test (suite, "data_from_yaml", test_data_from_yaml);
  CU_add_test (suite, "data_from_yaml_depth", test_data_from_yaml_depth);
  CU_add_test (suite, "data_from_yaml_values", test_data_from_yaml_values);
  CU_add_test (suite, "data_from_yaml_alias", test_data_from_yaml_alias);
  CU_add_test (suite, "data_from_yaml_invalid", test_data_from_yaml_invalid);
  CU_add_test (suite, "yaml_other_cases", test_yaml_other_cases);
#endif
}
