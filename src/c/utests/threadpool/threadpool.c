/*
 * Copyright (c) 2020-2026
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threadpool.h"
#include "iot/thread.h"
#include "iot/time.h"
#include "iot/component.h"
#include "CUnit.h"

static int prio_min = -1;
static int prio_max = -1;
static int prio1 = -1;
static int prio2 = -1;
static int prio3 = -1;
static int counter_max = 0;
static uint32_t counter = 0;
static iot_logger_t * logger = NULL;

static int suite_init (void)
{
  prio_max = sched_get_priority_max (SCHED_FIFO);
  prio_min = sched_get_priority_min (SCHED_FIFO);
  printf ("\nFIFO priority max: %d min: %d\n", prio_max, prio_min);
  logger = iot_logger_alloc ("ThreadPool", IOT_LOG_WARN, true);
  return 0;
}

static int suite_clean (void)
{
  iot_logger_free (logger);
  return 0;
}

static void * cunit_pool_sleeper (void * arg)
{
  (void) arg;
  iot_wait_secs (1u);
  return NULL;
}

static void * cunit_pool_blocker (void * arg)
{
  pthread_mutex_t * mutex = (pthread_mutex_t*) arg;
  pthread_mutex_lock (mutex);
  pthread_mutex_unlock (mutex);
  return NULL;
}

static void * cunit_pool_sole_counter (void * arg)
{
  (void) arg;
  counter++;
  if (counter > counter_max) counter_max = counter;
  iot_wait_msecs (500u);
  CU_ASSERT (counter == 1)
  counter--;
  return NULL;
}

static void * cunit_pool_counter (void * arg)
{
  (void) arg;
  counter++;
  return NULL;
}

typedef struct cunit_pool_waiter_t
{
  iot_threadpool_t * pool;
  _Atomic bool done;
} cunit_pool_waiter_t;

static void * cunit_pool_waiter (void * arg)
{
  cunit_pool_waiter_t * waiter = (cunit_pool_waiter_t*) arg;
  iot_threadpool_wait (waiter->pool);
  atomic_store (&waiter->done, true);
  return NULL;
}

static void * cunit_pool_prio_worker (void * arg)
{
  int prio = *((int*) arg);
  int current = iot_thread_current_get_priority ();
  if (prio != current)
  {
    printf ("**** Is CAP_SYS_NICE set for runner ??? **** prio %d != %d\n", current, prio);
  }
  CU_ASSERT (prio == iot_thread_current_get_priority ())
  return NULL;
}

static void * cunit_pool_prio_reader (void * arg)
{
  *((int*) arg) = iot_thread_current_get_priority ();
  return NULL;
}

static void cunit_threadpool_priority_range (void)
{
  CU_ASSERT (prio_max > prio_min)
  iot_thread_current_set_priority (prio_min + 1);
  int prio = iot_thread_current_get_priority ();
  CU_ASSERT (prio == (prio_min + 1))
}

static void cunit_threadpool_priority (void)
{
  prio1 = prio_min + 1;
  prio2 = prio1 + 1;
  prio3 = prio2 + 1;
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 0u, prio_min, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_sleeper, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio1, prio1);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio2, prio2);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio3, prio3);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio2, prio2);
  iot_threadpool_wait (pool);
  iot_threadpool_free (pool);
}

static void cunit_threadpool_priority_restore (void)
{
  int before = -1;
  int during = -1;
  int after = -1;
  int prio = prio_min + 2;
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 0u, prio_min, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_prio_reader, &before, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  iot_threadpool_add_work (pool, cunit_pool_prio_reader, &during, prio);
  iot_threadpool_wait (pool);
  iot_threadpool_add_work (pool, cunit_pool_prio_reader, &after, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  if (during != prio)
  {
    printf ("**** Is CAP_SYS_NICE set for runner ??? **** prio %d != %d\n", during, prio);
  }
  CU_ASSERT (after == before) // Default priority restored for job with no priority
  iot_threadpool_free (pool);
}

static void cunit_threadpool_try_work (void)
{
  bool ret;
  pthread_mutex_t mutex;
  pthread_mutex_init (&mutex, NULL);
  pthread_mutex_lock (&mutex);
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 2u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_blocker, &mutex, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_add_work (pool, cunit_pool_blocker, &mutex, IOT_THREAD_NO_PRIORITY);
  iot_wait_msecs (100u);
  ret = iot_threadpool_try_work (pool, cunit_pool_blocker, &mutex, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (ret)
  ret = iot_threadpool_try_work (pool, cunit_pool_blocker, &mutex, IOT_THREAD_NO_PRIORITY);
  CU_ASSERT (! ret)
  pthread_mutex_unlock (&mutex);
  iot_threadpool_wait (pool);
  iot_threadpool_free (pool);
  pthread_mutex_destroy (&mutex);
}

static void cunit_threadpool_block (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 1u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_sole_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_add_work (pool, cunit_pool_sole_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_add_work (pool, cunit_pool_sole_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_add_work (pool, cunit_pool_sole_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  CU_ASSERT (counter_max == 1)
  iot_threadpool_free (pool);
}

static void cunit_threadpool_stop_start (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  counter = 0;
  iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_start (pool);
  iot_threadpool_wait (pool);
  CU_ASSERT (counter == 1)
  iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  CU_ASSERT (counter == 2)
  iot_threadpool_stop (pool);
  iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_wait_msecs (500u);
  CU_ASSERT (counter == 2)
  iot_threadpool_start (pool);
  iot_threadpool_wait (pool);
  CU_ASSERT (counter == 3)
  iot_threadpool_stop (pool);
  iot_threadpool_free (pool);
}

static void cunit_threadpool_refcount (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_add_ref (pool);
  iot_threadpool_free (pool);
  iot_wait_msecs (500u);
  iot_threadpool_free (pool);
  iot_threadpool_free (NULL);
}

static void cunit_threadpool_multi_wait (void)
{
  pthread_t tid;
  cunit_pool_waiter_t waiters[3];
  pthread_mutex_t mutex;
  pthread_mutex_init (&mutex, NULL);
  pthread_mutex_lock (&mutex);
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_blocker, &mutex, IOT_THREAD_NO_PRIORITY);
  iot_wait_msecs (100u); // Ensure job running before waiters start
  for (int i = 0; i < 3; i++)
  {
    waiters[i].pool = pool;
    atomic_store (&waiters[i].done, false);
    CU_ASSERT (iot_thread_create (&tid, cunit_pool_waiter, &waiters[i], IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger))
  }
  iot_wait_msecs (100u); // Ensure all waiters blocked
  pthread_mutex_unlock (&mutex);
  for (int t = 0; t < 20; t++) // Allow up to 2 seconds for all waiters to complete
  {
    if (atomic_load (&waiters[0].done) && atomic_load (&waiters[1].done) && atomic_load (&waiters[2].done)) break;
    iot_wait_msecs (100u);
  }
  for (int i = 0; i < 3; i++)
  {
    CU_ASSERT (atomic_load (&waiters[i].done))
  }
  for (int i = 0; i < 3; i++) // On failure, release any stuck waiters before freeing pool
  {
    while (! atomic_load (&waiters[i].done))
    {
      iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
      iot_wait_msecs (10u);
    }
  }
  iot_threadpool_free (pool);
  pthread_mutex_destroy (&mutex);
}

static void cunit_threadpool_jobs_max (void)
{
  const uint32_t njobs = 70000u; // Exceeds UINT16_MAX
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_component_t * comp = (iot_component_t*) pool;
  counter = 0;
  for (uint32_t i = 0; i < njobs; i++) // Queue jobs with pool stopped
  {
    iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  }
  iot_data_t * stats = comp->stats_fn (comp);
  CU_ASSERT (iot_data_ui32 (iot_data_string_map_get (stats, "jobs")) == njobs)
  CU_ASSERT (iot_data_ui32 (iot_data_string_map_get (stats, "jobs_max")) == njobs)
  iot_data_free (stats);
  iot_threadpool_start (pool);
  iot_threadpool_wait (pool);
  CU_ASSERT (counter == njobs)
  stats = comp->stats_fn (comp);
  CU_ASSERT (iot_data_ui32 (iot_data_string_map_get (stats, "jobs")) == 0u)
  CU_ASSERT (iot_data_ui32 (iot_data_string_map_get (stats, "jobs_max")) == njobs)
  iot_data_free (stats);
  iot_threadpool_free (pool);
}

static void cunit_threadpool_free_delay (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  uint64_t start = iot_time_msecs ();
  iot_threadpool_free (pool);
  uint64_t elapsed = iot_time_msecs () - start;
  CU_ASSERT (elapsed < 100u) // Free should not be delayed by polling
}

#define JOIN_THREADS 4u
#define JOIN_EXIT_DELAY_US 50000u

static pthread_key_t join_key;
static pthread_barrier_t join_barrier;
static atomic_uint join_exited;

static void cunit_join_key_destructor (void * arg)
{
  (void) arg;
  iot_wait_usecs (JOIN_EXIT_DELAY_US); // Delays thread termination until after thread exit signalled to pool
  atomic_fetch_add (&join_exited, 1u);
}

static void * cunit_join_job (void * arg)
{
  (void) arg;
  pthread_barrier_wait (&join_barrier); // So each job runs on a different pool thread
  pthread_setspecific (join_key, &join_key); // Destructor called on thread termination
  return NULL;
}

static void cunit_threadpool_free_join (void)
{
  // When free returns, all pool threads have terminated
  pthread_key_create (&join_key, cunit_join_key_destructor);
  pthread_barrier_init (&join_barrier, NULL, JOIN_THREADS);
  atomic_store (&join_exited, 0u);
  iot_threadpool_t * pool = iot_threadpool_alloc (JOIN_THREADS, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  for (unsigned i = 0; i < JOIN_THREADS; i++) iot_threadpool_add_work (pool, cunit_join_job, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  iot_threadpool_free (pool);
  CU_ASSERT (atomic_load (&join_exited) == JOIN_THREADS)
  for (unsigned i = 0; i < 100u && atomic_load (&join_exited) != JOIN_THREADS; i++) iot_wait_usecs (JOIN_EXIT_DELAY_US); // Before deleting key
  pthread_barrier_destroy (&join_barrier);
  pthread_key_delete (join_key);
}

static atomic_bool self_free_done;

static void * cunit_self_free_job (void * arg)
{
  iot_threadpool_free ((iot_threadpool_t*) arg);
  atomic_store (&self_free_done, true);
  return NULL;
}

static void cunit_threadpool_free_self (void)
{
  // Pool freed by one of its own jobs (thread cannot join itself, so detaches, and pool freed when it exits)
  atomic_store (&self_free_done, false);
  iot_threadpool_t * pool = iot_threadpool_alloc (JOIN_THREADS, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_self_free_job, pool, IOT_THREAD_NO_PRIORITY);
  for (unsigned i = 0; i < 500u && ! atomic_load (&self_free_done); i++) iot_wait_usecs (10000u);
  CU_ASSERT (atomic_load (&self_free_done))
  iot_wait_usecs (100000u); // Allow detached thread to exit and free pool (checked by valgrind)
}

static void cunit_threadpool_free_queued (void)
{
  // Jobs still queued when a stopped pool is freed are discarded, not run (checked by valgrind)
  counter = 0u;
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  for (unsigned i = 0; i < 5u; i++) iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_free (pool);
  CU_ASSERT (counter == 0u)
}

static void cunit_threadpool_affinity (void)
{
  // Pool threads restricted to a processor run jobs
  counter = 0u;
  iot_threadpool_t * pool = iot_threadpool_alloc (2u, 0u, IOT_THREAD_NO_PRIORITY, 0, logger);
  iot_threadpool_start (pool);
  for (unsigned i = 0; i < 4u; i++) iot_threadpool_add_work (pool, cunit_pool_counter, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  CU_ASSERT (counter == 4u)
  iot_threadpool_free (pool);
}

void cunit_threadpool_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("threadpool", suite_init, suite_clean);
  CU_add_test (suite, "threadpool_priority_range", cunit_threadpool_priority_range);
  CU_add_test (suite, "threadpool_priority", cunit_threadpool_priority);
  CU_add_test (suite, "threadpool_priority_restore", cunit_threadpool_priority_restore);
  CU_add_test (suite, "threadpool_block", cunit_threadpool_block);
  CU_add_test (suite, "threadpool_try_work", cunit_threadpool_try_work);
  CU_add_test (suite, "threadpool_stop_start", cunit_threadpool_stop_start);
  CU_add_test (suite, "threadpool_refcount", cunit_threadpool_refcount);
  CU_add_test (suite, "threadpool_multi_wait", cunit_threadpool_multi_wait);
  CU_add_test (suite, "threadpool_jobs_max", cunit_threadpool_jobs_max);
  CU_add_test (suite, "threadpool_free_delay", cunit_threadpool_free_delay);
  CU_add_test (suite, "threadpool_free_join", cunit_threadpool_free_join);
  CU_add_test (suite, "threadpool_free_self", cunit_threadpool_free_self);
  CU_add_test (suite, "threadpool_free_queued", cunit_threadpool_free_queued);
  CU_add_test (suite, "threadpool_affinity", cunit_threadpool_affinity);
}
