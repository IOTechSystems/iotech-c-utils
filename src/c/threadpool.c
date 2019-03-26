//
// Copyright (c) 2019
// IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
// This file incorporates work covered by the following copyright and
// permission notice:
//
//   Copyright (c) 2016 Johan Hanssen Seferidis SPDX-License-Identifier: MIT
//

#include "iot/threadpool.h"
#include "iot/thread.h"

#if defined(__linux__)
#include <sys/prctl.h>
#endif

#ifdef THPOOL_DEBUG
#define err(str) fprintf (stderr, str)
#else
#define err(str)
#endif

/* Job */
typedef struct iot_job_t
{
  struct iot_job_t * prev;             /* Pointer to previous job */
  void (*function) (void * arg);       /* Function to run         */
  void * arg;                          /* Function's argument     */
  int priority;                        /* Job priority            */
  bool prio_set;                       /* Whether priority set    */
} iot_job_t;

/* Job queue */
typedef struct iot_jobqueue_t
{
  pthread_mutex_t mutex;               /* used for queue r/w access */
  pthread_cond_t cond;                 /* synchronisation condition */
  iot_job_t * front;                   /* pointer to front of queue */
  iot_job_t * rear;                    /* pointer to rear  of queue */
  iot_job_t * cache;                   /* free job  cache           */
  atomic_uint_fast32_t jobs;           /* number of jobs in queue   */
} iot_jobqueue_t;

/* Thread */
typedef struct iot_thread_t
{
  uint32_t id;                         /* friendly id               */
  pthread_t pthread;                   /* pointer to actual thread  */
  struct iot_threadpool_t * pool;      /* thread pool               */
} iot_thread_t;

/* Threadpool */
typedef struct iot_threadpool_t
{
  iot_thread_t * threads;                   /* array of threads           */
  atomic_uint_fast32_t num_threads_alive;   /* threads currently alive    */
  atomic_uint_fast32_t num_threads_working; /* threads currently working  */
  atomic_uint_fast32_t refs;                /* Reference count            */
  pthread_mutex_t mutex;                    /* used for thread count etc  */
  pthread_cond_t cond;                      /* signal to thread pool_wait */
  iot_jobqueue_t jobqueue;                  /* job queue                  */
  atomic_bool running;                      /* state of pool              */
} iot_threadpool_t;


static void * iot_threadpool_thread (iot_thread_t * th);

static void iot_jobqueue_fini (iot_jobqueue_t * jobqueue);
static void iot_jobqueue_push (iot_jobqueue_t * jobqueue, void (*func) (void*), void* arg, const int * prio);
static void iot_jobqueue_pull_locked (iot_jobqueue_t * jobqueue, iot_job_t * job);

static inline void iot_jobqueue_init (iot_jobqueue_t * jobqueue)
{
  iot_mutex_init (&jobqueue->mutex);
  pthread_cond_init (&jobqueue->cond, NULL);
}

/* ========================== THREADPOOL ============================ */

/* Initialise thread pool */
iot_threadpool_t * iot_threadpool_init (uint32_t num_threads, const int * default_prio)
{
  iot_threadpool_t * pool = (iot_threadpool_t*) calloc (1, sizeof (*pool));
  pool->threads = (iot_thread_t*) calloc (num_threads, sizeof (iot_thread_t));

  iot_mutex_init (&pool->mutex);
  pthread_mutex_lock (&pool->mutex);
  atomic_store (&pool->running, true);
  atomic_store (&pool->refs, 1);
  iot_jobqueue_init (&pool->jobqueue);
  pthread_cond_init (&pool->cond, NULL);

  /* Create and start threads */
  for (uint32_t n = 0; n < num_threads; n++)
  {
    iot_thread_t * th = &pool->threads[n];
    th->pool = pool;
    th->id = n;
    iot_thread_create (&th->pthread, (iot_thread_fn_t) iot_threadpool_thread, th, default_prio);

#if THPOOL_DEBUG
    printf ("THPOOL_DEBUG: Created thread %u in pool \n", n);
#endif
  }
  pthread_mutex_unlock (&pool->mutex);

  /* Wait for threads to initialize */
  while (atomic_load (&pool->num_threads_alive) != num_threads)
  {
    usleep (100000);
  }

  return pool;
}

/* Add work to the thread pool */
void iot_threadpool_add_work (iot_threadpool_t * pool, void (*func) (void*), void* arg, const int * prio)
{
  pthread_mutex_lock (&pool->mutex);
  if (atomic_load (&pool->running))
  {
    iot_jobqueue_push (&pool->jobqueue, func, arg, prio);
  }
  pthread_mutex_unlock (&pool->mutex);
}

/* Wait until all jobs have finished */
void iot_threadpool_wait (iot_threadpool_t * pool)
{
  pthread_mutex_lock (&pool->mutex);
  while (atomic_load (&pool->jobqueue.jobs) || atomic_load (&pool->num_threads_working))
  {
    pthread_cond_wait (&pool->cond, &pool->mutex);
  }
  pthread_mutex_unlock (&pool->mutex);
}

/* Increment thread pool reference count */
void iot_threadpool_addref (iot_threadpool_t * pool)
{
  assert (pool);
  atomic_fetch_add (&pool->refs, 1);
}

/* Destroy the threadpool if reference count zero  */
void iot_threadpool_fini (iot_threadpool_t * pool)
{
  if (pool && (atomic_fetch_add (&pool->refs, -1) <= 1))
  {
    /* Terminate threads, then wait for completion */
    atomic_store (&pool->running, false);
    while (atomic_load (&pool->num_threads_alive))
    {
      pthread_cond_broadcast (&pool->jobqueue.cond);
      sleep (1);
    }
    /* Cleanup */
    iot_jobqueue_fini (&pool->jobqueue);
    pthread_cond_destroy (&pool->cond);
    pthread_mutex_destroy (&pool->mutex);
    free (pool->threads);
    free (pool);
  }
}

uint32_t iot_threadpool_num_threads_working (iot_threadpool_t * pool)
{
  return (uint32_t) atomic_load (&pool->num_threads_working);
}

/* ============================ THREAD ============================== */

/* What each thread is doing
*
* In principle this is an endless loop. The only time this loop gets interuppted is once
* iot_threadpool_destroy() is invoked or the program exits.
*
* @param  thread        thread that will run this function
* @return nothing
*/
static void * iot_threadpool_thread (iot_thread_t * th)
{
  iot_threadpool_t * pool = th->pool;
  pthread_t tid = pthread_self ();
  int priority = iot_thread_get_priority (tid);

#if defined (__linux__)
  char thread_name[64];
  sprintf (thread_name, "thread-pool-%u", th->id);
  /* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
  prctl (PR_SET_NAME, thread_name);
#endif

  /* Mark thread as alive (initialized) */

  atomic_fetch_add (&pool->num_threads_alive, 1);
  while (atomic_load (&pool->running))
  {
    iot_job_t job;
    /* Read job from queue and execute it */
    pthread_mutex_lock (&pool->jobqueue.mutex);
    iot_jobqueue_pull_locked (&pool->jobqueue, &job);
    if (job.function)
    {
      pthread_mutex_unlock (&pool->jobqueue.mutex);
      atomic_fetch_add (&pool->num_threads_working, 1);
      /* If required, set thread priority */
      if (job.prio_set && (job.priority != priority))
      {
        if (iot_thread_set_priority (tid, job.priority))
        {
          priority = job.priority;
        }
      }
      (job.function) (job.arg);
      if (atomic_fetch_add (&pool->num_threads_working, -1) <= 1)
      {
        pthread_cond_signal (&pool->cond);
      }
    }
    else
    {
      pthread_cond_wait (&pool->jobqueue.cond, &pool->jobqueue.mutex);
      pthread_mutex_unlock (&pool->jobqueue.mutex);
    }
  }
  if (atomic_fetch_add (&pool->num_threads_alive, -1) <= 1)
  {
    pthread_cond_signal (&pool->cond);
  }
  return NULL;
}

/* ============================ JOB QUEUE =========================== */


static void iot_jobqueue_fini (iot_jobqueue_t * jobqueue)
{
  iot_job_t * job;
  pthread_mutex_lock (&jobqueue->mutex);
  while ((job = jobqueue->cache))
  {
    jobqueue->cache = job->prev;
    free (job);
  }
  pthread_mutex_unlock (&jobqueue->mutex);
  pthread_cond_destroy (&jobqueue->cond);
  pthread_mutex_destroy (&jobqueue->mutex);
}

/* Add new job to queue, ordered by priority */
static void iot_jobqueue_push (iot_jobqueue_t * jobqueue, void (*func) (void*), void * arg, const int * prio)
{
  pthread_mutex_lock (&jobqueue->mutex);
  iot_job_t * job = jobqueue->cache;
  if (job)
  {
    jobqueue->cache = job->prev;
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

  if (job->prio_set)
  {
    iot_job_t * iter = jobqueue->front;
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
          jobqueue->front = job;
        }
        goto added;
      }
      prev = iter;
      iter = iter->prev;
    }
  }

  /* Add to back of queue  */
  job->prev = NULL;
  if (jobqueue->rear)
  {
    jobqueue->rear->prev = job;
  }
  jobqueue->rear = job;
  if (jobqueue->front == NULL)
  {
    jobqueue->front = job;
  }

added:

  atomic_fetch_add (&jobqueue->jobs, 1);
  pthread_cond_signal (&jobqueue->cond);
  pthread_mutex_unlock (&jobqueue->mutex);
}

static void iot_jobqueue_pull_locked (iot_jobqueue_t * jobqueue, iot_job_t * job)
{
  iot_job_t * first = jobqueue->front;
  job->function = NULL;
  if (first)
  {
    *job = *first;
    jobqueue->front = first->prev;
    first->prev = jobqueue->cache;
    jobqueue->cache = first;
    if (atomic_fetch_add (&jobqueue->jobs, -1) <= 1) // Queue now empty
    {
      jobqueue->front = NULL;
      jobqueue->rear = NULL;
    }
    else // Still jobs in queue
    {
      pthread_cond_signal (&jobqueue->cond);
    }
  }
}
