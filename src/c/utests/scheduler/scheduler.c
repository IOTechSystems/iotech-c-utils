/*
 * Copyright (c) 2020-2026
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

static void * do_block (void * in)
{
  pthread_mutex_t * mutex = (pthread_mutex_t*) in;
  pthread_mutex_lock (mutex);
  pthread_mutex_unlock (mutex);
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

static _Atomic bool run_cb_started;
static _Atomic bool run_cb_done;

static void * slow_run_cb (void * arg)
{
  (void) arg;
  atomic_store (&run_cb_started, true);
  iot_wait_msecs (200u);
  atomic_store (&run_cb_done, true);
  return NULL;
}

static uint64_t scheduler_stat (iot_scheduler_t * scheduler, const char * name)
{
  iot_component_t * comp = (iot_component_t*) scheduler;
  iot_data_t * stats = comp->stats_fn (comp);
  const iot_data_t * value = iot_data_string_map_get (stats, name);
  uint64_t result = (iot_data_type (value) == IOT_DATA_UINT64) ? iot_data_ui64 (value) : iot_data_ui32 (value);
  iot_data_free (stats);
  return result;
}

static void cunit_scheduler_free_wait (void)
{
  atomic_store (&run_cb_started, false);
  atomic_store (&run_cb_done, false);
  iot_threadpool_t *pool = iot_threadpool_alloc (1u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_SEC_TO_NS (1u), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_add_run_callback (scheduler, sched, slow_run_cb);
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  for (int i = 0; i < 100 && ! atomic_load (&run_cb_started); i++) iot_wait_msecs (10u);
  CU_ASSERT (atomic_load (&run_cb_started))
  iot_scheduler_free (scheduler); // Must wait for scheduler thread, which is in run callback
  CU_ASSERT (atomic_load (&run_cb_done))
  iot_threadpool_free (pool);
}

static void cunit_scheduler_randomised_queued (void)
{
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_SEC_TO_NS (1u), IOT_HOUR_TO_NS (10u), 0, NULL, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_SEC_TO_NS (1u), IOT_HOUR_TO_NS (20u), 0, NULL, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_SEC_TO_NS (1u), IOT_HOUR_TO_NS (30u), 0, NULL, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  CU_ASSERT (iot_schedule_add (scheduler, sched3))
  CU_ASSERT (! iot_schedule_add_randomised (scheduler, sched3, 0u)) // Already queued, so must not change queue key
  CU_ASSERT (iot_schedule_remove (scheduler, sched3))
  CU_ASSERT (scheduler_stat (scheduler, "active_queue_size") == 2u)
  CU_ASSERT (scheduler_stat (scheduler, "idle_queue_size") == 1u)
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_dropped_count (void)
{
  pthread_mutex_t mutex;
  pthread_mutex_init (&mutex, NULL);
  pthread_mutex_lock (&mutex);
  iot_threadpool_t *pool = iot_threadpool_alloc (1u, 1u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, do_block, &mutex, IOT_THREAD_NO_PRIORITY); // Occupy pool thread
  iot_wait_msecs (50u);
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (20u), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_set_concurrent (sched, true); // So runs are dropped (pool full) rather than skipped (already running)
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  iot_scheduler_start (scheduler);
  iot_wait_msecs (300u);
  iot_scheduler_stop (scheduler);
  uint64_t dropped = iot_schedule_dropped (sched);
  CU_ASSERT (dropped > 1u)
  CU_ASSERT (scheduler_stat (scheduler, "dropped_runs") == dropped) // Every dropped run counted
  pthread_mutex_unlock (&mutex);
  iot_threadpool_wait (pool);
  iot_scheduler_free (scheduler);
  iot_threadpool_free (pool);
  pthread_mutex_destroy (&mutex);
}

static void cunit_scheduler_thread_runs (void)
{
  reset_counters ();
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (50u), 0, 3u, NULL, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  iot_scheduler_start (scheduler);
  iot_wait_msecs (500u);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&sum_test) == 3u)
  CU_ASSERT (scheduler_stat (scheduler, "schedule_runs") == 3u) // Runs in new thread are counted
  iot_scheduler_free (scheduler);
}

static void * state_waiter (void * arg)
{
  iot_component_wait ((iot_component_t*) arg, IOT_COMPONENT_STOPPED);
  return NULL;
}

static void cunit_scheduler_shared_wait (void)
{
  pthread_t waiter;
  reset_counters ();
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_start (scheduler);
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work1, NULL, NULL, IOT_HOUR_TO_NS (1u), IOT_HOUR_TO_NS (2u), 0, NULL, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  iot_wait_msecs (100u); // Scheduler thread waiting for schedule
  pthread_create (&waiter, NULL, state_waiter, scheduler); // Other thread waiting on component state
  iot_wait_msecs (100u);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work1, NULL, NULL, IOT_HOUR_TO_NS (1u), IOT_HOUR_TO_NS (1u), 0, NULL, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  iot_wait_msecs (100u);
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_HOUR_TO_NS (1u), IOT_MS_TO_NS (100u), 0, NULL, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched3))
  for (int i = 0; i < 100 && atomic_load (&sum_test) == 0u; i++) iot_wait_msecs (10u);
  CU_ASSERT (atomic_load (&sum_test) == 1u) // Scheduler thread woken despite other waiter
  iot_scheduler_stop (scheduler);
  pthread_join (waiter, NULL);
  iot_scheduler_free (scheduler);
}

static _Atomic uint32_t abort_count;

static void * abort_cb (void * arg)
{
  (void) arg;
  atomic_fetch_add (&abort_count, 1u);
  return NULL;
}

static void cunit_scheduler_abort_callback (void)
{
  // Abort callback called for each dropped run
  pthread_mutex_t mutex;
  pthread_mutex_init (&mutex, NULL);
  pthread_mutex_lock (&mutex);
  atomic_store (&abort_count, 0u);
  iot_threadpool_t *pool = iot_threadpool_alloc (1u, 1u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, do_block, &mutex, IOT_THREAD_NO_PRIORITY); // Occupy pool thread
  iot_wait_msecs (50u);
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (20u), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_set_concurrent (sched, true);
  iot_schedule_add_abort_callback (scheduler, sched, abort_cb);
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  iot_scheduler_start (scheduler);
  iot_wait_msecs (300u);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (atomic_load (&abort_count) > 1u)
  CU_ASSERT (atomic_load (&abort_count) == iot_schedule_dropped (sched))
  pthread_mutex_unlock (&mutex);
  iot_threadpool_wait (pool);
  iot_scheduler_free (scheduler);
  iot_threadpool_free (pool);
  pthread_mutex_destroy (&mutex);
}

static iot_scheduler_t * self_delete_scheduler;
static iot_schedule_t * self_delete_schedule;

static void * self_delete_run_cb (void * arg)
{
  (void) arg;
  iot_schedule_delete (self_delete_scheduler, self_delete_schedule);
  return NULL;
}

static void cunit_scheduler_delete_in_run_callback (void)
{
  // Schedule deleted by its run callback runs once, and is not requeued
  reset_counters ();
  self_delete_scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  self_delete_schedule = iot_schedule_create (self_delete_scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (20u), 0, 0, NULL, IOT_THREAD_NO_PRIORITY);
  iot_schedule_set_sync (self_delete_schedule, true);
  iot_schedule_add_run_callback (self_delete_scheduler, self_delete_schedule, self_delete_run_cb);
  CU_ASSERT (iot_schedule_add (self_delete_scheduler, self_delete_schedule))
  iot_scheduler_start (self_delete_scheduler);
  iot_wait_msecs (200u);
  iot_scheduler_stop (self_delete_scheduler);
  CU_ASSERT (atomic_load (&sum_test) == 1u)
  CU_ASSERT (scheduler_stat (self_delete_scheduler, "active_queue_size") == 0u)
  CU_ASSERT (scheduler_stat (self_delete_scheduler, "idle_queue_size") == 0u)
  iot_scheduler_free (self_delete_scheduler);
}

static void cunit_scheduler_reset_unscheduled (void)
{
  // Reset of a schedule not in the run queue updates its start time, so it runs after the reset delay when added
  reset_counters ();
  iot_scheduler_t *scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (10u), 0, 1u, NULL, IOT_THREAD_NO_PRIORITY);
  iot_schedule_reset (scheduler, sched, IOT_HOUR_TO_NS (1u));
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  iot_scheduler_start (scheduler);
  iot_wait_msecs (200u);
  CU_ASSERT (atomic_load (&sum_test) == 0u)
  iot_scheduler_stop (scheduler);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_config (void)
{
  // Scheduler and thread pool created from configuration, using configured logger
  iot_component_factory_add (iot_logger_factory ());
  iot_component_factory_add (iot_scheduler_factory ());
  iot_component_factory_add (iot_threadpool_factory ());
  iot_container_t * cont = iot_container_alloc ("sched_config");
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", "{\"Name\":\"logger\",\"Level\":\"Warn\"}");
  iot_container_add_component (cont, IOT_THREADPOOL_TYPE, "pool", "{\"Threads\":2,\"MaxJobs\":4,\"Logger\":\"logger\"}");
  iot_container_add_component (cont, IOT_SCHEDULER_TYPE, "scheduler", "{\"Logger\":\"logger\"}");
  iot_threadpool_t * pool = (iot_threadpool_t*) iot_container_find_component (cont, "pool");
  iot_scheduler_t * scheduler = (iot_scheduler_t*) iot_container_find_component (cont, "scheduler");
  CU_ASSERT_FATAL (pool && scheduler)
  iot_container_start (cont);
  reset_counters ();
  iot_schedule_t *sched = iot_schedule_create (scheduler, do_work4, NULL, NULL, IOT_MS_TO_NS (10u), 0, 3u, pool, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (iot_schedule_add (scheduler, sched))
  for (int i = 0; i < 100 && atomic_load (&sum_test) < 3u; i++) iot_wait_msecs (10u);
  CU_ASSERT (atomic_load (&sum_test) == 3u)
  iot_data_t * stats = iot_component_stats ((iot_component_t*) pool);
  const iot_data_t * max_jobs = iot_data_string_map_get (iot_data_string_map_get (stats, "stats"), "max_jobs");
  CU_ASSERT (max_jobs && iot_data_ui32 (max_jobs) == 4u)
  iot_data_free (stats);
  iot_container_free (cont);
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
  CU_add_test (suite, "scheduler_free_wait", cunit_scheduler_free_wait);
  CU_add_test (suite, "scheduler_randomised_queued", cunit_scheduler_randomised_queued);
  CU_add_test (suite, "scheduler_dropped_count", cunit_scheduler_dropped_count);
  CU_add_test (suite, "scheduler_thread_runs", cunit_scheduler_thread_runs);
  CU_add_test (suite, "scheduler_shared_wait", cunit_scheduler_shared_wait);
  CU_add_test (suite, "scheduler_abort_callback", cunit_scheduler_abort_callback);
  CU_add_test (suite, "scheduler_delete_in_run_callback", cunit_scheduler_delete_in_run_callback);
  CU_add_test (suite, "scheduler_reset_unscheduled", cunit_scheduler_reset_unscheduled);
  CU_add_test (suite, "scheduler_config", cunit_scheduler_config);
}
