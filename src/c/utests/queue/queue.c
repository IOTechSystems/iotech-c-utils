/*
 * Copyright (c) 2023
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "queue.h"
#include "CUnit.h"
#include "iot/queue.h"

#define SINGLE_SIZE 5
#define MULTI_SIZE 1000
#define MULTI_THREADS 2

static bool jobs[MULTI_SIZE];

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void test_alloc (void)
{
  uint32_t size;
  uint32_t maxsize;
  iot_queue_t *q = iot_queue_alloc (10);
  CU_ASSERT (q != NULL)
  size = iot_queue_size (q);
  CU_ASSERT (size == 0)
  maxsize = iot_queue_maxsize (q);
  CU_ASSERT (maxsize == 10)
  iot_queue_free (q);
}

static void test_run_single (void)
{
  bool ok;
  uint32_t size;
  iot_data_t *e;
  iot_queue_t *q = iot_queue_alloc (SINGLE_SIZE);
  iot_queue_enqueue (q, iot_data_alloc_ui32 (10));
  for (unsigned i = 1; i < SINGLE_SIZE; i++)
  {
    ok = iot_queue_try_enqueue (q, iot_data_alloc_ui64 (i));
    CU_ASSERT (ok)
  }
  size = iot_queue_size (q);
  CU_ASSERT (size == SINGLE_SIZE)
  ok = iot_queue_try_enqueue (q, iot_data_alloc_null ());
  CU_ASSERT (ok == false)
  e = iot_queue_dequeue (q);
  CU_ASSERT (e != NULL)
  iot_data_free (e);
  for (unsigned i = 1; i < SINGLE_SIZE; i++)
  {
    e = iot_queue_try_dequeue (q);
    CU_ASSERT (e != NULL)
    iot_data_free (e);
  }
  size = iot_queue_size (q);
  CU_ASSERT (size == 0)
  e = iot_queue_try_dequeue (q);
  CU_ASSERT (e == NULL)
  iot_queue_free (q);
}

static void *multi_processor (void *arg)
{
  iot_queue_t *q = (iot_queue_t *)arg;
  while (true)
  {
    iot_data_t *e = iot_queue_dequeue (q);
    if (e)
    {
      uint32_t i = iot_data_ui32 (e);
      CU_ASSERT (jobs[i] == false)
      jobs[i] = true;
      iot_data_free (e);
      sched_yield ();
    }
    else
    {
      break;
    }
  }
  return NULL;
}

static void test_run_multi (void)
{
  unsigned missed;
  pthread_t workers[MULTI_THREADS];
  iot_queue_t *q = iot_queue_alloc (0);
  for (unsigned i = 0; i < MULTI_SIZE; i++)
  {
    jobs[i] = false;
  }
  for (unsigned i = 0; i < MULTI_THREADS; i++)
  {
    pthread_create (&workers[i], NULL, multi_processor, q);
  }
  for (unsigned i = 0; i < MULTI_SIZE; i++)
  {
    iot_queue_enqueue (q, iot_data_alloc_ui32 (i));
  }
  printf ("residual queue size %u/%u ...", iot_queue_size (q), MULTI_SIZE);
  while (iot_queue_size (q))
  {
    sched_yield ();
  }
  iot_queue_stop (q);
  for (unsigned i = 0; i < MULTI_THREADS; i++)
  {
    pthread_join (workers[i], NULL);
  }
  missed = 0;
  for (unsigned i = 0; i < MULTI_SIZE; i++)
  {
    if (jobs[i] == false)
    {
      missed++;
    }
  }
  if (missed)
  {
    printf ("Queue: missed %u elements\n", missed);
    CU_ASSERT (missed == 0)
  }
  iot_queue_free (q);
}

void cunit_queue_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("queue", suite_init, suite_clean);
  CU_add_test (suite, "queue_alloc", test_alloc);
  CU_add_test (suite, "queue_run_single", test_run_single);
  CU_add_test (suite, "queue_run_multi", test_run_multi);
}
