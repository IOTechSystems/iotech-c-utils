//
// Copyright (c) 2018 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/scheduler.h"

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
  /* Create a threadpool */
  printf ("Create the threadpool\n");
  iot_threadpool_t * pool = iot_threadpool_alloc (4, 0, NULL, NULL);

  /* Initialise the scheduler */
  printf ("Initialise the scheduler\n");
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);

  /* Create two schedules */
  printf ("Create two schedules\n");
  iot_schedule_t * sched1 = iot_schedule_create (scheduler, testFunc1, NULL, IOT_MS_TO_NS (500), 0, 0, NULL);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, testFunc2, NULL, IOT_SEC_TO_NS (1), 0, 0, NULL);

  /* Add two schedules to the scheduler */
  printf ("Add two schedules\n");
  iot_schedule_add (scheduler, sched1);
  iot_schedule_add (scheduler, sched2);

  /* Start the scheduler */
  printf ("Start the scheduler\n");
  iot_scheduler_start (scheduler);
  sleep (5);

  /* Create and add a third schedule */
  printf ("\nCreate and add schedule 3\n");
  iot_schedule_t * sched3 = iot_schedule_create (scheduler, testFunc3, NULL, IOT_SEC_TO_NS (2), 0, 2, NULL);
  iot_schedule_add (scheduler, sched3);
  sleep (5);

  /* Remove a schedule */
  printf ("\nRemove schedule 1\n");
  iot_schedule_remove (scheduler, sched1);
  sleep (5);
  /* Delete a schedule */
  printf ("\nDelete all schedules\n");
  iot_schedule_delete (scheduler, sched2);

  /* Stop and delete the scheduler (and associated schedules) */
  printf ("Stop and delete the scheduler\n");
  iot_scheduler_stop (scheduler);
  iot_scheduler_free (scheduler);

  /* Destroy the thread pool */
  printf ("Destroy the thread pool\n");
  iot_threadpool_free (pool);
  return 0;
}
