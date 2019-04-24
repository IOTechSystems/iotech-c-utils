#include "threadpool.h"
#include "iot/thread.h"
#include "CUnit.h"

static int prio_min = -1;
static int prio_max = -1;
static int prio1 = -1;
static int prio2 = -1;
static int prio3 = -1;
static int counter = 0;

static int suite_init (void)
{
  prio_max = sched_get_priority_max (SCHED_FIFO);
  prio_min = sched_get_priority_min (SCHED_FIFO);
  printf ("\nFIFO priority max: %d min: %d\n", prio_max, prio_min);
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void cunit_pool_sleeper (void * arg)
{
  sleep (1);
}

static void cunit_pool_counter (void * arg)
{
  pthread_mutex_t * mutex = (pthread_mutex_t*) arg;
  counter++;
  pthread_mutex_lock (mutex);
  pthread_mutex_unlock (mutex);
  counter--;
}

static void cunit_pool_prio_worker (void * arg)
{
  int prio = *((int*) arg);
  if (prio != iot_thread_current_get_priority ())
  {
    printf ("\n**** Is CAP_SYS_NICE set for runner ??? ****\n");
  }
  CU_ASSERT (prio == iot_thread_current_get_priority ());
}

static void cunit_threadpool_priority_range (void)
{
  CU_ASSERT (prio_max > prio_min);
}

static void cunit_threadpool_priority (void)
{
  prio1 = prio_min + 1;
  prio2 = prio1 + 1;
  prio3 = prio2 + 1;
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 0, &prio_min);
  CU_ASSERT (pool != NULL);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_sleeper, NULL, NULL);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio1, &prio1);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio2, &prio2);
  iot_threadpool_add_work (pool, cunit_pool_prio_worker, &prio3, &prio3);
  iot_threadpool_wait (pool);
  CU_ASSERT (iot_threadpool_num_threads_working (pool) == 0u);
  iot_threadpool_free (pool);
}

static void cunit_threadpool_block (void)
{
  bool ret;
  pthread_mutex_t mutex;
  pthread_mutex_init (&mutex, NULL);
  pthread_mutex_lock (&mutex);
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, 2, NULL);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_counter, &mutex, NULL);
  ret = iot_threadpool_try_work (pool, cunit_pool_counter, &mutex, NULL);
  CU_ASSERT (ret);
  ret = iot_threadpool_try_work (pool, cunit_pool_counter, &mutex, NULL);
  CU_ASSERT (! ret);
  pthread_mutex_unlock (&mutex);
  
  iot_threadpool_wait (pool);
  iot_threadpool_free (pool);
  pthread_mutex_destroy (&mutex);
}

void cunit_threadpool_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("threadpool", suite_init, suite_clean);
  CU_add_test (suite, "threadpool_priority_range", cunit_threadpool_priority_range);
  CU_add_test (suite, "threadpool_priority", cunit_threadpool_priority);
  CU_add_test (suite, "threadpool_block", cunit_threadpool_block);
}
