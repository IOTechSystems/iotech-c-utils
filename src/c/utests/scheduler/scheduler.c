#include <iot/threadpool.h>
#include <iot/iot.h>
#include <CUnit.h>
#include "scheduler.h"

int sum_test;
int infinity_test;

static void do_work1 (void *in)
{
  int sum = 0;
  for (int i = 0; i < 10; ++i)
  {
    sum += i;
  }
}

static void do_work2 (void *in)
{
  int sum = 0;
  for (int i = 0; i < 20; ++i)
  {
    sum += i;
  }
}

static void do_work3 (void *in)
{
  int sum = 0;
  for (int i = 0; i < 30; ++i)
  {
    sum += i;
  }
}

static void do_work4 (void *in)
{
  sum_test += 1;
}

static void do_work5 (void *in)
{
  infinity_test += 1;
}


static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void cunit_scheduler_start (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);

  CU_ASSERT (iot_threadpool_start (pool) == true);
  CU_ASSERT (iot_scheduler_start (scheduler) == true);

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, do_work1, NULL, IOT_MS_TO_NS (500), 0, 0, NULL);
  CU_ASSERT (sched1 != NULL);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, do_work2, NULL, IOT_SEC_TO_NS (1), 0, 0, NULL);
  CU_ASSERT (sched2 != NULL);
  iot_schedule_t * sched3 = iot_schedule_create (scheduler, do_work3, NULL, IOT_SEC_TO_NS (1), 0, 0, NULL);
  CU_ASSERT (sched3 != NULL);

  CU_ASSERT (iot_schedule_add (scheduler, sched1) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched2) == 1);

  iot_scheduler_start (scheduler);
  sleep (2);

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_stop (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);

  CU_ASSERT (iot_threadpool_start (pool) == true);
  CU_ASSERT (iot_scheduler_start (scheduler) == true);
  sum_test = 0;

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  CU_ASSERT (sched1 != NULL);
  CU_ASSERT (iot_schedule_add (scheduler, sched1) == 1);

  iot_scheduler_start (scheduler);

  sleep (2);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (sum_test == 1);

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_create (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);

  CU_ASSERT (iot_threadpool_start (pool) == true);
  CU_ASSERT (iot_scheduler_start (scheduler) == true);
  sum_test = 0;

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (250), 0, 1, NULL);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);

  CU_ASSERT (sched1 != NULL);
  CU_ASSERT (sched2 != NULL);
  CU_ASSERT (iot_schedule_add (scheduler, sched1) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched2) == 1);
  iot_scheduler_start (scheduler);

  sleep (1);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (sum_test == 1);
  CU_ASSERT (infinity_test > 5);

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_remove (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);

  CU_ASSERT (iot_threadpool_start (pool) == true);
  CU_ASSERT (iot_scheduler_start (scheduler) == true);
  sum_test = 0;

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t * sched3 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);;

  CU_ASSERT (sched1 != NULL);
  CU_ASSERT (sched2 != NULL);
  CU_ASSERT (sched3 != NULL);
  CU_ASSERT (iot_schedule_add (scheduler, sched1) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched2) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched3) == 1);

  iot_scheduler_start (scheduler);
  sleep (1);

  iot_schedule_remove (scheduler, sched2);
  iot_schedule_remove (scheduler, sched3);
  CU_ASSERT (sum_test == 1);
  CU_ASSERT (infinity_test > 20);

  int temp = infinity_test;
  iot_scheduler_start (scheduler);
  sleep (1);

  CU_ASSERT (temp == infinity_test);

  iot_schedule_delete (scheduler, sched3);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_delete (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);

  CU_ASSERT (iot_threadpool_start (pool) == true);
  CU_ASSERT (iot_scheduler_start (scheduler) == true);
  sum_test = 0;

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  iot_schedule_t * sched3 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);;
  iot_schedule_t * sched4 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);;

  CU_ASSERT (sched1 != NULL);
  CU_ASSERT (sched2 != NULL);
  CU_ASSERT (sched3 != NULL);
  CU_ASSERT (sched4 != NULL);
  CU_ASSERT (iot_schedule_add (scheduler, sched1) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched2) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched3) == 1);
  CU_ASSERT (iot_schedule_add (scheduler, sched4) == 1);
  iot_scheduler_start (scheduler);
  sleep (1);

  iot_schedule_delete (scheduler, sched2);
  iot_schedule_delete (scheduler, sched3);

  CU_ASSERT (sum_test == 1);
  CU_ASSERT (infinity_test > 20);

  int temp = infinity_test;
  iot_scheduler_start (scheduler);
  sleep (1);

  CU_ASSERT (temp == infinity_test);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_refcount (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);
  iot_scheduler_add_ref (scheduler);
  iot_scheduler_free (scheduler);
  iot_scheduler_free (scheduler);
  iot_scheduler_free (NULL);
  iot_threadpool_free (pool);
}

static void cunit_scheduler_iot_scheduler_thread_pool (void)
{
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);
  assert (pool == iot_scheduler_thread_pool (scheduler));
  iot_scheduler_free (scheduler);
  iot_threadpool_free (pool);
}


extern void cunit_scheduler_test_init ()
{
  CU_pSuite suite = CU_add_suite ("scheduler", suite_init, suite_clean);
  CU_add_test (suite, "scheduler_start", cunit_scheduler_start);
  CU_add_test (suite, "scheduler_stop", cunit_scheduler_stop);
  CU_add_test (suite, "scheduler_create", cunit_scheduler_create);
  CU_add_test (suite, "scheduler_remove", cunit_scheduler_remove);
  CU_add_test (suite, "scheduler_delete", cunit_scheduler_delete);
  CU_add_test (suite, "scheduler_refcount", cunit_scheduler_refcount);
  CU_add_test (suite, "scheduler_iot_scheduler_thread_pool", cunit_scheduler_iot_scheduler_thread_pool);

}

