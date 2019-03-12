#include "threadpool.h"
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
  printf ("%s\n", (char*) arg); fflush (stdout);
  sleep (1);
}

static void cunit_threadpool_priority (void)
{
  int max = sched_get_priority_max (SCHED_FIFO);
  int min = sched_get_priority_min (SCHED_FIFO);
  int delta = (max > min) ? 1 : -1;
  printf ("FIFO max: %d min: %d\n", max, min);
  CU_ASSERT (max != min);
  int p1 = min;
  int p2 = p1 + delta;
  int p3 = p2 + delta;
  iot_threadpool_t * pool = iot_thpool_init (1u);
  CU_ASSERT (pool != NULL);
  iot_thpool_add_work (pool, cunit_pool_worker, "Dummy", NULL);
  iot_thpool_add_work (pool, cunit_pool_worker, "Job Prio 1", &p1);
  iot_thpool_add_work (pool, cunit_pool_worker, "Job Prio 2", &p2);
  iot_thpool_add_work (pool, cunit_pool_worker, "Job Prio 3", &p3);
  iot_thpool_wait (pool);
  iot_thpool_destroy (pool);
}

void cunit_threadpool_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("threadpool", suite_init, suite_clean);
  CU_add_test (suite, "threadpool_priority", cunit_threadpool_priority);
}
