//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/threadpool.h"
#include "iot/thread.h"
#include "iot/data.h"

#if defined (__linux__)
#include <sys/prctl.h>
#endif

#define IOT_THREADPOOL_THREADS_DEFAULT 2
#define IOT_THREADPOOL_JOBS_DEFAULT 0
#define IOT_THREADPOOL_SHUTDOWN_DELAY 200000

typedef struct iot_job_t
{
  struct iot_job_t * prev;           // Pointer to previous job
  void (*function) (void * arg);     // Function to run
  void * arg;                        // Function's argument
  int priority;                      // Job priority
  bool prio_set;                     // Whether priority set
} iot_job_t;

typedef struct iot_thread_t
{
  const uint32_t id;                 // Thread number
  pthread_t pthread;                 // Thread id
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
  iot_job_t * front;                 // Front of job queue
  iot_job_t * rear;                  // Rear of job queue
  iot_job_t * cache;                 // Free job cache
  const int * default_prio;          // Default thread priority
  pthread_mutex_t mutex;             // Concurrency guard mutex
  pthread_cond_t work_cond;          // Work control condition
  pthread_cond_t job_cond;           // Job control condition
  pthread_cond_t queue_cond;         // Job queue control condition
  pthread_cond_t state_cond;         // Service state control condition
} iot_threadpool_t;

static void * iot_threadpool_thread (iot_thread_t * th)
{
  iot_threadpool_t * pool = th->pool;
  pthread_t tid = pthread_self ();
  int priority = iot_thread_get_priority (tid);

#if defined (__linux__)
  char thread_name[64];
  sprintf (thread_name, "thread-pool-%u", th->id);
  prctl (PR_SET_NAME, thread_name);
#endif

  pthread_mutex_lock (&pool->mutex);
  while (true)
  {
    while (pool->component.state < IOT_COMPONENT_RUNNING)
    {
      pthread_cond_wait (&pool->state_cond, &pool->mutex); // Wait until running or deleted
    }
    if (pool->component.state == IOT_COMPONENT_DELETED)
    {
      break; // Exit thread on deletion
    }
    iot_job_t * first = pool->front;
    if (first) // Pull job from queue
    {
      iot_job_t job = *first;
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
      pthread_mutex_unlock (&pool->mutex);
      if (job.prio_set && (job.priority != priority)) // If required, set thread priority
      {
        if (iot_thread_set_priority (tid, job.priority))
        {
          priority = job.priority;
        }
      }
      (job.function) (job.arg); // Run job
      pthread_mutex_lock (&pool->mutex);
      if (--pool->working == 0)
      {
        pthread_cond_signal (&pool->work_cond); // Signal when no threads working
      }
    }
    else
    {
      pthread_cond_wait (&pool->job_cond, &pool->mutex); // Wait for new job
    }
  }
  pthread_mutex_unlock (&pool->mutex);
  return NULL;
}

iot_threadpool_t * iot_threadpool_alloc (uint32_t threads, uint32_t max_jobs, const int * default_prio)
{
  iot_threadpool_t * pool = (iot_threadpool_t*) calloc (1, sizeof (*pool));
  pool->thread_array = (iot_thread_t*) calloc (threads, sizeof (iot_thread_t));
  *(uint32_t*) &pool->max_threads = threads;
  *(uint32_t*) &pool->max_jobs = max_jobs ? max_jobs : UINT32_MAX;
  pool->default_prio = default_prio;
  iot_mutex_init (&pool->mutex);
  pthread_cond_init (&pool->work_cond, NULL);
  pthread_cond_init (&pool->queue_cond, NULL);
  pthread_cond_init (&pool->job_cond, NULL);
  pthread_cond_init (&pool->state_cond, NULL);
  pool->component.start_fn = (iot_component_start_fn_t) iot_threadpool_start;
  pool->component.stop_fn = (iot_component_stop_fn_t) iot_threadpool_stop;
  for (uint32_t n = 0; n < pool->max_threads; n++)
  {
    iot_thread_t *th = &pool->thread_array[n];
    th->pool = pool;
    *(uint32_t*) &th->id = n;
    iot_thread_create (&th->pthread, (iot_thread_fn_t) iot_threadpool_thread, th, pool->default_prio);
  }
  return pool;
}

static void iot_threadpool_add_job_locked (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
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
  bool ret = false;
  pthread_mutex_lock (&pool->mutex);
  if (pool->jobs < pool->max_jobs)
  {
    iot_threadpool_add_job_locked (pool, func, arg, prio);
    ret = true;
  }
  pthread_mutex_unlock (&pool->mutex);
  return ret;
}

void iot_threadpool_add_work (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
{
  assert (pool && func);
  pthread_mutex_lock (&pool->mutex);
  if (pool->jobs == pool->max_jobs)
  {
    pthread_cond_wait (&pool->queue_cond, &pool->mutex); // Wait until space in job queue
  }
  iot_threadpool_add_job_locked (pool, func, arg, prio);
  pthread_mutex_unlock (&pool->mutex);
}

static inline void iot_threadpool_wait_locked (iot_threadpool_t * pool)
{
  while (pool->jobs || pool->working)
  {
    pthread_cond_wait (&pool->work_cond, &pool->mutex); // Wait until all jobs processed
  }
}

void iot_threadpool_wait (iot_threadpool_t * pool)
{
  assert (pool);
  pthread_mutex_lock (&pool->mutex);
  iot_threadpool_wait_locked (pool);
  pthread_mutex_unlock (&pool->mutex);
}

static inline void iot_threadpool_stop_locked (iot_threadpool_t * pool)
{
  if (pool->component.state != IOT_COMPONENT_STOPPED)
  {
    pool->component.state = IOT_COMPONENT_STOPPED;
    pthread_cond_broadcast (&pool->state_cond);
    pthread_cond_broadcast (&pool->job_cond);
  }
}

void iot_threadpool_stop (iot_threadpool_t * pool)
{
  assert (pool);
  pthread_mutex_lock (&pool->mutex);
  iot_threadpool_stop_locked (pool);
  pthread_mutex_unlock (&pool->mutex);
}

bool iot_threadpool_start (iot_threadpool_t * pool)
{
  assert (pool);
  pthread_mutex_lock (&pool->mutex);
  if (pool->component.state != IOT_COMPONENT_RUNNING)
  {
    pool->component.state = IOT_COMPONENT_RUNNING;
    pthread_cond_broadcast (&pool->state_cond);
    pthread_cond_broadcast (&pool->job_cond);
  }
  pthread_mutex_unlock (&pool->mutex);
  return true;
}

void iot_threadpool_free (iot_threadpool_t * pool)
{
  if (pool)
  {
    iot_job_t * job;
    pthread_mutex_lock (&pool->mutex);
    iot_threadpool_wait_locked (pool);
    iot_threadpool_stop_locked (pool);
    pool->component.state = IOT_COMPONENT_DELETED;
    pthread_cond_broadcast (&pool->state_cond);
    while ((job = pool->cache))
    {
      pool->cache = job->prev;
      free (job);
    }
    pthread_mutex_unlock (&pool->mutex);
    usleep (IOT_THREADPOOL_SHUTDOWN_DELAY);
    pthread_cond_destroy (&pool->work_cond);
    pthread_cond_destroy (&pool->queue_cond);
    pthread_cond_destroy (&pool->job_cond);
    pthread_cond_destroy (&pool->state_cond);
    pthread_mutex_destroy (&pool->mutex);
    free (pool->thread_array);
    free (pool);
  }
}

/* Container support */

static iot_component_t * iot_threadpool_config (iot_container_t * cont, const iot_data_t * map)
{
  (void) cont;
  const iot_data_t * value = iot_data_string_map_get (map, "Threads");
  uint32_t threads = value ? (uint32_t) iot_data_i64 (value) : IOT_THREADPOOL_THREADS_DEFAULT;
  value = iot_data_string_map_get (map, "MaxJobs");
  uint32_t jobs = value ? (uint32_t) iot_data_i64 (value) : IOT_THREADPOOL_JOBS_DEFAULT;
  value = iot_data_string_map_get (map, "Priority");
  int prio = value ? (int) iot_data_i64 (value) : -1;
  return (iot_component_t*) iot_threadpool_alloc (threads, jobs, value ? &prio : NULL);
}

const iot_component_factory_t * iot_threadpool_factory (void)
{
  static iot_component_factory_t factory = { IOT_THREADPOOL_TYPE, iot_threadpool_config, (iot_component_free_fn_t) iot_threadpool_free };
  return &factory;
}
