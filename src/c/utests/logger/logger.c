/*
 * Copyright (c) 2020-2021
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger.h"
#include "iot/container.h"
#include "CUnit.h"

#define IOT_TEST_LOGGER_TYPE "IOT::UTEST::Logger"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static uint32_t cunit_custom_log_count = 0;

static const char * logger1_config =
"{"
  "\"Name\":\"logger1\","
  "\"Level\":\"Warning\""
"}";

static const char * logger2_config =
"{"
  "\"Name\":\"logger2\","
  "\"Next\":\"logger1\","
  "\"Level\":\"Info\""
"}";

static const char * logger3_config =
"{"
  "\"Name\":\"logger3\","
  "\"Next\":\"logger2\","
  "\"Level\":\"Debug\""
"}";

static void cunit_custom_log_fn (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  (void) logger;
  (void) level;
  (void) timestamp;
  (void) message;
  (void) ctx;
  cunit_custom_log_count++;
}

static void cunit_test_logs (iot_logger_t * logger)
{
  iot_log_warn (logger, "Test Warning");
  iot_log_error (logger, "Test Error");
  iot_log_trace (logger, "Test Trace");
  iot_log_debug (logger, "Test Debug");
  iot_log_info (logger, "Test Error");
}

static void cunit_logger_default (void)
{
  iot_logger_t * logger = iot_logger_default ();
  iot_logger_start (logger);
  iot_log_warn (logger, "Test Warning from default logger");
  iot_logger_free (logger); /* Should ignore default logger */
  iot_log_error (logger, "Test Error from default logger");
  iot_log_trace (logger, "ERROR should not see trace log");
  iot_log_debug (logger, "ERROR should not see debug log");
  iot_log_info (logger, "ERROR should not see info log");
}

static void cunit_logger_named (void)
{
  iot_logger_t * logger = iot_logger_alloc ("MyLogger", IOT_LOG_ERROR, true);
  iot_log_error (logger, "Test Error from my logger");
  iot_log_warn (logger, "ERROR should not see warn log");
  iot_log_trace (logger, "ERROR should not see trace log");
  iot_log_debug (logger, "ERROR should not see debug log");
  iot_log_info (logger, "ERROR should not see info log");
  iot_logger_free (logger);
}

static void cunit_logger_impl (void)
{
  iot_logger_t * logger = iot_logger_alloc_custom ("Sub", IOT_LOG_NONE, false, NULL, cunit_custom_log_fn, NULL, NULL);
  iot_logger_start (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 0)
  iot_logger_set_level (logger, IOT_LOG_ERROR);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 1)
  iot_logger_set_level (logger, IOT_LOG_WARN);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 3)
  iot_logger_set_level (logger, IOT_LOG_INFO);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 6)
  iot_logger_set_level (logger, IOT_LOG_DEBUG);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 10)
  iot_logger_set_level (logger, IOT_LOG_TRACE);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 15)
  iot_logger_free (logger);
}

static void cunit_logger_sub (void)
{
  iot_logger_t * sub = iot_logger_default ();
  iot_logger_t * logger = iot_logger_alloc_custom ("Custom", IOT_LOG_INFO, false, NULL, cunit_custom_log_fn, sub, NULL);
  iot_logger_start (sub);
  iot_logger_start (logger);
  iot_logger_set_level (sub, IOT_LOG_NONE);
  cunit_custom_log_count = 0;
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 3)
  iot_logger_free (logger);
}

static void cunit_logger_file (void)
{
  iot_logger_t * logger = iot_logger_alloc_file ("File", IOT_LOG_WARN, false, NULL, "./test.log");
  iot_logger_start (logger);
  cunit_test_logs (logger);
  iot_logger_free (logger);
}

static void cunit_logger_udp (void)
{
  iot_logger_t * logger = iot_logger_alloc_udp ("udp", IOT_LOG_WARN, false, NULL, "localhost", 22222);
  iot_logger_start (logger);
  cunit_test_logs (logger);
  iot_logger_free (logger);
}

static void cunit_logger_udp_broadcast (void)
{
  iot_logger_t * logger = iot_logger_alloc_udp ("udp-broadcast", IOT_LOG_WARN, false, NULL, NULL, 33333);
  iot_logger_start (logger);
  cunit_test_logs (logger);
  iot_logger_free (logger);
}

static void cunit_logger_null (void)
{
  cunit_test_logs (NULL); // Should be able to have logger as NULL
  iot_logger_free (NULL);
}

static void cunit_logger_start_stop (void)
{
  iot_logger_t * logger = iot_logger_alloc_custom ("StartStop", IOT_LOG_WARN, false, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0;
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 0)
  iot_logger_start (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 2)
  iot_logger_stop (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 2)
  iot_logger_start (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 4)
  iot_logger_free (logger);
}

static void cunit_logger_refcount (void)
{
  iot_logger_t *logger = iot_logger_alloc_custom ("StartStop", IOT_LOG_WARN, false, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0;
  iot_logger_add_ref (logger);
  iot_logger_start (logger);
  iot_logger_free (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 2)
  iot_logger_free (logger);
}

static void cunit_logger_selfstart (void)
{
  iot_logger_t *logger = iot_logger_alloc ("MyLogger", IOT_LOG_TRACE, true);
  iot_log_trace (logger, "TRACE: Test logger start");
  iot_logger_free (logger);
}

static void cunit_logger_format (void)
{
  iot_logger_t *logger = iot_logger_alloc ("MyLogger", IOT_LOG_INFO, true);
  iot_log_info (logger, "INFO: %s %u", "testarg", 1234);
  iot_logger_free (logger);
}

static void cunit_logger_set_next (void)
{
  iot_logger_t * logger = iot_logger_alloc ("MyLogger", IOT_LOG_INFO, true);
  iot_logger_t * next = iot_logger_alloc_custom ("StartStop", IOT_LOG_ERROR, true, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0u;
  iot_logger_set_next (logger, next);
  iot_log_info (logger, "Info: set_next");
  iot_log_error (logger, "Error: set_next");
  CU_ASSERT (cunit_custom_log_count == 1u)
  iot_logger_free (logger);
  iot_logger_free (next);
}

static void cunit_logger_level_name (void)
{
  CU_ASSERT (strcmp ("", iot_logger_level_to_string (IOT_LOG_NONE)) == 0)
  CU_ASSERT (strcmp ("ERROR", iot_logger_level_to_string (IOT_LOG_ERROR)) == 0)
  CU_ASSERT (strcmp ("WARN", iot_logger_level_to_string (IOT_LOG_WARN)) == 0)
  CU_ASSERT (strcmp ("Info", iot_logger_level_to_string (IOT_LOG_INFO)) == 0)
  CU_ASSERT (strcmp ("Debug", iot_logger_level_to_string (IOT_LOG_DEBUG)) == 0)
  CU_ASSERT (strcmp ("Trace", iot_logger_level_to_string (IOT_LOG_TRACE)) == 0)
}

static iot_component_t * cunit_logger_chained_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t *result;
  iot_loglevel_t level = iot_logger_level_from_string (iot_data_string_map_get_string (map, "Level"));
  iot_logger_t * next = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Next"));
  bool start = iot_data_string_map_get_bool (map, "Start", true);
  const char * name = iot_data_string_map_get_string (map, "Name");
  result = iot_logger_alloc_custom (name, level, start, next, cunit_custom_log_fn, NULL, NULL);
  return (iot_component_t*) result;
}

static void cunit_logger_chained (void)
{
  iot_component_factory_t f =
  {
    IOT_TEST_LOGGER_TYPE,
    IOT_CATEGORY_CORE,
    cunit_logger_chained_config,
    iot_logger_factory ()->free_fn,
    iot_logger_factory ()->reconfig_fn,
    NULL
  };
  iot_component_factory_add (&f);
  iot_container_t *cont = iot_container_alloc ("logger_chained");
  iot_container_add_component (cont, IOT_TEST_LOGGER_TYPE, "logger1", logger1_config);
  iot_container_add_component (cont, IOT_TEST_LOGGER_TYPE, "logger2", logger2_config);
  iot_container_add_component (cont, IOT_TEST_LOGGER_TYPE, "logger3", logger3_config);
  iot_logger_t * logger1 = (iot_logger_t *)iot_container_find_component (cont, "logger1");
  iot_logger_t * logger3 = (iot_logger_t *)iot_container_find_component (cont, "logger3");

  cunit_custom_log_count = 0u;
  cunit_test_logs (logger3);
  CU_ASSERT (cunit_custom_log_count == 9u)

  iot_logger_set_level (logger1, IOT_LOG_DEBUG);
  iot_logger_relevel (logger1, cont);
  iot_logger_set_level (logger3, IOT_LOG_WARN);
  iot_logger_relevel (logger3, cont);

  cunit_custom_log_count = 0u;
  cunit_test_logs (logger3);
  CU_ASSERT (cunit_custom_log_count == 9u)

  iot_container_free (cont);
}

void cunit_logger_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("logger", suite_init, suite_clean);
  CU_add_test (suite, "logger_default", cunit_logger_default);
  CU_add_test (suite, "logger_named", cunit_logger_named);
  CU_add_test (suite, "logger_impl", cunit_logger_impl);
  CU_add_test (suite, "logger_sub", cunit_logger_sub);
  CU_add_test (suite, "logger_file", cunit_logger_file);
  CU_add_test (suite, "logger_udp", cunit_logger_udp);
  CU_add_test (suite, "logger_udp_broadcast", cunit_logger_udp_broadcast);
  CU_add_test (suite, "logger_null", cunit_logger_null);
  CU_add_test (suite, "logger_start_stop", cunit_logger_start_stop);
  CU_add_test (suite, "logger_refcount", cunit_logger_refcount);
  CU_add_test (suite, "logger_selfstart", cunit_logger_selfstart);
  CU_add_test (suite, "logger_format", cunit_logger_format);
  CU_add_test (suite, "logger_set_next", cunit_logger_set_next);
  CU_add_test (suite, "logger_level_name", cunit_logger_level_name);
  CU_add_test (suite, "logger_chained", cunit_logger_chained);
}
