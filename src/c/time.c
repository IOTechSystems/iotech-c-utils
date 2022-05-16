//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/time.h"

#define IOT_TIME_NANOS_PER_MIC 1000U
#define IOT_TIME_NANOS_PER_MIL 1000000U
#define IOT_TIME_NANOS_PER_SEC 1000000000U

static inline uint64_t iot_time_nanosecs (void)
{
  struct timespec ts = { 0 };
  clock_gettime (CLOCK_REALTIME, &ts);
  return ((uint64_t) ts.tv_sec * IOT_TIME_NANOS_PER_SEC) + (uint64_t) ts.tv_nsec;
}

uint64_t iot_time_msecs (void)
{
  return iot_time_nanosecs () / IOT_TIME_NANOS_PER_MIL;
}

uint64_t iot_time_usecs (void)
{
  return iot_time_nanosecs () / IOT_TIME_NANOS_PER_MIC;
}

extern uint64_t iot_time_secs (void)
{
  struct timespec ts = { 0 };
  clock_gettime (CLOCK_REALTIME, &ts);
  return (uint64_t) ts.tv_sec;
}

uint64_t iot_time_nsecs (void)
{
  static _Atomic uint64_t lasttime = ATOMIC_VAR_INIT (0);
  uint64_t result = iot_time_nanosecs ();
  uint64_t prev = atomic_load (&lasttime);
  do
  {
    if (result <= prev) result = prev + 1;
  }
  while (!atomic_compare_exchange_weak (&lasttime, &prev, result));
  return result;
}

static void iot_wait (struct timespec * tm)
{
  struct timespec rem;
  while (nanosleep (tm, &rem) == -1 && errno == EINTR) *tm = rem;
}

void iot_wait_secs (uint64_t interval)
{
  struct timespec tm = { .tv_sec = (time_t)interval, .tv_nsec = 0 };
  iot_wait (&tm);
}

void iot_wait_msecs (uint64_t interval)
{
  struct timespec tm = { .tv_sec = (time_t) (interval / 1000), .tv_nsec = (long) (1000000 * (interval % 1000)) };
  iot_wait (&tm);
}

void iot_wait_usecs (uint64_t interval)
{
  struct timespec tm = { .tv_sec = (time_t) (interval / 1000000), .tv_nsec = (long) (1000 * (interval % 1000000)) };
  iot_wait (&tm);
}