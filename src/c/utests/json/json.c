#include "json.h"
#include "CUnit.h"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static int dump_parse (const char *js, iot_json_tok_t *t, int count, int indent)
{
  int i, j, k;

  if (count == 0)
  {
    return 0;
  }
  if (t->type == IOT_JSON_PRIMITIVE)
  {
    printf ("%.*s", (int) (t->end - t->start), js + t->start);
    return 1;
  }
  else if (t->type == IOT_JSON_STRING)
  {
    printf ("'%.*s'", (int) (t->end - t->start), js + t->start);
    return 1;
  }
  else if (t->type == IOT_JSON_OBJECT)
  {
    printf ("\n");
    j = 0;
    for (i = 0; i < t->size; i++)
    {
      for (k = 0; k < indent; k++) { printf ("  "); }
      j += dump_parse (js, t + 1 + j, count - j, indent + 1);
      printf (": ");
      j += dump_parse (js, t + 1 + j, count - j, indent + 1);
      printf ("\n");
    }
    return j + 1;
  }
  else if (t->type == IOT_JSON_ARRAY)
  {
    j = 0;
    printf ("\n");
    for (i = 0; i < t->size; i++)
    {
      for (k = 0; k < indent - 1; k++) { printf ("  "); }
      printf ("   - ");
      j += dump_parse (js, t + 1 + j, count - j, indent + 1);
      printf ("\n");
    }
    return j + 1;
  }
  return 0;
}

static void cunit_json_parse_string (void)
{
  iot_json_tok_t tokens[10];
  iot_json_parser parser;
  int count;

  iot_json_init (&parser);

  static const char * json_fail_unknown_symbol = "\"\\kMyString\"";

  count = iot_json_parse (&parser, json_fail_unknown_symbol, strlen (json_fail_unknown_symbol), NULL, 11);
  CU_ASSERT (count == IOT_JSON_ERROR_INVAL);
  dump_parse (json_fail_unknown_symbol, tokens, count, 0);

  static const char * json_fail = "\"\\uMyString\"";

  count = iot_json_parse (&parser, json_fail, strlen (json_fail), NULL, 11);
  CU_ASSERT (count == IOT_JSON_ERROR_INVAL);
  dump_parse (json_fail, tokens, count, 0);

  static const char * json_hex = "\"\\uA9\"";

  count = iot_json_parse (&parser, json_hex, strlen (json_hex), tokens, 10);
  CU_ASSERT (count != 0);
  dump_parse (json_hex, tokens, count, 0);

  static const char * json = "\"MyString\"";

  count = iot_json_parse (&parser, json, strlen (json), tokens, 10);
  CU_ASSERT (count != 0);
  printf (" # %d ", count);
  dump_parse (json_hex, tokens, count, 0);
}

static void cunit_json_parse_number (void)
{
  iot_json_tok_t tokens[10];
  iot_json_parser parser;
  int count;

  iot_json_init (&parser);
  static const char * json = "4321";

  count = iot_json_parse (&parser, json, strlen (json), tokens, 10);
  CU_ASSERT (count != 0);
  printf (" # %d ", count);
  dump_parse (json, tokens, count, 0);
  printf (" ");
}

static void cunit_json_parse_array (void)
{
  iot_json_tok_t tokens[10];
  iot_json_parser parser;
  int count;

  iot_json_init (&parser);
  static const char * json = "[ \"MyString\", 1234, true, false, null, 11.22 ]";

  count = iot_json_parse (&parser, json, strlen (json), tokens, 10);
  CU_ASSERT (count != 0);
  printf (" # %d ", count);
  dump_parse (json, tokens, count, 0);
  printf (" ");
}

static void cunit_json_parse_object (void)
{
  iot_json_tok_t tokens[10];
  iot_json_parser parser;
  int count;

  iot_json_init (&parser);
  static const char * json = "{ \"Key1\":\"AString\", \"Key2\": 12345 }";

  count = iot_json_parse (&parser, json, strlen (json), tokens, 10);
  CU_ASSERT (count != 0);
  printf (" # %d ", count);
  dump_parse (json, tokens, count, 0);
  printf (" ");
}

static void cunit_json_parse_nested (void)
{
  iot_json_tok_t tokens[20];
  iot_json_parser parser;
  int count;

  iot_json_init (&parser);
  static const char * json = "{ \"Key1\":\"AString\", \"Key2\": 12345, \"Key3\": [ \"YourString\", 6789 ], \"Key4\": { \"SKey1\": false }}";

  count = iot_json_parse (&parser, json, strlen (json), tokens, 20);
  CU_ASSERT (count != 0);
  printf (" # %d ", count);
  dump_parse (json, tokens, count, 0);
  printf (" ");
}

static void cunit_json_parse_config (void)
{
  iot_json_tok_t tokens[20];
  iot_json_parser parser;
  int count;

  iot_json_init (&parser);
  static const char * json =
    "{"
    "\"Interval\": 200000000,"
    "\"Threads\": 10,"
    "\"Topics\": [{ \"Topic\": \"test/tube\", \"Priority\": 10 }]"
    "}";

  count = iot_json_parse (&parser, json, strlen (json), tokens, 20);
  CU_ASSERT (count != 0);
  printf (" # %d ", count);
  dump_parse (json, tokens, count, 0);
  printf (" ");
}

void cunit_json_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("json", suite_init, suite_clean);
  CU_add_test (suite, "json_parse_string", cunit_json_parse_string);
  CU_add_test (suite, "json_parse_number", cunit_json_parse_number);
  CU_add_test (suite, "json_parse_object", cunit_json_parse_object);
  CU_add_test (suite, "json_parse_array", cunit_json_parse_array);
  CU_add_test (suite, "json_parse_nested", cunit_json_parse_nested);
  CU_add_test (suite, "json_parse_config", cunit_json_parse_config);
}
