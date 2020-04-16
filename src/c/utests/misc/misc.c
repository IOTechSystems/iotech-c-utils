/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/iot.h"
#include "misc.h"
#include "CUnit.h"

#define MAX_COUNTER 1000
#define MAX_SECS_COUNTER 4

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void test_time_secs (void)
{
  volatile uint64_t secs_time;
  for (int counter = 0; counter < MAX_SECS_COUNTER; counter++)
  {
    secs_time = iot_time_secs ();
    usleep (1000000);
    CU_ASSERT (iot_time_secs ()  > secs_time)
  }
}

static void test_time_msecs (void)
{
  volatile uint64_t msecs_time;
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    msecs_time = iot_time_msecs ();
    usleep (1000);
    CU_ASSERT (iot_time_msecs ()  > msecs_time)
  }
}

static void test_time_nsecs (void)
{
  unsigned counter;
  uint64_t times[MAX_COUNTER];
  for (counter = 0; counter < MAX_COUNTER; counter++)
  {
    times[counter] = iot_time_nsecs ();
  }
  for (counter = 0; counter < (MAX_COUNTER - 1); counter++)
  {
    CU_ASSERT (times[counter] < times[counter + 1])
  }
}

static void test_hash (void)
{
  uint32_t hash = iot_hash ("Dummy");
  CU_ASSERT (hash == 3802084562)
}

void cunit_misc_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("time", suite_init, suite_clean);
  CU_add_test (suite, "time_secs", test_time_secs);
  CU_add_test (suite, "time_msecs", test_time_msecs);
  CU_add_test (suite, "time_nsecs", test_time_nsecs);
  CU_add_test (suite, "hash", test_hash);
}
