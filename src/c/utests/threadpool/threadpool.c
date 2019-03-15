#include "threadpool.h"
#include "iot/thread.h"
#include "CUnit.h"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void cunit_pool_worker (void * arg)
{
  printf ("%s @ priority %d\n", (char*) arg, iot_thread_current_get_priority ());
  sleep (1);
}

static void cunit_threadpool_priority (void)
{
  int max = sched_get_priority_max (SCHED_FIFO);
  int min = sched_get_priority_min (SCHED_FIFO);
  int delta = (max > min) ? 1 : -1;
  printf ("\nFIFO priority max: %d min: %d\n", max, min);
  CU_ASSERT (max != min);
  int p1 = min + delta;
  int p2 = p1 + delta;
  int p3 = p2 + delta;
  iot_threadpool_t * pool = iot_threadpool_init (1u, &min);
  CU_ASSERT (pool != NULL);
  iot_threadpool_add_work (pool, cunit_pool_worker, "Job 0", NULL);
  iot_threadpool_add_work (pool, cunit_pool_worker, "Job 1", &p1);
  iot_threadpool_add_work (pool, cunit_pool_worker, "Job 2", &p2);
  iot_threadpool_add_work (pool, cunit_pool_worker, "Job 3", &p3);
  iot_threadpool_wait (pool);
  iot_threadpool_destroy (pool);
}

void cunit_threadpool_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("threadpool", suite_init, suite_clean);
  CU_add_test (suite, "threadpool_priority", cunit_threadpool_priority);
}
