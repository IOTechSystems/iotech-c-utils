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

struct iot_schedule_t
{
  iot_schedule_fn_t function;        /* The function called by the schedule */
  iot_schedule_fn_t run_cb;          /* The function called when schedule is run */
  iot_schedule_fn_t abort_cb;        /* The function called when schedule run is aborted */
  iot_schedule_free_fn_t free_cb;    /* The function to clear the arguments when schedule is deleted */
  void * arg;                        /* Function input arg */
  iot_threadpool_t * threadpool;     /* Thread pool used to run scheduled function */
  int priority;                      /* Schedule priority (pool override) */
  uint64_t period;                   /* The period of the schedule, in ns */
  uint64_t start;                    /* The start time of the schedule, in ns */
  uint64_t repeat;                   /* The number of repetitions, 0 == infinite */
  uint64_t id;                       /* Schedule unique id */
  _Atomic uint64_t dropped;          /* Number of events dropped */
  bool scheduled;                    /* A flag to indicate schedule status */
  bool concurrent;                   /* Whether a schedule can be concurrently executed */
  iot_data_static_t start_key;       /* Data wrapper for schedule start time used as key for queue map */
  iot_data_static_t id_key;          /* Data wrapper for schedule id used as key for idle map */
  iot_data_static_t self_static;     /* Data wrapper for self pointer used as value for idle and queue maps */
  atomic_int_fast32_t refs;          /* Current reference count */
};

struct iot_scheduler_t
{
  iot_component_t component;      /* Component base type */
  iot_data_t * queue;             /* Map of active schedules, keyed by unique schedule time */
  iot_data_t * idle;              /* Map of idle schedules, keyed by unique schedule id */
  iot_logger_t * logger;          /* Optional logger */
  struct timespec schd_time;      /* Time for next schedule */
};

static inline void iot_schedule_add_ref (iot_schedule_t * schedule)
{
  atomic_fetch_add (&schedule->refs, 1u);
}

static inline bool iot_schedule_dec_ref (iot_schedule_t * schedule)
{
  return (atomic_fetch_sub (&schedule->refs, 1u) <= 1u);
}

static void iot_schedule_free (iot_schedule_t * schedule)
{
  if (iot_schedule_dec_ref (schedule))
  {
    (schedule->free_cb) ? schedule->free_cb (schedule->arg) : 0;
    iot_threadpool_free (schedule->threadpool);
    free (schedule);
  }
}

static inline void iot_schedule_update_start (iot_schedule_t * schedule, uint64_t start)
{
  schedule->start = start;
  iot_data_alloc_const_ui64 (&schedule->start_key, start);
}

static inline iot_schedule_t * iot_schedule_queue_next (iot_data_t * map)
{
  return (iot_schedule_t*) iot_data_map_start_pointer (map);
}

static inline bool iot_schedule_is_next (iot_data_t * map, const iot_schedule_t * schedule)
{
  return (iot_schedule_queue_next (map) == schedule);
}

static bool iot_schedule_queue_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  while (iot_data_map_get (scheduler->queue, IOT_DATA_STATIC (&schedule->start_key)))
  {
    iot_schedule_update_start (schedule, schedule->start + 1u); // Need unique start as used as map key
  }
  iot_data_map_add (scheduler->queue, IOT_DATA_STATIC (&schedule->start_key), IOT_DATA_STATIC (&schedule->self_static));
  schedule->scheduled = true;
  return iot_schedule_is_next (scheduler->queue, schedule);
}

static inline void iot_schedule_idle_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  iot_data_map_add (scheduler->idle, IOT_DATA_STATIC (&schedule->id_key), IOT_DATA_STATIC (&schedule->self_static));
  schedule->scheduled = false;
}

static inline void iot_schedule_idle_remove (iot_scheduler_t * scheduler, const iot_schedule_t * schedule)
{
  iot_data_map_remove (scheduler->idle, IOT_DATA_STATIC (&schedule->id_key));
}

static inline void iot_schedule_queue_remove (iot_scheduler_t * scheduler, const iot_schedule_t * schedule)
{
  iot_data_map_remove (scheduler->queue, IOT_DATA_STATIC (&schedule->start_key));
}

static bool iot_schedule_queue_update (iot_scheduler_t * scheduler, iot_schedule_t * schedule, uint64_t next)
{
  iot_schedule_queue_remove (scheduler, schedule);
  iot_schedule_update_start (schedule, next);
  return iot_schedule_queue_add (scheduler, schedule);
}

static inline void nsToTimespec (uint64_t ns, struct timespec * ts)
{
  ts->tv_sec = (time_t) IOT_NS_TO_SEC (ns);
  ts->tv_nsec = (long) IOT_NS_REMAINING (ns);
}

void iot_schedule_set_concurrent (iot_schedule_t * schedule, bool enable)
{
  assert (schedule);
  schedule->concurrent = enable;
}

static void * schedule_fn (void * data)
{
  iot_schedule_t * schedule = data;
  void * ret = schedule->function (schedule->arg);
  iot_schedule_free (schedule);
  return ret;
}

/* Scheduler thread function */
static void * iot_scheduler_thread (void * arg)
{
  iot_component_state_t state;
  uint64_t next = iot_time_nsecs ();
  iot_scheduler_t * scheduler = (iot_scheduler_t*) arg;
  iot_data_t * queue = scheduler->queue;

  nsToTimespec (next, &scheduler->schd_time);
  while (true)
  {
    state = iot_component_wait_and_lock (&scheduler->component, (uint32_t) IOT_COMPONENT_DELETED | (uint32_t) IOT_COMPONENT_RUNNING); // State wait
    if (state == IOT_COMPONENT_DELETED) break; // Exit thread on deletion
    pthread_cond_timedwait (&scheduler->component.cond, &scheduler->component.mutex, &scheduler->schd_time); // Schedule wait
    state = scheduler->component.state;
    if (state != IOT_COMPONENT_RUNNING)
    {
      iot_log_debug (scheduler->logger, "Scheduler thread %s", (state == IOT_COMPONENT_DELETED) ? "terminating" : "stopping");
      if (state == IOT_COMPONENT_DELETED) break; // Exit thread on deletion
      iot_component_unlock (&scheduler->component);
      continue; // Wait for thread to be restarted or deleted
    }

    /* Get the schedule at the front of the queue */
    iot_schedule_t * current = iot_schedule_queue_next (queue);
    if (current && current->start < iot_time_nsecs ()) // If a schedule and ready to run
    {
      if (current->concurrent || (atomic_load (&current->refs) == 1u)) // Check for concurrent execution
      {
        /* Notify that the schedule is about to run */
        if (current->run_cb)
        {
          iot_component_unlock (&scheduler->component);
          current->run_cb (current->arg);
          iot_component_lock (&scheduler->component);
        }
        /* Post the work to the thread pool or run as thread */
        iot_schedule_add_ref (current);
        if (current->threadpool)
        {
          iot_log_trace (scheduler->logger, "Running schedule #%" PRIu64 " from threadpool", current->id);
          if (! iot_threadpool_try_work (current->threadpool, schedule_fn, current, current->priority))
          {
            iot_schedule_dec_ref (current);
            /* Notify that the run is aborted */
            if (current->abort_cb)
            {
              iot_component_unlock (&scheduler->component);
              current->abort_cb (current->arg);
              iot_component_lock (&scheduler->component);
            }
            if (atomic_fetch_add (&current->dropped, 1u) == 0u)
            {
              iot_log_warn (scheduler->logger, "Scheduled event dropped for schedule #%" PRIu64, current->id);
            }
          }
        }
        else
        {
          iot_log_trace (scheduler->logger, "Running schedule #%" PRIu64 " as thread", current->id);
          if (!iot_thread_create (NULL, schedule_fn, current, current->priority, IOT_THREAD_NO_AFFINITY, scheduler->logger))
          {
            iot_schedule_dec_ref (current);
          }
        }
      }
      else
      {
        iot_log_trace (scheduler->logger, "Skipping schedule #%" PRIu64 " as running", current->id);
      }

      /* Recalculate the next start time for the schedule */
      next = current->period + iot_time_nsecs ();
      if (current->repeat > 0u) // Repetitive schedule
      {
        if (--(current->repeat) == 0u) // Last repetition
        {
          iot_log_trace (scheduler->logger, "Schedule #%" PRIu64 " now idle", current->id);
          iot_schedule_queue_remove (scheduler, current);
          iot_schedule_idle_add (scheduler, current);
        }
        else
        {
          iot_log_trace (scheduler->logger, "Re-queue repeating schedule #%" PRIu64, current->id);
          iot_schedule_queue_update (scheduler, current, next);
        }
      }
      else
      {
        iot_log_trace (scheduler->logger, "Re-schedule infinite schedule #%" PRIu64, current->id);
        iot_schedule_queue_update (scheduler, current, next);
      }
      current = iot_schedule_queue_next (queue);
    }
    next = current ? current->start : (iot_time_nsecs () + IOT_SCHEDULER_DEFAULT_WAKE);
    nsToTimespec (next, &scheduler->schd_time); /* Calculate next execution time */
    iot_component_unlock (&scheduler->component);
  }
  iot_component_unlock (&scheduler->component);
  return NULL;
}

iot_scheduler_t * iot_scheduler_alloc (int priority, int affinity, iot_logger_t * logger)
{
  iot_scheduler_t * scheduler = (iot_scheduler_t*) calloc (1u, sizeof (*scheduler));
  iot_component_init (&scheduler->component, IOT_SCHEDULER_FACTORY, (iot_component_start_fn_t) iot_scheduler_start, (iot_component_stop_fn_t) iot_scheduler_stop);
  scheduler->logger = logger;
  scheduler->idle = iot_data_alloc_map (IOT_DATA_UINT64);
  scheduler->queue = iot_data_alloc_map (IOT_DATA_UINT64);
  iot_logger_add_ref (logger);
  iot_log_info (logger, "iot_scheduler_alloc (priority: %d affinity: %d)", priority, affinity);
  iot_thread_create (NULL, iot_scheduler_thread, scheduler, priority, affinity, logger);
  return scheduler;
}

void iot_scheduler_add_ref (iot_scheduler_t * scheduler)
{
  if (scheduler) iot_component_add_ref (&scheduler->component);
}

void iot_scheduler_start (iot_scheduler_t * scheduler)
{
  assert (scheduler);
  iot_log_trace (scheduler->logger, "iot_scheduler_start");
  iot_component_set_running (&scheduler->component);
}

void iot_scheduler_stop (iot_scheduler_t * scheduler)
{
  assert (scheduler);
  iot_log_trace (scheduler->logger, "iot_scheduler_stop");
  iot_component_set_stopped (&scheduler->component);
}

/* Create a schedule and insert it into the idle queue */
iot_schedule_t * iot_schedule_create (iot_scheduler_t * scheduler, iot_schedule_fn_t func, iot_schedule_free_fn_t free_func, void * arg, uint64_t period, uint64_t delay, uint64_t repeat, iot_threadpool_t * pool, int priority)
{
  static _Atomic uint64_t schedule_id_counter = 0;

  assert (scheduler && func);
  iot_schedule_t * schedule = (iot_schedule_t*) calloc (1, sizeof (*schedule));
  schedule->function = func;
  schedule->free_cb = free_func;
  schedule->arg = arg;
  schedule->period = period;
  schedule->start = iot_time_nsecs () + delay;
  schedule->repeat = repeat;
  schedule->threadpool = pool;
  schedule->priority = priority;
  schedule->id = atomic_fetch_add (&schedule_id_counter, 1u);
  iot_data_alloc_const_ui64 (&schedule->id_key, schedule->id);
  iot_data_alloc_const_ui64 (&schedule->start_key, schedule->start);
  iot_data_alloc_const_pointer (&schedule->self_static, schedule);
  atomic_store (&schedule->refs, 1u);
  iot_log_trace (scheduler->logger, "iot_schedule_create #%" PRIu64 " (period: %"PRId64" repeat: %"PRId64" delay: %"PRId64")", schedule->id, period, repeat, delay);
  iot_threadpool_add_ref (pool);
  iot_component_lock (&scheduler->component);
  iot_schedule_idle_add (scheduler, schedule);
  iot_component_unlock (&scheduler->component);
  return schedule;
}

bool iot_schedule_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  bool ret;
  iot_log_trace (scheduler->logger, "iot_schedule_add #%" PRIu64, schedule->id);
  iot_component_lock (&scheduler->component);
  if ((ret = !schedule->scheduled))
  {
    /* Remove from idle map, add to scheduled queue */
    iot_schedule_idle_remove (scheduler, schedule);
    bool front = iot_schedule_queue_add (scheduler, schedule);

    /* If the schedule was placed and the front of the queue and the scheduler is running */
    if (front && (scheduler->component.state == IOT_COMPONENT_RUNNING))
    {
      pthread_cond_signal (&scheduler->component.cond);
    }
  }
  iot_component_unlock (&scheduler->component);
  return ret;
}

bool iot_schedule_remove (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  bool ret;
  iot_log_trace (scheduler->logger, "iot_schedule_remove #%" PRIu64, schedule->id);
  iot_component_lock (&scheduler->component);
  if ((ret = schedule->scheduled))
  {
    iot_schedule_queue_remove (scheduler, schedule);
    iot_schedule_idle_add (scheduler, schedule);
  }
  iot_component_unlock (&scheduler->component);
  return ret;
}

/* Reset schedule timeout */
void iot_schedule_reset (iot_scheduler_t * scheduler, iot_schedule_t * schedule, uint64_t delay)
{
  assert (scheduler && schedule);
  iot_log_trace (scheduler->logger, "iot_schedule_reset #%"PRIu64" delay: %"PRIu64, schedule->id, delay);
  iot_component_lock (&scheduler->component);

  /* Recalculate the next start time for the schedule */
  uint64_t next = schedule->period + iot_time_nsecs () + delay;
  if (schedule->scheduled)
  {
    bool front = iot_schedule_queue_update (scheduler, schedule, next);
    if (front && (scheduler->component.state == IOT_COMPONENT_RUNNING))
    {
      pthread_cond_signal (&scheduler->component.cond);
    }
  }
  else
  {
    iot_schedule_update_start (schedule, next);
  }
  iot_component_unlock (&scheduler->component);
}

void iot_schedule_add_run_callback (iot_scheduler_t * scheduler, iot_schedule_t * schedule, iot_schedule_fn_t func)
{
  assert (scheduler && schedule);
  iot_component_lock (&scheduler->component);
  schedule->run_cb = func;
  iot_component_unlock (&scheduler->component);
}

void iot_schedule_add_abort_callback (iot_scheduler_t * scheduler, iot_schedule_t * schedule, iot_schedule_fn_t func)
{
  assert (scheduler && schedule);
  iot_component_lock (&scheduler->component);
  schedule->abort_cb = func;
  iot_component_unlock (&scheduler->component);
}

void iot_schedule_delete (iot_scheduler_t * scheduler, iot_schedule_t * schedule)
{
  assert (scheduler && schedule);
  iot_log_trace (scheduler->logger, "iot_schedule_delete #%" PRIu64, schedule->id);
  iot_component_lock (&scheduler->component);
  (schedule->scheduled) ? iot_schedule_queue_remove (scheduler, schedule) : iot_schedule_idle_remove (scheduler, schedule);
  iot_component_unlock (&scheduler->component);
  iot_schedule_free (schedule);
}

extern uint64_t iot_schedule_dropped (const iot_schedule_t * schedule)
{
  assert (schedule);
  return atomic_load (&schedule->dropped);
}

extern uint64_t iot_schedule_id (const iot_schedule_t * schedule)
{
  assert (schedule);
  return schedule->id;
}

static void iot_scheduler_free_schedules (iot_data_t * map)
{
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    iot_schedule_t * schedule = (iot_schedule_t*) iot_data_map_iter_pointer_value (&iter);
    iot_data_map_remove (map, key);
    iot_schedule_free (schedule);
    iot_data_map_iter (map, &iter);
  }
  iot_data_free (map);
}

void iot_scheduler_free (iot_scheduler_t * scheduler)
{
  if (scheduler && iot_component_dec_ref (&scheduler->component))
  {
    iot_log_trace (scheduler->logger, "iot_scheduler_free");
    iot_component_set_stopped (&scheduler->component); // Break schedule thread out of schedule wait
    iot_wait_usecs (500u);
    iot_component_set_deleted (&scheduler->component); // Break schedule thread out of state wait
    iot_wait_usecs (500u);
    iot_scheduler_free_schedules (scheduler->queue);
    iot_scheduler_free_schedules (scheduler->idle);
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
  static iot_component_factory_t factory =
  {
    IOT_SCHEDULER_TYPE,
    IOT_CATEGORY_CORE,
    iot_scheduler_config,
    (iot_component_free_fn_t) iot_scheduler_free,
    NULL,
    NULL
  };
  return &factory;
}

#endif
