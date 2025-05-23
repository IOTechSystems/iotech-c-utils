/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iot/threadpool.h>
#include <iot/iot.h>
#include <CUnit.h>
#include "scheduler.h"

static _Atomic uint32_t sum_test;
static _Atomic uint32_t sum_work1, sum_work2, sum_work3, sum_work5;
static _Atomic uint32_t counter;
static iot_logger_t *logger = NULL;

static void reset_counters (void)
{
  atomic_store (&sum_test, 0);
  atomic_store (&sum_work5, 0);
  atomic_store (&sum_work1, 0);
  atomic_store (&sum_work2, 0);
  atomic_store (&sum_work3, 0);
  atomic_store (&counter, 0);
}

static void * do_work1 (void * in)
{
  (void) in;
  for (uint32_t i = 0; i < 10; ++i)
  {
    atomic_fetch_add (&sum_work1, 1u);
  }
  return NULL;
}

static void * do_work2 (void * in)
{
  (void) in;
  for (uint32_t i = 0; i < 20; ++i)
  {
    atomic_fetch_add (&sum_work2, i);
  }
  return NULL;
}

static void * do_work3 (void * in)
{
  (void) in;
  for (int i = 0; i < 30; ++i)
  {
    atomic_fetch_add (&sum_work3, i);
  }
  return NULL;
}

static void * do_work4 (void * in)
{
  (void) in;
  atomic_fetch_add (&sum_test, 1u);
  return NULL;
}

static void * do_sum_100 (void * in)
{
  (void) in;
  iot_wait_msecs (100u);
  atomic_fetch_add (&sum_test, 1u);
  return NULL;
}

static void * do_work5 (void *in)
{
  (void) in;
  atomic_fetch_add (&sum_work5, 1u);
  return NULL;
}

static void * do_count (void *in)
{
  (void) in;
  atomic_fetch_add (&counter, 1u);
  return NULL;
}

static void * sched_create (void * data)
{
  *(int*) data = 10;
  return data;
}

static void sched_free (void * data)
{
  free (data);
}

static int suite_init (void)
{
  logger = iot_logger_alloc ("Test", IOT_LOG_WARN, true);
  return 0;
}

static int suite_clean (void)
{
  iot_logger_free (logger);
  return 0;
}

static void cunit_scheduler_start (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);

  reset_counters ();
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work1, NULL, NULL, IOT_MS_TO_NS (500), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (sched1 != NULL)
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work2, NULL, NULL, IOT_SEC_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (iot_schedule_id (sched1) != iot_schedule_id (sched2))
  const iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work3, NULL, NULL, IOT_SEC_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (sched3 != NULL)
  CU_ASSERT (iot_schedule_id (sched3) != iot_schedule_id (sched2))

  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))

  iot_wait_secs (2);

  CU_ASSERT (atomic_load (&sum_work1) > 0u)
  CU_ASSERT (atomic_load (&sum_work2) > 0u)
  CU_ASSERT (atomic_load (&sum_work3) == 0u)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_stop (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);

  reset_counters ();
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  sum_test = 0;

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_US_TO_NS (1000u), 0, 1, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  iot_wait_secs (2);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&sum_test) == 1u)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_create (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  reset_counters ();

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (250), 0, 1, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work5, NULL, NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);

  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  iot_scheduler_start (scheduler);

  iot_wait_secs (2);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&sum_test) == 1u)
  CU_ASSERT (atomic_load (&sum_work5) > 5u)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_remove (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (1u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  reset_counters ();

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (1), 0, 1, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work5, NULL, NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work5, NULL, NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched4 = iot_schedule_create (scheduler, do_work3, NULL, NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched5 = iot_schedule_create (scheduler, do_work3, NULL, NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched6 = iot_schedule_create (scheduler, do_work3, NULL, NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);

  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (sched3 != NULL)
  CU_ASSERT (sched4 != NULL)
  CU_ASSERT (sched5 != NULL)
  CU_ASSERT (sched6 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  CU_ASSERT (iot_schedule_add (scheduler, sched3))
  CU_ASSERT (iot_schedule_add (scheduler, sched4))
  CU_ASSERT (iot_schedule_add (scheduler, sched5))
  CU_ASSERT (iot_schedule_add (scheduler, sched6))

  iot_wait_secs (1);

  iot_schedule_remove (scheduler, sched2);
  iot_schedule_remove (scheduler, sched3);
  CU_ASSERT (atomic_load (&sum_test) == 1u)
  CU_ASSERT (atomic_load (&sum_work5) > 20u)

  uint32_t temp = atomic_load (&sum_work5);
  iot_wait_secs (1);

  CU_ASSERT (temp <= (atomic_load (&sum_work5) + 2u))

  iot_schedule_delete (scheduler, sched3);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_delete (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);

  iot_threadpool_start (pool);
  reset_counters ();

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work3, NULL,NULL, IOT_MS_TO_NS (1), 0, 1, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work4, NULL,NULL, IOT_MS_TO_NS (1), 0, 1, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work3, NULL,NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched4 = iot_schedule_create (scheduler, do_work3, NULL,NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched5 = iot_schedule_create (scheduler, do_work5, NULL,NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched6 = iot_schedule_create (scheduler, do_work3, NULL,NULL, IOT_MS_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);

  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (sched3 != NULL)
  CU_ASSERT (sched4 != NULL)
  CU_ASSERT (sched5 != NULL)
  CU_ASSERT (sched6 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  CU_ASSERT (iot_schedule_add (scheduler, sched3))
  CU_ASSERT (iot_schedule_add (scheduler, sched4))
  CU_ASSERT (iot_schedule_add (scheduler, sched5))
  CU_ASSERT (iot_schedule_add (scheduler, sched6))

  iot_scheduler_start (scheduler);
  iot_wait_secs (1);

  iot_schedule_delete (scheduler, sched5);
  iot_scheduler_stop (scheduler);
  iot_threadpool_wait (pool);

  CU_ASSERT (atomic_load (&sum_test) == 1u)
  CU_ASSERT (atomic_load (&sum_work5) > 20u)

  uint32_t temp = atomic_load (&sum_work5);
  iot_scheduler_start (scheduler);
  iot_wait_secs (1);

  CU_ASSERT (temp == atomic_load (&sum_work5))
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_refcount (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_add_ref (scheduler);
  iot_scheduler_free (scheduler);
  iot_scheduler_free (scheduler);
  iot_scheduler_free (NULL);
  iot_threadpool_free (pool);
}

static void cunit_scheduler_nrepeat (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  CU_ASSERT (scheduler != NULL)

  reset_counters ();
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_count, NULL,NULL, IOT_MS_TO_NS (100), 0, 5, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  iot_wait_secs (2);

  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&counter) == 5u)
  CU_ASSERT (iot_schedule_dropped (sched1) == 0)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_starttime (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  CU_ASSERT (scheduler != NULL)

  reset_counters ();
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL,NULL, IOT_MS_TO_NS (100), IOT_MS_TO_NS (100), 1, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  iot_wait_secs (2);

  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&sum_test) == 1)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_reset (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  CU_ASSERT (scheduler != NULL)

  reset_counters ();
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL,NULL, IOT_SEC_TO_NS (2u), IOT_MS_TO_NS (100u), 10u, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  iot_wait_secs (1u);
  iot_schedule_reset (scheduler, sched1, 0u);
  iot_wait_secs (1u);
  iot_schedule_reset (scheduler, sched1, 0u);
  iot_wait_secs (1u);
  iot_schedule_reset (scheduler, sched1, IOT_SEC_TO_NS (2u));
  iot_wait_secs (3u);

  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&sum_test) == 1u)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_setpriority (void)
{
  int prio_max = sched_get_priority_max (SCHED_FIFO);
  int prio_min = sched_get_priority_min (SCHED_FIFO);
  iot_threadpool_t *pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  CU_ASSERT (scheduler != NULL)

  reset_counters ();
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_count, NULL, NULL, IOT_MS_TO_NS (10), 0, 100, pool, prio_max);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (1000), IOT_MS_TO_NS (1000), 5, pool, prio_min);
  CU_ASSERT (iot_schedule_add (scheduler, sched2))

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  iot_wait_secs (2);

  iot_scheduler_stop (scheduler);

  CU_ASSERT (atomic_load (&counter) == 100u)
  CU_ASSERT (atomic_load (&sum_test) >= 1u)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_setfreefn (void)
{
  int prio_max = sched_get_priority_max (SCHED_FIFO);
  iot_threadpool_t *pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  CU_ASSERT (scheduler != NULL)

  void *test_arg = malloc (sizeof (int));

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, sched_create, sched_free, test_arg, IOT_MS_TO_NS (10), 0, 1, pool, prio_max);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  iot_wait_secs (2);

  iot_scheduler_stop (scheduler);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_rate (bool concurrent, bool sync)
{
  reset_counters ();
  iot_threadpool_t *pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_sum_100, NULL, NULL, IOT_MS_TO_NS (50u), 0, 0, pool, -1);
  iot_schedule_set_concurrent (sched, concurrent);
  if (concurrent && sync)
  {
    CU_ASSERT (! iot_schedule_set_sync (sched, sync))
    sync = false;
  }
  CU_ASSERT (iot_schedule_set_sync (sched, sync))
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  iot_wait_secs (2);
  iot_scheduler_stop (scheduler);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
  uint32_t sum = atomic_load (&sum_test);
  if (concurrent)
  {
    CU_ASSERT (sum > 21u)
  }
  else
  {
    CU_ASSERT (sum <= 21u)
  }
}

static void cunit_scheduler_concurrent (void)
{
  cunit_scheduler_rate (true, false);
}

static void cunit_scheduler_serialized (void)
{
  cunit_scheduler_rate (false, false);
}

static void cunit_scheduler_sync (void)
{
  cunit_scheduler_rate (false, true);
}

static void * scheduler_delete_with_user_data_do_work (void * arg)
{
  iot_wait_secs (2); //do some work
  uint64_t *arg2 = arg;
  (*arg2)++;
  return NULL;
}

static void scheduler_delete_with_user_data_free (void *data)
{
  free (data);
}

static void cunit_scheduler_delete_with_user_data (void)
{
  uint64_t *arg = calloc (1, sizeof(*arg));
  iot_scheduler_t * scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, NULL);
  iot_schedule_t * sched1 = iot_schedule_create
  (
    scheduler, scheduler_delete_with_user_data_do_work, scheduler_delete_with_user_data_free,
    arg, 10, 0, 0, NULL, -1
  );
  CU_ASSERT_TRUE (iot_schedule_add (scheduler, sched1))
  iot_scheduler_start (scheduler);
  iot_wait_secs (1);
  iot_schedule_delete (scheduler, sched1);
  iot_wait_secs (2);
  iot_scheduler_stop (scheduler);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_random (void)
{
  iot_schedule_t *sched[10];
  reset_counters ();
  iot_threadpool_t *pool = iot_threadpool_alloc (1u, 1u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  for (unsigned i = 0; i < 10; i++)
  {
    sched[i] = iot_schedule_create (scheduler, do_sum_100, NULL, NULL, IOT_SEC_TO_NS (10u), 0, 0, pool, -1);
  }
  for (unsigned i = 0; i < 10; i++)
  {
    CU_ASSERT (iot_schedule_add_randomised (scheduler, sched[i], IOT_SEC_TO_NS (2u)));
  }
  iot_wait_secs (9);
  iot_scheduler_stop (scheduler);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
  CU_ASSERT (atomic_load (&sum_test) >= 9u)   // Allow for one unlucky collision
}

extern void cunit_scheduler_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("scheduler", suite_init, suite_clean);
  CU_add_test (suite, "scheduler_start", cunit_scheduler_start);
  CU_add_test (suite, "scheduler_stop", cunit_scheduler_stop);
  CU_add_test (suite, "scheduler_create", cunit_scheduler_create);
  CU_add_test (suite, "scheduler_remove", cunit_scheduler_remove);
  CU_add_test (suite, "scheduler_delete", cunit_scheduler_delete);
  CU_add_test (suite, "scheduler_refcount", cunit_scheduler_refcount);
  CU_add_test (suite, "scheduler_nrepeat", cunit_scheduler_nrepeat);
  CU_add_test (suite, "scheduler_starttime", cunit_scheduler_starttime);
  CU_add_test (suite, "scheduler_random", cunit_scheduler_random);
  CU_add_test (suite, "scheduler_reset", cunit_scheduler_reset);
  CU_add_test (suite, "scheduler_setpriority", cunit_scheduler_setpriority);
  CU_add_test (suite, "scheduler_freefn", cunit_scheduler_setfreefn);
  CU_add_test (suite, "scheduler_concurrent", cunit_scheduler_concurrent);
  CU_add_test (suite, "scheduler_sync", cunit_scheduler_sync);
  CU_add_test (suite, "scheduler_serialized", cunit_scheduler_serialized);
  CU_add_test (suite, "scheduler_delete_with_user_data", cunit_scheduler_delete_with_user_data);
}
