#include "logger.h"
#include "CUnit.h"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static uint32_t cunit_custom_log_count = 0;

static void cunit_custom_log_fn (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message)
{
   cunit_custom_log_count++;
}

static void cunit_logger_default (void)
{
  iot_logger_t * logger = iot_logger_default ();
  iot_log_warn (logger, "Test Warning from default logger");
  iot_logger_free (logger); /* Should ignore default logger */
  iot_log_error (logger, "Test Error from default logger");
  iot_log_trace (logger, "ERROR should not see trace log");
  iot_log_debug (logger, "ERROR should not see debug log");
  iot_log_info (logger, "ERROR should not see info log");
}

static void cunit_logger_impl (void)
{
  iot_logger_t * logger = iot_logger_alloc ("Custom", "", cunit_custom_log_fn, NULL);
  iot_logger_free (logger);
}


void cunit_logger_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("logger", suite_init, suite_clean);
  CU_add_test (suite, "logger_default", cunit_logger_default);
  CU_add_test (suite, "logger_impl", cunit_logger_impl);
}
