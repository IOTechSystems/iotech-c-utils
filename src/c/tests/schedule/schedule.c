#include "iot/iot.h"

static void * sched_fn1 (void * arg)
{
  printf ("Schedule 1 run\n");
  return NULL;
}

static void * sched_fn2 (void * arg)
{
  printf ("Schedule 2 run\n");
  return NULL;
}

#define SECS 5

int main (void)
{
  void * arg = NULL;
  uint64_t repeat = 5;
  uint64_t period = IOT_SEC_TO_NS (SECS);

  srand (time (NULL));
  uint64_t start = ((rand () * period) / RAND_MAX);
  printf ("Start NS %" PRIu64 " S %" PRIu64 "\n", start, start / 1000000000);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, NULL);
  iot_schedule_t * sched1 = iot_schedule_create (scheduler, sched_fn1, NULL, arg, period, start, repeat, NULL, -1);
  iot_schedule_t * sched2 = iot_schedule_create (scheduler, sched_fn2, NULL, arg, period, start, repeat, NULL, -1);
  iot_schedule_add (scheduler, sched1);
  iot_schedule_add (scheduler, sched2);
  iot_scheduler_start (scheduler);
  sleep (SECS * repeat + 2);
  iot_scheduler_stop (scheduler);
  return 0;
}
