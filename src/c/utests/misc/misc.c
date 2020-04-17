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
  CU_ASSERT ( iot_hash ("Dummy") == 3802084562)
  CU_ASSERT ( iot_hash ("int8") == 636741937)
  CU_ASSERT ( iot_hash ("int8array") == 3352350152)
  CU_ASSERT ( iot_hash ("uint8") == 3827140740)
  CU_ASSERT ( iot_hash ("uint8array") == 1993633373)
  CU_ASSERT ( iot_hash ("int16") == 3832614926)
  CU_ASSERT ( iot_hash ("int16array") == 3293575959)
  CU_ASSERT ( iot_hash ("uint16") == 1741593115)
  CU_ASSERT ( iot_hash ("uint16array") == 3942928418)
  CU_ASSERT ( iot_hash ("int32") == 3832614984)
  CU_ASSERT ( iot_hash ("int32array") == 1264005137)
  CU_ASSERT ( iot_hash ("uint32") == 1741593181)
  CU_ASSERT ( iot_hash ("uint32array") == 2235632164)
  CU_ASSERT ( iot_hash ("int64") == 3832615211)
  CU_ASSERT ( iot_hash ("int64array") == 1709245714)
  CU_ASSERT ( iot_hash ("uint64") == 1741593086)
  CU_ASSERT ( iot_hash ("uint64array") == 2964325927)
  CU_ASSERT ( iot_hash ("float32") == 2863538027)
  CU_ASSERT ( iot_hash ("float32array") == 3463494610)
  CU_ASSERT ( iot_hash ("float64") == 2863538056)
  CU_ASSERT ( iot_hash ("float64array") == 464521169)
  CU_ASSERT ( iot_hash ("bool") == 636838452)
  CU_ASSERT ( iot_hash ("boolarray") == 1007050925)
  CU_ASSERT ( iot_hash ("string") == 1386610095)
  CU_ASSERT ( iot_hash ("binary") == 2016023253)
}

void cunit_misc_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("time", suite_init, suite_clean);
  CU_add_test (suite, "time_secs", test_time_secs);
  CU_add_test (suite, "time_msecs", test_time_msecs);
  CU_add_test (suite, "time_nsecs", test_time_nsecs);
  CU_add_test (suite, "hash", test_hash);
}
