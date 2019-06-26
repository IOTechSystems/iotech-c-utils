//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/time.h"

#define IOT_TIME_MILLIS 1000U
#define IOT_TIME_NANOS 1000000000U

static uint64_t iot_time_nanosecs (void)
{
  struct timespec ts;
  return ((clock_gettime (CLOCK_REALTIME, &ts) == 0) ? ((uint64_t) ts.tv_sec * IOT_TIME_NANOS + ts.tv_nsec) : ((uint64_t) time (NULL) * IOT_TIME_NANOS));
}

uint64_t iot_time_msecs ()
{
  return iot_time_nanosecs () / (IOT_TIME_NANOS / IOT_TIME_MILLIS);
}

uint64_t iot_time_nsecs ()
{
  static atomic_uint_fast64_t lasttime = 0;
  uint64_t prev;
  uint64_t result = iot_time_nanosecs ();
  prev = lasttime;
  do
  {
    if (result <= prev)
    {
      result = prev + 1;
    }
  } while (!atomic_compare_exchange_weak (&lasttime, &prev, result));
  return result;
}
