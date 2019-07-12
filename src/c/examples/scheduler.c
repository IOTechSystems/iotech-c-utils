//
// Copyright (c) 2018 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/scheduler.h"
#include "iot/logger.h"

static void testFunc1 (void *in)
{
  printf ("FN-1 ");
}

static void testFunc2 (void *in)
{
  printf ("FN-2 ");
}

static void testFunc3 (void *in)
{
  printf ("FN-3 ");
}

int main (void)
{
  iot_logger_t *logger = NULL;
  logger = iot_logger_alloc ("Scheduler_Example", IOT_LOG_INFO);

  iot_logger_start (logger);

  /* Create a threadpool */
  iot_log_info (logger, "Create the threadpool");
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, logger);

  /* Initialise the scheduler */
  iot_log_info (logger, "Initialise the scheduler");
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool, logger);


  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  /* Create two schedules */
  iot_log_info (logger, "Create two schedules");
  iot_schedule_t * sched1 = iot_schedule_create (scheduler, testFunc1, NULL, IOT_MS_TO_NS (500), 0, 0, NULL);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, testFunc2, NULL, IOT_SEC_TO_NS (1), 0, 0, NULL);

  /* Add two schedules to the scheduler */
  iot_log_info (logger,"Add two schedules");
  iot_schedule_add (scheduler, sched1);
  iot_schedule_add (scheduler, sched2);

  /* Start the scheduler */
  iot_log_info (logger, "Start the scheduler");
  iot_scheduler_start (scheduler);
  sleep (5);

  /* Create and add a third schedule */
  printf ("\n");
  iot_log_info (logger, "Create and add schedule 3");
  iot_schedule_t * sched3 = iot_schedule_create (scheduler, testFunc3, NULL, IOT_SEC_TO_NS (2), 0, 2, NULL);
  iot_schedule_add (scheduler, sched3);
  sleep (5);

  /* Remove a schedule */
  printf ("\n");
  iot_log_info (logger, "Remove schedule 1");
  iot_schedule_remove (scheduler, sched1);
  sleep (5);

  /* Delete a schedule */
  printf ("\n");
  iot_log_info (logger, "Delete all schedules");
  iot_schedule_delete (scheduler, sched2);

  /* Stop and delete the scheduler (and associated schedules) */
  iot_log_info (logger, "Stop and delete the scheduler");
  iot_scheduler_stop (scheduler);
  iot_scheduler_free (scheduler);

  /* Destroy the thread pool */
  iot_log_info (logger, "Destroy the thread pool");
  iot_threadpool_free (pool);
  iot_logger_free (logger);
  return 0;
}
