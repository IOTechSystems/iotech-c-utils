//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/container.h"
#include "iot/threadpool.h"
#include "iot/thread.h"
#include "iot/data.h"
#include "iot/time.h"

#ifdef IOT_HAS_PRCTL
#include <sys/prctl.h>
#endif

#define IOT_PRCTL_NAME_MAX 16
#define IOT_TP_THREADS_DEFAULT 2
#define IOT_TP_JOBS_DEFAULT 0
#define IOT_TP_SHUTDOWN_MIN 200

#ifdef IOT_BUILD_COMPONENTS
#define IOT_THREADPOOL_FACTORY iot_threadpool_factory ()
#else
#define IOT_THREADPOOL_FACTORY NULL
#endif

typedef struct iot_job_t
{
  struct iot_job_t * prev;           // Pointer to previous job
  void * (*function) (void * arg);   // Function to run
  void * arg;                        // Function's argument
  int priority;                      // Job priority
  uint32_t id;                       // Job id
} iot_job_t;

typedef struct iot_thread_t
{
  uint16_t id;                       // Thread number
  pthread_t tid;                     // Thread id
  struct iot_threadpool_t * pool;    // Thread pool
  bool pending_delete;               // Finalise thread pool deletion on thread exit
  bool deleted;                      // Mark thread as exited
} iot_thread_t;

typedef struct iot_threadpool_t
{
  iot_component_t component;         // Component base type
  iot_thread_t * thread_array;       // Array of threads
  const uint32_t max_jobs;           // Maximum number of queued jobs
  const uint16_t id;                 // Thread pool id
  uint16_t working;                  // Number of threads currently working
  uint16_t threads;                  // Number of threads allocated
  _Atomic uint16_t created;          // Number of threads created
  uint32_t jobs;                     // Number of jobs in queue
  uint32_t delay;                    // Shutdown delay in milliseconds
  uint32_t next_id;                  // Job id counter
  iot_job_t * front;                 // Front of job queue
  iot_job_t * rear;                  // Rear of job queue
  iot_job_t * cache;                 // Free job cache
  int affinity;                      // Pool threads processor affinity
  pthread_cond_t work_cond;          // Work control condition
  pthread_cond_t job_cond;           // Job control condition
  pthread_cond_t queue_cond;         // Job queue control condition
  iot_logger_t * logger;             // Optional logger
} iot_threadpool_t;

static void iot_threadpool_final_free (iot_threadpool_t * pool)
{
  pthread_cond_destroy (&pool->work_cond);
  pthread_cond_destroy (&pool->queue_cond);
  pthread_cond_destroy (&pool->job_cond);
  iot_logger_free (pool->logger);
  free (pool->thread_array);
  iot_component_fini (&pool->component);
  free (pool);
}

static void * iot_threadpool_thread (void * arg)
{
  iot_thread_t * th = (iot_thread_t*) arg;
  iot_threadpool_t * pool = th->pool;
  iot_component_t * comp = &pool->component;
  pthread_t tid = pthread_self ();
  int priority = iot_thread_get_priority (tid);
  char name[IOT_PRCTL_NAME_MAX];
  iot_component_state_t state;
  bool pending_delete = false;

  snprintf (name, IOT_PRCTL_NAME_MAX, "iot-%" PRIu16 "-%" PRIu16, th->pool->id, th->id);
#ifdef IOT_HAS_PRCTL
  prctl (PR_SET_NAME, name);
#endif
  iot_log_debug (pool->logger, "Thread %s #%" PRIu16 " starting", name, th->id);

  atomic_fetch_add (&pool->created, 1u);
  while (true)
  {
    state = iot_component_wait_and_lock (comp, (uint32_t) IOT_COMPONENT_DELETED | (uint32_t) IOT_COMPONENT_RUNNING);

    if (state == IOT_COMPONENT_DELETED) // Exit thread on deletion
    {
      pending_delete = th->pending_delete;
      th->deleted = true;
      iot_component_unlock (comp);
      break;
    }
    iot_job_t * first = pool->front;
    if (first) // Pull job from queue
    {
      iot_job_t job = *first;
      iot_log_trace (pool->logger, "Thread %" PRIu16 " processing job %" PRIu32, th->id, job.id);
      pool->front = first->prev;
      first->prev = pool->cache;
      pool->cache = first;
      if (--pool->jobs == 0)
      {
        pool->front = NULL;
        pool->rear = NULL;
        pthread_cond_signal (&pool->work_cond); // Signal no jobs in queue
      }
      if ((pool->jobs + 1) == pool->max_jobs)
      {
        pthread_cond_broadcast (&pool->queue_cond); // Signal now space in job queue
      }
      pool->working++;
      iot_component_unlock (comp);
      if ((job.priority != IOT_THREAD_NO_PRIORITY) && (job.priority != priority)) // If required, set thread priority
      {
        if (iot_thread_set_priority (tid, job.priority))
        {
          priority = job.priority;
        }
      }
      (job.function) (job.arg); // Run job
      iot_log_trace (pool->logger, "Thread %" PRIu16 " completed job %" PRIu32, th->id, job.id);
      iot_component_lock (comp);
      if (--pool->working == 0)
      {
        pthread_cond_signal (&pool->work_cond); // Signal when no threads working
      }
    }
    else
    {
      iot_log_trace (pool->logger, "Thread %" PRIu16 " waiting for new job", th->id);
      pthread_cond_wait (&pool->job_cond, &comp->mutex); // Wait for new job
    }
    iot_component_unlock (comp);
  }
  iot_log_debug (pool->logger, "Thread %" PRIu16 " exiting", th->id);
  atomic_fetch_sub (&pool->created, 1u);
  if (pending_delete) iot_threadpool_final_free (pool);
  return NULL;
}

iot_threadpool_t * iot_threadpool_alloc (uint16_t threads, uint32_t max_jobs, int default_prio, int affinity, iot_logger_t * logger)
{
  static _Atomic uint16_t pool_id = ATOMIC_VAR_INIT (0);

  uint16_t created;
  iot_threadpool_t * pool = (iot_threadpool_t*) calloc (1, sizeof (*pool));
  pool->affinity = affinity;
  pool->logger = logger;
  *((uint16_t*) &pool->id) = (uint16_t) atomic_fetch_add (&pool_id, 1u);
  iot_logger_add_ref (logger);
  iot_log_info (logger, "iot_threadpool_alloc (threads: %" PRIu16 " max_jobs: %u default_priority: %d affinity: %d)", threads, max_jobs, default_prio, affinity);
  pool->thread_array = (iot_thread_t*) calloc (threads, sizeof (iot_thread_t));
  *((uint32_t*) &pool->max_jobs) = max_jobs ? max_jobs : UINT32_MAX;
  pool->delay = IOT_TP_SHUTDOWN_MIN;
  atomic_store (&pool->created, 0u);
  pthread_cond_init (&pool->work_cond, NULL);
  pthread_cond_init (&pool->queue_cond, NULL);
  pthread_cond_init (&pool->job_cond, NULL);
  iot_component_init (&pool->component, IOT_THREADPOOL_FACTORY, (iot_component_start_fn_t) iot_threadpool_start, (iot_component_stop_fn_t) iot_threadpool_stop);
  pool->threads = threads;
  for (created = 0; created < threads; created++)
  {
    iot_thread_t * th = &pool->thread_array[created];
    th->pool = pool;
    th->id = created;
    if (! iot_thread_create (&th->tid, iot_threadpool_thread, th, default_prio, affinity, logger))
    {
      break;
    }
  }
  while (atomic_load (&pool->created) != created)
  {
    iot_wait_usecs (100); /* Wait until all threads running */
  }
  return pool;
}

void iot_threadpool_add_ref (iot_threadpool_t * pool)
{
  if (pool) iot_component_add_ref (&pool->component);
}

static void iot_threadpool_add_work_locked (iot_threadpool_t * pool, void * (*func) (void*), void * arg, int prio)
{
  iot_job_t * job = pool->cache;
  if (job)
  {
    pool->cache = job->prev;
  }
  else
  {
    job = malloc (sizeof (*job));
  }
  job->function = func;
  job->arg = arg;
  job->priority = prio;
  job->prev = NULL;
  job->id = pool->next_id++;
  iot_log_trace (pool->logger, "Added new job #%u", job->id);

  if (job->priority != IOT_THREAD_NO_PRIORITY) // Order job by priority
  {
    iot_job_t * iter = pool->front;
    iot_job_t * prev = NULL;
    while (iter)
    {
      if (iter->priority == IOT_THREAD_NO_PRIORITY || iter->priority < job->priority)
      {
        job->prev = iter;
        if (prev)
        {
          prev->prev = job;
        }
        else
        {
          pool->front = job;
        }
        goto DONE;
      }
      prev = iter;
      iter = iter->prev;
    }
  }
  job->prev = NULL; // Add job to back of queue
  if (pool->rear)
  {
    pool->rear->prev = job;
  }
  pool->rear = job;
  if (pool->front == NULL)
  {
    pool->front = job;
  }

DONE:

  pool->jobs++;
  pthread_cond_signal (&pool->job_cond); // Signal new job added
}

bool iot_threadpool_try_work (iot_threadpool_t * pool, void * (*func) (void*), void * arg, int prio)
{
  assert (pool && func);
  iot_log_trace (pool->logger, "iot_threadpool_try_work");
  bool ret = false;
  iot_component_lock (&pool->component);
  if (pool->jobs < pool->max_jobs)
  {
    iot_threadpool_add_work_locked (pool, func, arg, prio);
    ret = true;
  }
  iot_component_unlock (&pool->component);
  return ret;
}

void iot_threadpool_add_work (iot_threadpool_t * pool, void * (*func) (void*), void * arg, int prio)
{
  assert (pool && func);
  iot_log_trace (pool->logger, "iot_threadpool_add_work");
  iot_component_lock (&pool->component);
  while (pool->jobs == pool->max_jobs)
  {
    iot_log_debug (pool->logger, "iot_threadpool_add_work jobs at max (%u), waiting for job completion", pool->max_jobs);
    pthread_cond_wait (&pool->queue_cond, &pool->component.mutex); // Wait until space in job queue
  }
  iot_threadpool_add_work_locked (pool, func, arg, prio);
  iot_log_debug (pool->logger, "iot_threadpool_add_work jobs/max: %u/%u", pool->jobs, pool->max_jobs);
  iot_component_unlock (&pool->component);
}

void iot_threadpool_wait (iot_threadpool_t * pool)
{
  assert (pool);
  iot_log_trace (pool->logger, "iot_threadpool_wait");
  iot_component_lock (&pool->component);
  while (pool->jobs || pool->working)
  {
    iot_log_debug (pool->logger, "iot_threadpool_wait (jobs:%u active threads:%u)", pool->jobs, pool->working);
    pthread_cond_wait (&pool->work_cond, &pool->component.mutex); // Wait until all jobs processed
  }
  iot_component_unlock (&pool->component);
}

void iot_threadpool_stop (iot_threadpool_t * pool)
{
  assert (pool);
  iot_log_trace (pool->logger, "iot_threadpool_stop");
  iot_component_set_stopped (&pool->component);
  iot_component_lock (&pool->component);
  pthread_cond_broadcast (&pool->job_cond);
  iot_component_unlock (&pool->component);
}

void iot_threadpool_start (iot_threadpool_t * pool)
{
  assert (pool);
  iot_log_trace (pool->logger, "iot_threadpool_start");
  iot_component_set_running (&pool->component);
  iot_component_lock (&pool->component);
  pthread_cond_broadcast (&pool->job_cond);
  iot_component_unlock (&pool->component);
}

void iot_threadpool_free (iot_threadpool_t * pool)
{
  if (pool && iot_component_dec_ref (&pool->component))
  {
    iot_job_t * job;
    bool self_delete = false;
    pthread_t self = pthread_self ();
    iot_log_trace (pool->logger, "iot_threadpool_free");
    iot_component_lock (&pool->component);
    for (uint16_t i = 0; i < pool->threads; i++)
    {
      self_delete = !pool->thread_array[i].deleted && pthread_equal (pool->thread_array[i].tid, self);
      if (self_delete)
      {
        pool->thread_array[i].pending_delete = true;
        break;
      }
    }
    iot_component_unlock (&pool->component);
    if (self_delete) iot_log_debug (pool->logger, "pool %p self delete", pool);
    iot_threadpool_stop (pool);
    iot_component_set_deleted (&pool->component);
    while (atomic_load (&pool->created) > (self_delete ? 1 : 0))
    {
      iot_wait_msecs (pool->delay);
    }
    while ((job = pool->cache))
    {
      pool->cache = job->prev;
      free (job);
    }
    while ((job = pool->front))
    {
      pool->front = job->prev;
      free (job);
    }
    if (!self_delete) iot_threadpool_final_free (pool);
  }
}

#ifdef IOT_BUILD_COMPONENTS

static iot_component_t * iot_threadpool_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * logger = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Logger"));
  uint16_t threads = (uint16_t) iot_data_string_map_get_i64 (map, "Threads", IOT_TP_THREADS_DEFAULT);
  uint32_t jobs = (uint32_t) iot_data_string_map_get_i64 (map, "MaxJobs", IOT_TP_JOBS_DEFAULT);
  int prio = (int) iot_data_string_map_get_i64 (map, "Priority", IOT_THREAD_NO_PRIORITY);
  int affinity = (int) iot_data_string_map_get_i64 (map, "Affinity", IOT_THREAD_NO_AFFINITY);
  uint32_t delay = (uint32_t) iot_data_string_map_get_i64 (map, "ShutdownDelay", IOT_TP_SHUTDOWN_MIN);
  iot_threadpool_t * pool = iot_threadpool_alloc (threads, jobs, prio, affinity, logger);
  pool->delay = (delay < IOT_TP_SHUTDOWN_MIN) ? IOT_TP_SHUTDOWN_MIN : delay;
  return &pool->component;
}

const iot_component_factory_t * iot_threadpool_factory (void)
{
  static iot_component_factory_t factory =
  {
    IOT_THREADPOOL_TYPE,
    IOT_CATEGORY_CORE,
    iot_threadpool_config,
    (iot_component_free_fn_t) iot_threadpool_free,
    NULL,
    NULL
  };
  return &factory;
}
#endif
