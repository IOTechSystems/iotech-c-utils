#include "iot/iot.h"
#include "misc.h"
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
  volatile uint64_t msecs_time;
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    msecs_time = iot_time_msecs ();
    usleep (1000);
    CU_ASSERT (iot_time_msecs ()  > msecs_time);
  }
}

static void test_time_nsecs (void)
{
  volatile uint64_t nsecs_time;
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    nsecs_time = iot_time_nsecs ();
    CU_ASSERT (iot_time_nsecs () > nsecs_time);
  }
}

static void test_hash (void)
{
  uint32_t hash = iot_hash ("Dummy");
  CU_ASSERT (hash == 3802084562);
}

void cunit_misc_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("time", suite_init, suite_clean);
  CU_add_test (suite, "time_msecs", test_time_msecs);
  CU_add_test (suite, "time_nsecs", test_time_nsecs);
  CU_add_test (suite, "hash", test_hash);
}
