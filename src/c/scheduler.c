//
// Copyright (c) 2018 IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/container.h"
#include "iot/scheduler.h"
#include "iot/thread.h"

#define IOT_NS_TO_SEC(s) ((s) / IOT_BILLION)
#define IOT_NS_REMAINING(s) ((s) % IOT_BILLION)

/* Schedule */
struct iot_schedule_t
{
  iot_schedule_t * next;           /* The next schedule */
  iot_schedule_t * previous;       /* The previous schedule */
  void (*function) (void * arg);   /* The function called by the schedule */
  void * arg;                      /* Function input arg */
  uint64_t period;                 /* The period of the schedule, in ns */
  uint64_t start;                  /* The start time of the schedule, in ns, */
  uint64_t repeat;                 /* The number of repetitions, 0 = infinite */
  int priority;                    /* Thread priority */
  bool scheduled;                  /* A flag to indicate schedule status */
  bool prio_set;                   /* Whether priority set */
};

/* Schedule Queue */
typedef struct iot_schd_queue_t
{
  struct iot_schedule_t * front;     /* Pointer to the front of queue */
  uint32_t length;                   /* Number of jobs in the queue   */
} iot_schd_queue_t;

/* Scheduler */
struct iot_scheduler_t
{
  iot_component_t component;      /* Component base type */
  iot_schd_queue_t queue;         /* Schedule queue */
  iot_schd_queue_t idle_queue;    /* The queue of idle schedules */
  pthread_mutex_t mutex;          /* Mutex to control access to the scheduler */
  pthread_cond_t cond;            /* Condition to control schedule execution */
  atomic_bool running;            /* Flag to indicate if the scheduler is running */
  iot_threadpool_t * threadpool;  /* Thread pool to post jobs to */
};

/* ========================== PROTOTYPES ============================ */

static void add_schedule_to_queue (iot_schd_queue_t * queue, iot_schedule_t * schedule);
static void remove_schedule_from_queue (iot_schd_queue_t * queue, iot_schedule_t * schedule);

/* ========================== Scheduler ============================ */

/* Convert time in ns to timespec */
static inline void nsToTimespec (uint64_t ns, struct timespec * ts)
{
  ts->tv_sec = IOT_NS_TO_SEC (ns);
  ts->tv_nsec = IOT_NS_REMAINING (ns);
}

/* Get the current time as an unsigned 64bit int */
static uint64_t getTimeAsUInt64 (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);
  return (uint64_t) (ts.tv_sec * IOT_BILLION + ts.tv_nsec);
}

/* Scheduler Thread */
static void iot_scheduler_thread (void * arg)
{
  struct timespec schdTime;
  uint64_t ns;

  iot_scheduler_t * scheduler = (iot_scheduler_t*) arg;
  iot_schd_queue_t * queue = &scheduler->queue;
  iot_schd_queue_t * idle_queue = &scheduler->idle_queue;

  clock_gettime (CLOCK_REALTIME, &schdTime);
  while (atomic_load (&scheduler->running))
  {
    pthread_mutex_lock (&scheduler->mutex);
    /* Wait until the next schedule is due to execute*/
    if (pthread_cond_timedwait (&scheduler->cond, &scheduler->mutex, &schdTime) == 0)
    {
      ns = (atomic_load (&scheduler->running)) ? queue->front->start : 0;
    }
    else
    {
      /* Check if the queue is populated */
      if (queue->length > 0)
      {
        /* Get the schedule at the front of the queue */
        iot_schedule_t *current = queue->front;

        /* Post the work to the threadpool */
        iot_threadpool_add_work (scheduler->threadpool, current->function, current->arg, current->prio_set ? &current->priority : NULL);

        /* Recalculate the next start time for the schedule */
        uint64_t time_now = getTimeAsUInt64 ();
        current->start = time_now + current->period;

        if (current->repeat != 0)
        {
          current->repeat -= 1;
          /* If the number of repetitions has just become 0 */
          if (current->repeat == 0)
          {
            /* Move schedule to idle queue */
            remove_schedule_from_queue (queue, current);
            add_schedule_to_queue (idle_queue, current);
            current->scheduled = false;
          }
          else
          {
            /* Re-queue schedule */
            remove_schedule_from_queue (queue, current);
            add_schedule_to_queue (queue, current);
          }
        }
        else
        {
          /* Remove from current position and add in new location */
          remove_schedule_from_queue (queue, current);
          add_schedule_to_queue (queue, current);
        }
        ns = (queue->length > 0) ? queue->front->start : (getTimeAsUInt64 () + IOT_SEC_TO_NS (1));
      }
      else
      {
        /* Set the wait time to 1 second if the queue is not populated */
        ns = getTimeAsUInt64 () + IOT_SEC_TO_NS (1);
      }
    }
    /* Convert the next execution time (in ns) to timespec */
    nsToTimespec (ns, &schdTime);
    pthread_mutex_unlock (&scheduler->mutex);
  }
}

/* Initialise the schedule queue and processing thread */
iot_scheduler_t * iot_scheduler_alloc (iot_threadpool_t * pool)
{
  iot_scheduler_t * scheduler = (iot_scheduler_t*) calloc (1, sizeof (*scheduler));
  iot_mutex_init (&scheduler->mutex);
  pthread_cond_init (&scheduler->cond, NULL);
  scheduler->threadpool = pool;
  scheduler->component.start_fn = (iot_component_start_fn_t) iot_scheduler_start;
  scheduler->component.stop_fn = (iot_component_stop_fn_t) iot_scheduler_stop;
  return scheduler;
}

iot_threadpool_t * iot_scheduler_thread_pool (iot_scheduler_t * scheduler)
{
  assert (scheduler);
  return scheduler->threadpool;
}

/* Start the scheduler thread */
bool iot_scheduler_start (iot_scheduler_t * scheduler)
{
  pthread_mutex_lock (&scheduler->mutex);
  if (scheduler->component.state != IOT_COMPONENT_RUNNING)
  {
    scheduler->component.state = IOT_COMPONENT_RUNNING;
    atomic_store (&scheduler->running, true);
    iot_threadpool_add_work (scheduler->threadpool, iot_scheduler_thread, scheduler, NULL);
  }
  pthread_mutex_unlock (&scheduler->mutex);
  return true;
}

/* Stop the scheduler thread */
void iot_scheduler_stop (iot_scheduler_t * scheduler)
{
  pthread_mutex_lock (&scheduler->mutex);
  if (scheduler->component.state != IOT_COMPONENT_STOPPED)
  {
    scheduler->component.state = IOT_COMPONENT_STOPPED;
    atomic_store (&scheduler->running, false);
    pthread_cond_signal (&scheduler->cond);
  }
  pthread_mutex_unlock (&scheduler->mutex);
}


/* Create a schedule and insert it into the queue */
iot_schedule_t * iot_schedule_create (iot_scheduler_t * scheduler, void (*function) (void*), void * arg, uint64_t period, uint64_t start, uint64_t repeat, const int * priority)
{
  iot_schedule_t * schedule = (iot_schedule_t*) calloc (1, sizeof (*schedule));
  schedule->function = function;
  schedule->arg = arg;
  schedule->period = period;
  schedule->start = start;
  schedule->repeat = repeat;
  schedule->priority = (priority) ? *priority : 0;
  schedule->prio_set = (priority != NULL);

  pthread_mutex_lock (&scheduler->mutex);
  add_schedule_to_queue (&scheduler->idle_queue, schedule);
  pthread_mutex_unlock (&scheduler->mutex);

  return schedule;
}

/* Add a schedule to the queue */
int iot_schedule_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  int ret = 0;

  pthread_mutex_lock (&scheduler->mutex);
  if (!schedule->scheduled)
  {
    /* Remove from idle queue, add to scheduled queue */
    remove_schedule_from_queue (&scheduler->idle_queue, schedule);
    add_schedule_to_queue (&scheduler->queue, schedule);
    /* If the schedule was placed and the front of the queue & the scheduler is running */
    if (scheduler->queue.front == schedule && atomic_load (&scheduler->running))
    {
      pthread_cond_signal (&scheduler->cond);
    }
    schedule->scheduled = true;
    ret = 1;
  }
  pthread_mutex_unlock (&scheduler->mutex);
  return ret;
}

/* Remove a schedule from the queue */
int iot_schedule_remove (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  int ret = 0;

  pthread_mutex_lock (&scheduler->mutex);
  if (schedule->scheduled)
  {
    remove_schedule_from_queue (&scheduler->queue, schedule);
    add_schedule_to_queue (&scheduler->idle_queue, schedule);
    schedule->scheduled = false;
    ret = 1;
  }
  pthread_mutex_unlock (&scheduler->mutex);
  return ret;
}

/* Delete a schedule */
void iot_schedule_delete (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  pthread_mutex_lock (&scheduler->mutex);
  remove_schedule_from_queue (schedule->scheduled ? &scheduler->queue : &scheduler->idle_queue, schedule);
  pthread_mutex_unlock (&scheduler->mutex);
  free (schedule);
}

/* Destroy all remaining scheduler resouces */
void iot_scheduler_free (iot_scheduler_t * scheduler)
{
  iot_scheduler_stop (scheduler);
  while (scheduler->queue.length > 0)
  {
    iot_schedule_delete (scheduler, scheduler->queue.front);
  }
  while (scheduler->idle_queue.length > 0)
  {
    iot_schedule_delete (scheduler, scheduler->idle_queue.front);
  }
  pthread_cond_destroy (&scheduler->cond);
  pthread_mutex_destroy (&scheduler->mutex);
  free (scheduler);
}

/* Add a schedule to the queue */
static void add_schedule_to_queue (iot_schd_queue_t * queue, iot_schedule_t * schedule)
{
  /* Search for the correct schedule location */
  iot_schedule_t * next_schedule = NULL;
  iot_schedule_t * previous_schedule = NULL;
  iot_schedule_t * current_sched = queue->front;
  
  for (uint32_t i = 0; i < queue->length; i++)
  {
    if (schedule->start < current_sched->start)
    {
      next_schedule = current_sched;
      previous_schedule = current_sched->previous;
      i = queue->length;
    }
    else
    {
      previous_schedule = current_sched;
    }
    current_sched = current_sched->next;
  }

  /* Insert new schedule in correct location */
  if (queue->length == 0)
  {
    schedule->next = NULL;
    schedule->previous = NULL;
  }
  else
  {
    /* Set references in new entry */
    schedule->next = next_schedule;
    schedule->previous = previous_schedule;

    /* Update existing references, check if either is at the front or back */
    if (previous_schedule != NULL)
    {
      previous_schedule->next = schedule;
    }
    if (next_schedule != NULL)
    {
      next_schedule->previous = schedule;
    }
  }
  /* Increment the queue length */
  queue->length += 1;

  /* If no pervious schedule, set as front */
  if (previous_schedule == NULL)
  {
    queue->front = schedule;
  }
}

/* Remove a schedule from the queue */
static void remove_schedule_from_queue (iot_schd_queue_t * queue, iot_schedule_t * schedule)
{
  if (schedule->next == NULL && schedule->previous == NULL)
  {
    /* If only one schedule exsists in the queue */
    queue->front = NULL;
  }
  else if (schedule->next == NULL && schedule->previous != NULL)
  {
    /* If the schedule to remove is at the back of the queue */
    schedule->previous->next = NULL;
  }
  else if (schedule->previous == NULL && schedule->next != NULL)
  {
    /* If the schedule to remove is at the front of the queue */
    schedule->next->previous = NULL;
    queue->front = schedule->next;
  }
  else
  {
    /* If the schedule is in the middle of the queue */
    schedule->next->previous = schedule->previous;
    schedule->previous->next = schedule->next;
  }
  schedule->next = NULL;
  schedule->previous = NULL;

  /* Decrement the number of schedules */
  queue->length -= 1;
}

/* Container support */

static iot_component_t * iot_scheduler_config (iot_container_t * cont, const iot_data_t * map)
{
  const char * name = iot_data_string_map_get_string (map, "ThreadPool");
  assert (name);
  iot_threadpool_t * pool = (iot_threadpool_t*) iot_container_find (cont, name);
  assert (pool);
  return (iot_component_t*) iot_scheduler_alloc (pool);
}

const iot_component_factory_t * iot_scheduler_factory (void)
{
  static iot_component_factory_t factory = { IOT_SCHEDULER_TYPE, iot_scheduler_config, (iot_component_free_fn_t) iot_scheduler_free };
  return &factory;
}
