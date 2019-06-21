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

static void cunit_logger_default (void)
{
  iot_logger_t * logger = iot_logger_default ();
  iot_log_warn (logger, "Test Warning from default logger");
}

void cunit_logger_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("logger", suite_init, suite_clean);
  CU_add_test (suite, "logger_default", cunit_logger_default);
}
