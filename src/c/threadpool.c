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

#ifdef THPOOL_DEBUG
#define err(str) fprintf (stderr, str)
#else
#define err(str)
#endif

#define IOT_THREADPOOL_THREADS_DEFAULT 2
#define IOT_THREADPOOL_JOBS_DEFAULT 0

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
  uint32_t id;                       // Thread number
  pthread_t pthread;                 // Thread id
  struct iot_threadpool_t * pool;    // Thread pool
} iot_thread_t;

typedef struct iot_threadpool_t
{
  iot_component_t component;         // Component base type
  iot_thread_t * threads;            // Array of threads
  uint32_t max_threads;              // Maximum number of threads
  uint32_t max_jobs;                 // Maximum number of queued jobs
  uint32_t jobs;                     // Number of jobs in queue
  uint32_t threads_alive;            // Threads currently alive
  uint32_t threads_working;          // Threads currently working
  iot_job_t * front;                 // Pointer to front of job queue
  iot_job_t * rear;                  // Pointer to rear of job queue
  iot_job_t * cache;                 // Free job cache
  const int * default_prio;          // Default thread priority
  pthread_mutex_t mutex;             // Concurrency guard mutex
  pthread_cond_t thread_cond;        // Thread control condition
  pthread_cond_t queue_cond;         // Job queue control condition
} iot_threadpool_t;

static void iot_threadpool_pull_locked (iot_threadpool_t * pool, iot_job_t * job)
{
  iot_job_t * first = pool->front;
  job->function = NULL;
  if (first)
  {
    *job = *first;
    pool->front = first->prev;
    first->prev = pool->cache;
    pool->cache = first;
    if (pool->jobs == pool->max_jobs)
    {
      pthread_cond_signal (&pool->queue_cond);
    }
    if (--pool->jobs == 0) // Queue now empty
    {
      pool->front = NULL;
      pool->rear = NULL;
    }
    else // Still jobs in queue, so wake thread
    {
      pthread_cond_signal (&pool->thread_cond);
    }
  }
}

/*
* Thread entry function. Loops processing jobs until pool is stopped.
*
* @param  th        thread that will run this function
* @return NULL
*/
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
  if (++pool->threads_alive == pool->max_threads)
  {
    pthread_cond_signal (&pool->thread_cond); // Signal when all threads running
  }
  while (pool->component.state == IOT_COMPONENT_RUNNING)
  {
    iot_job_t job;
    iot_threadpool_pull_locked (pool, &job); // Pull job from queue
    if (job.function)
    {
      pool->threads_working++;
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
      if (--pool->threads_working == 0)
      {
        pthread_cond_signal (&pool->thread_cond); // Signal when no threads working
      }
    }
    else
    {
      pthread_cond_wait (&pool->thread_cond, &pool->mutex); // Wait for new job
    }
  }
  if (--pool->threads_alive == 0)
  {
    pthread_cond_signal (&pool->thread_cond); // Signal when all threads done
  }
  pthread_mutex_unlock (&pool->mutex);
  return NULL;
}

iot_threadpool_t * iot_threadpool_alloc (uint32_t threads, uint32_t max_jobs, const int * default_prio)
{
  iot_threadpool_t * pool = (iot_threadpool_t*) calloc (1, sizeof (*pool));
  pool->threads = (iot_thread_t*) calloc (threads, sizeof (iot_thread_t));
  pool->max_threads = threads;
  pool->default_prio = default_prio;
  pool->max_jobs = max_jobs ? max_jobs : UINT32_MAX;
  iot_mutex_init (&pool->mutex);
  pthread_cond_init (&pool->thread_cond, NULL);
  pthread_cond_init (&pool->queue_cond, NULL);
  pool->component.start_fn = (iot_component_start_fn_t) iot_threadpool_start;
  pool->component.stop_fn = (iot_component_stop_fn_t) iot_threadpool_stop;
  return pool;
}

/* Add work to the thread pool, ordering job by priority  */
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

  if (job->prio_set) // Sort job by priority
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
  pthread_cond_signal (&pool->thread_cond); // Signal new job added
}

bool iot_threadpool_try_work (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
{
  assert (pool && func);
  bool ret = false;
  pthread_mutex_lock (&pool->mutex);
  if (pool->component.state == IOT_COMPONENT_RUNNING)
  {
    if (pool->jobs < pool->max_jobs)
    {
      iot_threadpool_add_job_locked (pool, func, arg, prio);
      ret = true;
    }
  }
  pthread_mutex_unlock (&pool->mutex);
  return ret;
}

void iot_threadpool_add_work (iot_threadpool_t * pool, void (*func) (void*), void * arg, const int * prio)
{
  assert (pool && func);
  pthread_mutex_lock (&pool->mutex);
  if (pool->component.state == IOT_COMPONENT_RUNNING)
  {
    if (pool->jobs == pool->max_jobs)
    {
      pthread_cond_wait (&pool->queue_cond, &pool->mutex);
    }
    iot_threadpool_add_job_locked (pool, func, arg, prio);
  }
  pthread_mutex_unlock (&pool->mutex);
}

/* Wait until all jobs have finished */
void iot_threadpool_wait (iot_threadpool_t * pool)
{
  pthread_mutex_lock (&pool->mutex);
  while (pool->jobs || pool->threads_working)
  {
    pthread_cond_wait (&pool->thread_cond, &pool->mutex);
  }
  pthread_mutex_unlock (&pool->mutex);
}

void iot_threadpool_stop (iot_threadpool_t * pool)
{
  pthread_mutex_lock (&pool->mutex);
  if (pool->component.state != IOT_COMPONENT_STOPPED)
  {
    pool->component.state = IOT_COMPONENT_STOPPED;
  }
  while (pool->threads_alive)
  {
    pthread_cond_broadcast (&pool->thread_cond);
    pthread_mutex_unlock (&pool->mutex);
    sleep (1);
    pthread_mutex_lock (&pool->mutex);
  }
  pthread_mutex_unlock (&pool->mutex);
}

bool iot_threadpool_start (iot_threadpool_t * pool)
{
  pthread_mutex_lock (&pool->mutex);
  if (pool->component.state != IOT_COMPONENT_RUNNING)
  {
    pool->component.state = IOT_COMPONENT_RUNNING;
    for (uint32_t n = 0; n < pool->max_threads; n++)
    {
      iot_thread_t *th = &pool->threads[n];
      th->pool = pool;
      th->id = n;
      iot_thread_create (&th->pthread, (iot_thread_fn_t) iot_threadpool_thread, th, pool->default_prio);
    }
    while (pool->threads_alive != pool->max_threads) // Wait for all threads to start
    {
      pthread_cond_wait (&pool->thread_cond, &pool->mutex);
    }
  }
  pthread_mutex_unlock (&pool->mutex);
  return true;
}

void iot_threadpool_free (iot_threadpool_t * pool)
{
  if (pool)
  {
    iot_job_t * job;
    iot_threadpool_stop (pool);
    pthread_mutex_lock (&pool->mutex);
    while ((job = pool->cache))
    {
      pool->cache = job->prev;
      free (job);
    }
    pthread_mutex_unlock (&pool->mutex);
    pthread_cond_destroy (&pool->thread_cond);
    pthread_cond_destroy (&pool->queue_cond);
    pthread_mutex_destroy (&pool->mutex);
    free (pool->threads);
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
