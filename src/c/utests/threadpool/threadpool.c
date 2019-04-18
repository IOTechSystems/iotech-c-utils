#include "threadpool.h"
#include "iot/thread.h"
#include "CUnit.h"

static int prio_min = -1;
static int prio_max = -1;
static int prio1 = -1;
static int prio2 = -1;
static int prio3 = -1;

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

static void cunit_pool_worker (void * arg)
{
  int prio = *((int*) arg);
  if (prio != iot_thread_current_get_priority ())
  {
    printf ("\n**** Is CAP_SYS_NICE set for runner ??? ****\n");
  }
  CU_ASSERT (prio == iot_thread_current_get_priority ());
}

static void cunit_threadpool_priority (void)
{
  CU_ASSERT (prio_max > prio_min);
  prio1 = prio_min + 1;
  prio2 = prio1 + 1;
  prio3 = prio2 + 1;
  iot_threadpool_t * pool = iot_threadpool_alloc (1u, &prio_min);
  CU_ASSERT (pool != NULL);
  iot_threadpool_start (pool);
  iot_threadpool_add_work (pool, cunit_pool_sleeper, NULL, NULL);
  iot_threadpool_add_work (pool, cunit_pool_worker, &prio1, &prio1);
  iot_threadpool_add_work (pool, cunit_pool_worker, &prio2, &prio2);
  iot_threadpool_add_work (pool, cunit_pool_worker, &prio3, &prio3);
  iot_threadpool_wait (pool);
  iot_threadpool_free (pool);
}

void cunit_threadpool_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("threadpool", suite_init, suite_clean);
  CU_add_test (suite, "threadpool_priority", cunit_threadpool_priority);
}
