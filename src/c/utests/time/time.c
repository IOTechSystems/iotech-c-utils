#include "time.h"
#include "CUnit.h"

#define MAX_COUNTER 1000

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void test_time_msecs (void)
{
  volatile uint64_t msecs_time1, msecs_time2;
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    msecs_time1 = iot_time_msecs ();
    usleep (1000);
    msecs_time2 = iot_time_msecs ();

    CU_ASSERT (msecs_time1 != msecs_time2);
  }
}

static void test_time_nsecs (void)
{
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    CU_ASSERT (iot_time_nsecs () != iot_time_nsecs ());
  }
}

void cunit_time_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("time", suite_init, suite_clean);
  CU_add_test (suite, "time_msecs", test_time_msecs);
  CU_add_test (suite, "time_nsecs", test_time_nsecs);
}
