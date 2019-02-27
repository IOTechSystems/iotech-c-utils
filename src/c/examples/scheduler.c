//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/scheduler.h"
#include <unistd.h>

static void testFunc (void *in)
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
  iot_threadpool * thpool = iot_thpool_init (4);

  /* Initialise the scheduler */
  printf ("Initialise the scheduler\n");
  iot_scheduler_t * schd = iot_scheduler_init (thpool);

  /* Create two schedules */
  printf ("Create two schedules\n");
  iot_schedule_t * test1 = iot_schedule_create (schd, testFunc, NULL, IOT_MS_TO_NS(500), 0, 0);
  iot_schedule_t * test2 = iot_schedule_create (schd, testFunc2, NULL, IOT_SEC_TO_NS(1), 0, 0);

  /* Add two schedules to the scheduler */
  printf ("Add two schedules\n");
  iot_schedule_add (schd, test1);
  iot_schedule_add (schd, test2);

  /* Start the scheduler */
  printf ("Start the scheduler\n");
  iot_scheduler_start (schd);
  sleep (5);


  /* Create and add a third schedule */
  printf ("\nCreate and add schedule 3\n");
  iot_schedule_t * test3 = iot_schedule_create (schd, testFunc3, NULL, IOT_SEC_TO_NS(2), 0, 2);
  iot_schedule_add (schd, test3);
  sleep (5);

  /* Remove a schedule */
  printf ("\nRemove schedule 1\n");
  iot_schedule_remove (schd, test1);
  sleep (5);
  /* Delete a schedule */
  printf ("\nDelete all schedules\n");
  iot_schedule_delete (schd, test2);

  /* Stop and delete the scheduler (and associated schedules) */
  printf ("Stop and delete the scheduler\n");
  iot_scheduler_fini (schd);

  sleep (1);
  /* Destroy the threadpool */
  printf ("Destroy the threadpool\n");
  iot_thpool_destroy (thpool);
}
