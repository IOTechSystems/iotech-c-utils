#include "threadpool.h"
#include "iot/thread.h"
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
  sleep (1);
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
  usleep (500000);
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

static void * cunit_pool_prio_worker (void * arg)
{
  int prio = *((int*) arg);
  int current = iot_thread_current_get_priority ();
  if (prio != current)
  {
    printf ("\n**** Is CAP_SYS_NICE set for runner ??? **** prio %d != %d\n", current, prio);
  }
  CU_ASSERT (prio == iot_thread_current_get_priority ())
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
  usleep (100000);
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
  usleep (500000);
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
  usleep (500000);
  iot_threadpool_free (pool);
  iot_threadpool_free (NULL);
}

void cunit_threadpool_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("threadpool", suite_init, suite_clean);
  CU_add_test (suite, "threadpool_priority_range", cunit_threadpool_priority_range);
  CU_add_test (suite, "threadpool_priority", cunit_threadpool_priority);
  CU_add_test (suite, "threadpool_block", cunit_threadpool_block);
  CU_add_test (suite, "threadpool_try_work", cunit_threadpool_try_work);
  CU_add_test (suite, "threadpool_stop_start", cunit_threadpool_stop_start);
  CU_add_test (suite, "threadpool_refcount", cunit_threadpool_refcount);
}
