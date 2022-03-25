//
// Copyright (c) 2018-2022 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/container.h"
#include "iot/scheduler.h"
#include "iot/thread.h"
#include "iot/time.h"

#define IOT_NS_TO_SEC(s) ((s) / IOT_BILLION)
#define IOT_NS_REMAINING(s) ((s) % IOT_BILLION)
#define IOT_SCHEDULER_DEFAULT_WAKE (IOT_HOUR_TO_NS (24))

#ifdef IOT_BUILD_COMPONENTS
#define IOT_SCHEDULER_FACTORY iot_scheduler_factory ()
#else
#define IOT_SCHEDULER_FACTORY NULL
#endif

static atomic_uint_fast64_t iot_schedule_id_counter = 0;

/* Schedule */
struct iot_schedule_t
{
  iot_schedule_fn_t function;        /* The function called by the schedule */
  iot_schedule_free_fn_t freefn;     /* The function to clear the arguments when schedule is deleted */
  void * arg;                        /* Function input arg */
  iot_threadpool_t * threadpool;     /* Thread pool used to run scheduled function */
  int priority;                      /* Schedule priority (pool override) */
  uint64_t period;                   /* The period of the schedule, in ns */
  uint64_t start;                    /* The start time of the schedule, in ns, */
  uint64_t repeat;                   /* The number of repetitions, 0 = infinite */
  iot_data_t * id;                   /* Unique schedule id (uint64_t) */
  iot_data_t * self;                 /* Data pointer wrapper for schedule */
  atomic_uint_fast64_t dropped;      /* Number of events dropped */
  bool scheduled;                    /* A flag to indicate schedule status */
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
//  iot_schd_queue_t queue;         /* Active schedule queue */
  iot_data_t * listq;             /* Map of schedule lists, keyed by schedule time */
  iot_data_t * idle_map;          /* Map of idle schedules keyed by unique id */
  iot_logger_t * logger;          /* Optional logger */
  struct timespec schd_time;      /* Time for next schedule */
};

static void iot_schedule_queue_add (iot_data_t * map, iot_schedule_t * schedule)
{
  iot_data_t * key = iot_data_alloc_ui64 (schedule->start);
  iot_data_t * set = (iot_data_t*) iot_data_map_get (map, key); // Get set of schedules for start time
  if (set == NULL)
  {
    set = iot_data_alloc_typed_map (IOT_DATA_UINT64, IOT_DATA_POINTER);
    iot_data_map_add (map, key, set);
  }
  else
  {
    iot_data_free (key);
  }
  iot_data_map_add (set, iot_data_add_ref (schedule->id), iot_data_add_ref (schedule->self));
}

static void iot_schedule_queue_remove (iot_data_t * map, iot_schedule_t * schedule)
{
  iot_data_t * key = iot_data_alloc_ui64 (schedule->start);
  iot_data_t * set = (iot_data_t*) iot_data_map_get (map, key);
  iot_data_map_remove (set, schedule->id);
  iot_data_free (key);
}

static iot_schedule_t * iot_schedule_queue_next (iot_data_t * map)
{
  iot_schedule_t * schedule = NULL;
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  if (iot_data_map_iter_has_next (&iter))
  {
    iot_data_t * set = (iot_data_t*) iot_data_map_iter_value (&iter);
    iot_data_map_iter_t iter2;
    iot_data_map_iter (set, &iter2);
    if (iot_data_map_iter_has_next (&iter2))
    {
      schedule = (iot_schedule_t*) iot_data_map_iter_pointer_value (&iter2);
    }
  }
}

static void iot_schedule_queue_update (iot_data_t * map, iot_schedule_t * schedule, uint64_t next)
{
  iot_schedule_queue_remove (map, schedule);
  schedule->start = next;
  iot_schedule_queue_add (map, schedule);
}

/* ========================== Scheduler ============================ */

/* Convert time in ns to timespec */
static inline void nsToTimespec (uint64_t ns, struct timespec * ts)
{
  ts->tv_sec = (time_t) IOT_NS_TO_SEC (ns);
  ts->tv_nsec = (long) IOT_NS_REMAINING (ns);
}

/* Get the current time as an unsigned 64bit int */
static uint64_t getTimeAsUInt64 (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);
  return (uint64_t) ((uint64_t) ts.tv_sec * IOT_BILLION) + (uint64_t) ts.tv_nsec;
}

/* Scheduler thread function */
static void * iot_scheduler_thread (void * arg)
{
  iot_component_state_t state;
  uint64_t ns;
  iot_scheduler_t * scheduler = (iot_scheduler_t*) arg;
  iot_data_t * queue = scheduler->listq;
  // iot_schd_queue_t * queue = &scheduler->queue;

  clock_gettime (CLOCK_REALTIME, &scheduler->schd_time);

  while (true)
  {
    state = iot_component_wait_and_lock (&scheduler->component, (uint32_t) IOT_COMPONENT_DELETED | (uint32_t) IOT_COMPONENT_RUNNING);

    if (state == IOT_COMPONENT_DELETED)
    {
      iot_log_debug (scheduler->logger, "Scheduler thread terminating");
      break; // Exit thread on deletion
    }
    int ret = pthread_cond_timedwait (&scheduler->component.cond, &scheduler->component.mutex, &scheduler->schd_time);
    state = scheduler->component.state;
    if (state == IOT_COMPONENT_DELETED)
    {
      iot_log_debug (scheduler->logger, "Scheduler thread terminating");
      break; // Exit thread on deletion
    }
    if (state == IOT_COMPONENT_STOPPED)
    {
      iot_log_debug (scheduler->logger, "Scheduler thread stopping");
      iot_component_unlock (&scheduler->component);
      continue; // Wait for thread to be restarted or deleted
    }

    /* Get the schedule at the front of the queue */
    iot_schedule_t * current = iot_schedule_queue_next (queue);
    if (ret == 0)
    {
      ns = (current) ? current->start : (getTimeAsUInt64 () + IOT_SCHEDULER_DEFAULT_WAKE);
    }
    else
    {
      /* Check if the queue is populated */
      if (current)
      {
        /* Post the work to the thread pool or run as thread */
        if (current->threadpool)
        {
          iot_log_trace (scheduler->logger, "Running schedule from threadpool");
          if (! iot_threadpool_try_work (current->threadpool, current->function, current->arg, current->priority))
          {
            if (atomic_fetch_add (&current->dropped, 1u) == 0)
            {
              iot_log_warn (scheduler->logger, "Scheduled event dropped for schedule @ %p", current);
            }
          }
        }
        else
        {
          iot_log_trace (scheduler->logger, "Running schedule as thread");
          iot_thread_create (NULL, current->function, current->arg, current->priority, IOT_THREAD_NO_AFFINITY, scheduler->logger);
        }

        /* Recalculate the next start time for the schedule */
        uint64_t next = current->period + getTimeAsUInt64 ();
          // Steve current->start = next;

        if (current->repeat != 0)
        {
          current->repeat -= 1;
          /* If the number of repetitions has just become 0 */
          if (current->repeat == 0)
          {
            iot_log_trace (scheduler->logger, "Schedule %" PRIu64 " now idle", iot_data_ui64 (current->id));
            iot_schedule_queue_remove (scheduler->listq, current);
            current->start = 0u;
            iot_data_map_add (scheduler->idle_map, iot_data_add_ref (current->id), iot_data_add_ref (current->self));
            current->scheduled = false;
          }
          else
          {
            iot_log_trace (scheduler->logger, "Re-queue schedule");
            iot_schedule_queue_update (scheduler->listq, current, next);
          }
        }
        else
        {
          iot_log_trace (scheduler->logger, "Re-schedule schedule");
          iot_schedule_queue_update (scheduler->listq, current, next);
        }
        current = iot_schedule_queue_next (queue)
        ns = (current) ? current->start : (getTimeAsUInt64 () + IOT_SCHEDULER_DEFAULT_WAKE);
      }
      else
      {
        /* Set the wait time to 1 second if the queue is not populated */
        ns = getTimeAsUInt64 () + IOT_SCHEDULER_DEFAULT_WAKE;
      }
    }
    nsToTimespec (ns, &scheduler->schd_time); /* Calculate next execution time */
    iot_component_unlock (&scheduler->component);
  }
  iot_component_unlock (&scheduler->component);
  return NULL;
}

/* Initialise the schedule queue and processing thread */
iot_scheduler_t * iot_scheduler_alloc (int priority, int affinity, iot_logger_t * logger)
{
  iot_scheduler_t * scheduler = (iot_scheduler_t*) calloc (1, sizeof (*scheduler));
  iot_component_init (&scheduler->component, IOT_SCHEDULER_FACTORY, (iot_component_start_fn_t) iot_scheduler_start, (iot_component_stop_fn_t) iot_scheduler_stop);
  scheduler->logger = logger;
  scheduler->idle_map = iot_data_alloc_typed_map (IOT_DATA_UINT64, IOT_DATA_POINTER);
  iot_logger_add_ref (logger);
  iot_log_info (logger, "iot_scheduler_alloc (priority: %d affinity: %d)", priority, affinity);
  iot_thread_create (NULL, iot_scheduler_thread, scheduler, priority, affinity, logger);
  return scheduler;
}

void iot_scheduler_add_ref (iot_scheduler_t * scheduler)
{
  if (scheduler) iot_component_add_ref (&scheduler->component);
}

/* Start the scheduler thread */
void iot_scheduler_start (iot_scheduler_t * scheduler)
{
  assert (scheduler);
  iot_log_trace (scheduler->logger, "iot_scheduler_start()");
  iot_component_set_running (&scheduler->component);
}

/* Stop the scheduler thread */
void iot_scheduler_stop (iot_scheduler_t * scheduler)
{
  assert (scheduler);
  iot_log_trace (scheduler->logger, "iot_scheduler_stop()");
  iot_component_set_stopped (&scheduler->component);
}

/* Create a schedule and insert it into the queue */
iot_schedule_t * iot_schedule_create (iot_scheduler_t * scheduler, iot_schedule_fn_t func, iot_schedule_free_fn_t free_func, void * arg, uint64_t period, uint64_t start, uint64_t repeat, iot_threadpool_t * pool, int priority)
{
  assert (scheduler && func);
  iot_log_trace (scheduler->logger, "iot_schedule_create (period: %" PRId64 " repeat: %" PRId64 ")", period, repeat);
  iot_schedule_t * schedule = (iot_schedule_t*) calloc (1, sizeof (*schedule));
  schedule->function = func;
  schedule->freefn = free_func;
  schedule->arg = arg;
  schedule->period = period;
  schedule->start = getTimeAsUInt64 () + start;
  schedule->repeat = repeat;
  schedule->threadpool = pool;
  schedule->priority = priority;
  schedule->id = iot_data_alloc_ui64 (atomic_fetch_add (&iot_schedule_id_counter, 1u));
  schedule->self = iot_data_alloc_pointer (schedule, NULL);
  iot_threadpool_add_ref (pool);
  iot_component_lock (&scheduler->component);
  iot_data_map_add (scheduler->idle_map, iot_data_add_ref (schedule->id), iot_data_add_ref (schedule->self));
  iot_component_unlock (&scheduler->component);
  return schedule;
}

/* Add a schedule to the queue */
bool iot_schedule_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  bool ret;
  iot_log_trace (scheduler->logger, "iot_schedule_add()");
  iot_component_lock (&scheduler->component);
  if ((ret = !schedule->scheduled))
  {
    /* Remove from idle map, add to scheduled queue */
    iot_data_map_remove (scheduler->idle_map, schedule->id);
    iot_schedule_queue_add (scheduler->listq, schedule);

    /* If the schedule was placed and the front of the queue & the scheduler is running */
    if (scheduler->queue.front == schedule && (scheduler->component.state == IOT_COMPONENT_RUNNING))
    {
      pthread_cond_signal (&scheduler->component.cond);
    }
    schedule->scheduled = true;
  }
  iot_component_unlock (&scheduler->component);
  return ret;
}

/* Remove a schedule from the queue */
bool iot_schedule_remove (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  bool ret;
  iot_log_trace (scheduler->logger, "iot_schedule_remove()");
  iot_component_lock (&scheduler->component);
  if ((ret = schedule->scheduled))
  {
    iot_schedule_queue_remove (scheduler->listq, schedule);
    iot_data_map_add (scheduler->idle_map, iot_data_add_ref (schedule->id), iot_data_add_ref (schedule->self));
    schedule->scheduled = false;
  }
  iot_component_unlock (&scheduler->component);
  return ret;
}

/* Reset schedule timeout */
void iot_schedule_reset (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  iot_log_trace (scheduler->logger, "iot_schedule_reset()");
  iot_component_lock (&scheduler->component);

  /* Recalculate the next start time for the schedule */
  uint64_t next = schedule->period + getTimeAsUInt64 ();
 // schedule->start = getTimeAsUInt64 () + schedule->period;

  if (schedule->scheduled)
  {
    iot_schedule_queue_update (scheduler->listq, schedule, next);
    if (scheduler->queue.front == schedule && (scheduler->component.state == IOT_COMPONENT_RUNNING))
    {
      pthread_cond_signal (&scheduler->component.cond);
    }
  }
  // Steve need to set next even if not scheduled ?
  iot_component_unlock (&scheduler->component);
}

/* Delete a schedule */
void iot_schedule_delete (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  iot_log_trace (scheduler->logger, "iot_schedule_delete()");
  iot_component_lock (&scheduler->component);
  if (schedule->scheduled)
  {
    iot_schedule_queue_remove (scheduler->listq, schedule);
  }
  else
  {
    iot_data_map_remove (scheduler->idle_map, schedule->id);
  }
  iot_component_unlock (&scheduler->component);
  iot_threadpool_free (schedule->threadpool);
  iot_data_free (schedule->id);
  iot_data_free (schedule->self);
  (schedule->freefn) ? schedule->freefn (schedule->arg) : 0;
  free (schedule);
}

extern uint64_t iot_schedule_dropped (const iot_schedule_t * schedule)
{
  assert (schedule);
  return atomic_load (&schedule->dropped);
}

/* Delete all remaining scheduler resources */
void iot_scheduler_free (iot_scheduler_t * scheduler)
{
  if (scheduler && iot_component_dec_ref (&scheduler->component))
  {
    iot_log_trace (scheduler->logger, "iot_scheduler_free()");
    iot_component_set_stopped (&scheduler->component);
    iot_component_lock (&scheduler->component);
    clock_gettime (CLOCK_REALTIME, &scheduler->schd_time);
    iot_component_unlock (&scheduler->component);
    iot_component_set_deleted (&scheduler->component);
    iot_wait_secs (1u);
    while (scheduler->queue.length > 0)
    {
      iot_schedule_delete (scheduler, scheduler->queue.front);
    }
    iot_data_free (scheduler->idle_map);
    iot_logger_free (scheduler->logger);
    iot_component_fini (&scheduler->component);
    free (scheduler);
  }
}

#ifdef IOT_BUILD_COMPONENTS

static iot_component_t * iot_scheduler_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * logger = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Logger"));
  int affinity = (int) iot_data_string_map_get_i64 (map, "Affinity", IOT_THREAD_NO_AFFINITY);
  int prio = (int) iot_data_string_map_get_i64 (map, "Priority", IOT_THREAD_NO_PRIORITY);
  return (iot_component_t*) iot_scheduler_alloc (prio, affinity, logger);
}

const iot_component_factory_t * iot_scheduler_factory (void)
{
  static iot_component_factory_t factory = { IOT_SCHEDULER_TYPE, iot_scheduler_config, (iot_component_free_fn_t) iot_scheduler_free, NULL };
  return &factory;
}

#endif
