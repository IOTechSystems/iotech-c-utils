//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/container.h"
#include "iot/threadpool.h"
#include "iot/thread.h"
#include "iot/data.h"

#if defined (__linux__)
#include <sys/prctl.h>
#endif

#define IOT_PRCTL_NAME_MAX 16
#define IOT_TP_THREADS_DEFAULT 2
#define IOT_TP_JOBS_DEFAULT 0
#define IOT_TP_SHUTDOWN_MIN 200

typedef struct iot_job_t
{
  struct iot_job_t * prev;           // Pointer to previous job
  void (*function) (void * arg);     // Function to run
  void * arg;                        // Function's argument
  int priority;                      // Job priority
  uint32_t id;                       // Job id
  bool prio_set;                     // Whether priority set
} iot_job_t;

typedef struct iot_thread_t
{
  const uint32_t id;                 // Thread number
  pthread_t tid;                     // Thread id
  struct iot_threadpool_t * pool;    // Thread pool
} iot_thread_t;

typedef struct iot_threadpool_t
{
  iot_component_t component;         // Component base type
  iot_thread_t * thread_array;       // Array of threads
  const uint32_t max_threads;        // Maximum number of threads
  const uint32_t max_jobs;           // Maximum number of queued jobs
  uint32_t jobs;                     // Number of jobs in queue
  uint32_t working;                  // Number of threads currently working
  uint32_t delay;                    // Shutdown delay in milli seconds
  uint32_t next_id;                  // Job id counter
  iot_job_t * front;                 // Front of job queue
  iot_job_t * rear;                  // Rear of job queue
  iot_job_t * cache;                 // Free job cache
  const int * default_prio;          // Default thread priority
  pthread_cond_t work_cond;          // Work control condition
  pthread_cond_t job_cond;           // Job control condition
  pthread_cond_t queue_cond;         // Job queue control condition
  iot_logger_t * logger;             // Optional logger
} iot_threadpool_t;

static void * iot_threadpool_thread (void * arg)
{
  iot_thread_t * th = (iot_thread_t*) arg;
  iot_threadpool_t * pool = th->pool;
  pthread_t tid = pthread_self ();
  int priority = iot_thread_get_priority (tid);
  char name[IOT_PRCTL_NAME_MAX];
  iot_component_state_t state;

  snprintf (name, IOT_PRCTL_NAME_MAX, "iot-%u", th->id);
  iot_log_debug (pool->logger, "Thread %s starting", name);

#if defined (__linux__)
  prctl (PR_SET_NAME, name);
#endif

  while (true)
  {
    state = iot_component_wait_and_lock (&pool->component, IOT_COMPONENT_DELETED | IOT_COMPONENT_RUNNING);

    if (state == IOT_COMPONENT_DELETED)
    {
      break; // Exit thread on deletion
    }
    iot_job_t * first = pool->front;
    if (first) // Pull job from queue
    {
      iot_job_t job = *first;
      iot_log_debug (pool->logger, "Thread processing job #%u", job.id);
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
      pthread_mutex_unlock (&pool->component.mutex);
      if (job.prio_set && (job.priority != priority)) // If required, set thread priority
      {
        if (iot_thread_set_priority (tid, job.priority))
        {
          priority = job.priority;
        }
      }
      (job.function) (job.arg); // Run job
      iot_log_debug (pool->logger, "Thread completed job #%u", job.id);
      pthread_mutex_lock (&pool->component.mutex);
      if (--pool->working == 0)
      {
        pthread_cond_signal (&pool->work_cond); // Signal when no threads working
      }
      pthread_mutex_unlock (&pool->component.mutex);
    }
    else
    {
      iot_log_debug (pool->logger, "Thread waiting for new job");
      pthread_cond_wait (&pool->job_cond, &pool->component.mutex); // Wait for new job
      pthread_mutex_unlock (&pool->component.mutex);
    }
  }
  pthread_mutex_unlock (&pool->component.mutex);
  iot_log_debug (pool->logger, "Thread exiting", name);
  return NULL;
}

iot_threadpool_t * iot_threadpool_alloc (uint32_t threads, uint32_t max_jobs, const int * default_prio, iot_logger_t * logger)
{
  iot_threadpool_t * pool = (iot_threadpool_t*) calloc (1, sizeof (*pool));
  pool->logger = logger;
  iot_logger_add_ref (logger);
  iot_log_info (logger, "iot_threadpool_alloc (threads: %u max jobs: %u)", threads, max_jobs);
  pool->thread_array = (iot_thread_t*) calloc (threads, sizeof (iot_thread_t));
  *(uint32_t*) &pool->max_threads = threads;
  *(uint32_t*) &pool->max_jobs = max_jobs ? max_jobs : UINT32_MAX;
  pool->default_prio = default_prio;
  pool->delay = IOT_TP_SHUTDOWN_MIN;
  iot_mutex_init (&pool->component.mutex);
  pthread_cond_init (&pool->work_cond, NULL);
  pthread_cond_init (&pool->queue_cond, NULL);
  pthread_cond_init (&pool->job_cond, NULL);
  iot_component_init (&pool->component, (iot_component_start_fn_t) iot_threadpool_start, (iot_component_stop_fn_t) iot_threadpool_stop);
  for (uint32_t n = 0; n < pool->max_threads; n++)
  {
    iot_thread_t * th = &pool->thread_array[n];
    th->pool = pool;
    *(uint32_t*) &th->id = n;
    iot_thread_create (&th->tid, iot_threadpool_thread, th, pool->default_prio);
  }
  return pool;
}

void iot_threadpool_add_ref (iot_threadpool_t * pool)
{
  assert (pool);
  iot_component_add_ref (&pool->component);
}

static void iot_threadpool_add_work_locked (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
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
  job->priority = prio ? *prio : 0;
  job->prio_set = (prio != NULL);
  job->prev = NULL;
  job->id = pool->next_id++;
  iot_log_debug (pool->logger, "Added new job #%u", job->id);

  if (job->prio_set) // Order job by priority
  {
    iot_job_t * iter = pool->front;
    iot_job_t * prev = NULL;
    while (iter)
    {
      if (! iter->prio_set || iter->priority < job->priority)
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
        goto added;
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

added:

  pool->jobs++;
  pthread_cond_signal (&pool->job_cond); // Signal new job added
}

bool iot_threadpool_try_work (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
{
  assert (pool && func);
  iot_log_trace (pool->logger, "iot_threadpool_try_work()");
  bool ret = false;
  pthread_mutex_lock (&pool->component.mutex);
  if (pool->jobs < pool->max_jobs)
  {
    iot_threadpool_add_work_locked (pool, func, arg, prio);
    ret = true;
  }
  pthread_mutex_unlock (&pool->component.mutex);
  return ret;
}

void iot_threadpool_add_work (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
{
  assert (pool && func);
  iot_log_trace (pool->logger, "iot_threadpool_add_work()");
  pthread_mutex_lock (&pool->component.mutex);
  if (pool->jobs == pool->max_jobs)
  {
    iot_log_debug (pool->logger, "iot_threadpool_add_work jobs at max (%u), waiting for job completion", pool->max_jobs);
    pthread_cond_wait (&pool->queue_cond, &pool->component.mutex); // Wait until space in job queue
  }
  iot_threadpool_add_work_locked (pool, func, arg, prio);
  iot_log_debug (pool->logger, "iot_threadpool_add_work jobs/max: %u/%u", pool->jobs, pool->max_jobs);
  pthread_mutex_unlock (&pool->component.mutex);
}

static inline void iot_threadpool_wait_locked (iot_threadpool_t * pool)
{
  while (pool->jobs || pool->working)
  {
    iot_log_debug (pool->logger, "iot_threadpool_wait (jobs:%u active threads:%u)", pool->jobs, pool->working);
    pthread_cond_wait (&pool->work_cond, &pool->component.mutex); // Wait until all jobs processed
  }
}

void iot_threadpool_wait (iot_threadpool_t * pool)
{
  assert (pool);
  iot_log_trace (pool->logger, "iot_threadpool_wait()");
  pthread_mutex_lock (&pool->component.mutex);
  iot_threadpool_wait_locked (pool);
  pthread_mutex_unlock (&pool->component.mutex);
}

void iot_threadpool_stop (iot_threadpool_t * pool)
{
  assert (pool);
  iot_log_trace (pool->logger, "iot_threadpool_stop()");
  iot_component_set_stopped (&pool->component);
  pthread_cond_broadcast (&pool->job_cond);
}

bool iot_threadpool_start (iot_threadpool_t * pool)
{
  assert (pool);
  iot_log_trace (pool->logger, "iot_threadpool_start()");
  iot_component_set_running (&pool->component);
  pthread_cond_broadcast (&pool->job_cond);
  return true;
}

void iot_threadpool_free (iot_threadpool_t * pool)
{
  if (pool && iot_component_dec_ref (&pool->component))
  {
    iot_job_t * job;
    iot_log_trace (pool->logger, "iot_threadpool_free()");
    iot_threadpool_stop (pool);
    iot_component_set_deleted (&pool->component);
    usleep (pool->delay * 1000);
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
    pthread_cond_destroy (&pool->work_cond);
    pthread_cond_destroy (&pool->queue_cond);
    pthread_cond_destroy (&pool->job_cond);
    iot_logger_free (pool->logger);
    free (pool->thread_array);
    iot_component_fini (&pool->component);
    free (pool);
  }
}

#ifdef IOT_BUILD_COMPONENTS

static iot_component_t * iot_threadpool_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * logger = NULL;
  iot_threadpool_t * pool;
  const char * name;
  uint32_t threads, jobs, delay;
  const iot_data_t * value = iot_data_string_map_get (map, "Threads");
  threads = value ? (uint32_t) iot_data_i64 (value) : IOT_TP_THREADS_DEFAULT;
  value = iot_data_string_map_get (map, "MaxJobs");
  jobs = value ? (uint32_t) iot_data_i64 (value) : IOT_TP_JOBS_DEFAULT;
  value = iot_data_string_map_get (map, "ShutdownDelay");
  delay = value ? (uint32_t) iot_data_i64 (value) : IOT_TP_SHUTDOWN_MIN;
  if (delay < IOT_TP_SHUTDOWN_MIN) delay = IOT_TP_SHUTDOWN_MIN;
  value = iot_data_string_map_get (map, "Priority");
  int prio = value ? (int) iot_data_i64 (value) : -1;
  name = iot_data_string_map_get_string (map, "Logger");
  if (name) logger = (iot_logger_t*) iot_container_find (cont, name);
  pool = iot_threadpool_alloc (threads, jobs, value ? &prio : NULL, logger);
  pool->delay = delay;
  return &pool->component;
}

const iot_component_factory_t * iot_threadpool_factory (void)
{
  static iot_component_factory_t factory = { IOT_THREADPOOL_TYPE, iot_threadpool_config, (iot_component_free_fn_t) iot_threadpool_free };
  return &factory;
}
#endif