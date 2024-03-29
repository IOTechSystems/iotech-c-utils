//
// Copyright (c) 2018-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/iot.h"

static void * testFunc1 (void *in)
{
  printf ("FN-1 ");
  return NULL;
}

static void * testFunc2 (void *in)
{
  printf ("FN-2 ");
  return NULL;
}

static void * testFunc3 (void *in)
{
  printf ("FN-3 ");
  return NULL;
}

static void * testFunc4 (void *in)
{
  printf ("FN-4 ");
  return NULL;
}

static void testFree4 (void *in)
{
  printf ("testFree4\n");
}

int main (void)
{
  iot_logger_t * plogger = iot_logger_alloc ("Scheduler", IOT_LOG_WARN, true);
  iot_logger_t * slogger = iot_logger_alloc ("ThreadPool", IOT_LOG_WARN, true);
  iot_logger_t * mlogger = iot_logger_alloc ("main", IOT_LOG_INFO, true);

  /* Create a threadpool */
  iot_log_info (mlogger, "Creating threadpool");
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, plogger);

  /* Create a scheduler */
  iot_log_info (mlogger, "Creating scheduler");
  iot_scheduler_t * scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, slogger);

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  /* Create two schedules */
  iot_log_info (mlogger, "Create two schedules");
  iot_schedule_t * sched1 = iot_schedule_create (scheduler, testFunc1, NULL, NULL, IOT_MS_TO_NS (700), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, testFunc2, NULL, NULL, IOT_SEC_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);

  /* Add two schedules to the scheduler */
  iot_log_info (mlogger,"Add two schedules");
  iot_schedule_add (scheduler, sched1);
  iot_schedule_add (scheduler, sched2);

  /* Start the scheduler */
  iot_log_info (mlogger, "Start the scheduler");
  iot_scheduler_start (scheduler);
  iot_wait_secs (4);

  /* Create and add a third schedule */
  printf ("\n");
  iot_log_info (mlogger, "Create and add schedule 3");
  iot_schedule_t * sched3 = iot_schedule_create (scheduler, testFunc3, NULL, NULL, IOT_SEC_TO_NS (2), 0, 2, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_add (scheduler, sched3);

  /* Create a schedule with a free function parameter set */
  iot_log_info (mlogger, "Create and add schedule 4");
  iot_schedule_t * sched4 = iot_schedule_create (scheduler, testFunc4, testFree4, NULL, IOT_SEC_TO_NS (1), 0, 0, pool, IOT_THREAD_NO_PRIORITY);
  iot_schedule_add (scheduler, sched4);
  iot_wait_secs (5);

  /* Remove a schedule */
  printf ("\n");
  iot_log_info (mlogger, "Remove schedule 1");
  iot_schedule_remove (scheduler, sched1);
  iot_log_info (mlogger, "Remove schedule 4");
  iot_schedule_remove (scheduler, sched4);
  iot_wait_secs (3);

  /* Delete a schedule */
  printf ("\n");
  iot_log_info (mlogger, "Delete schedule 2");
  iot_schedule_delete (scheduler, sched2);

  iot_wait_secs (2);

  /* Stop and delete the scheduler (and associated schedules) */
  iot_log_info (mlogger, "Stop and delete the scheduler");
  iot_scheduler_stop (scheduler);
  iot_scheduler_free (scheduler);

  iot_wait_secs (1);
  /* Destroy the thread pool */
  iot_log_info (mlogger, "Destroy the thread pool");
  iot_threadpool_stop (pool);
  iot_threadpool_free (pool);
  iot_logger_free (slogger);
  iot_logger_free (plogger);
  iot_logger_free (mlogger);
  return 0;
}
