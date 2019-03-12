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
#if defined(__linux__)
#include <sys/prctl.h>
#endif

#ifdef THPOOL_DEBUG
#define err(str) fprintf (stderr, str)
#else
#define err(str)
#endif

#ifdef __ZEPHYR__
#define STACK_SIZE 4096
#define MAX_THREADS 5
static K_THREAD_STACK_ARRAY_DEFINE (thread_stacks, MAX_THREADS, STACK_SIZE);
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
  iot_thread_t * threads;                   /* array of threads          */
  atomic_uint_fast32_t num_threads_alive;   /* threads currently alive   */
  atomic_uint_fast32_t num_threads_working; /* threads currently working */
  pthread_mutex_t mutex;                    /* used for thread count etc */
  pthread_cond_t cond;                      /* signal to iot_thpool_wait */
  iot_jobqueue_t jobqueue;                  /* job queue                 */
  atomic_bool running;                      /* state of pool             */
} iot_threadpool_t;


static void * thread_do (iot_thread_t * th);

static void jobqueue_init (iot_jobqueue_t * jobqueue);
static void jobqueue_fini (iot_jobqueue_t * jobqueue);
static void jobqueue_push (iot_jobqueue_t * jobqueue, iot_job_t * job);
static struct iot_job_t * jobqueue_pull_locked (iot_jobqueue_t * jobqueue);


/* ========================== THREADPOOL ============================ */

/* Initialise thread pool */
iot_threadpool_t * iot_thpool_init (uint32_t num_threads)
{
  iot_threadpool_t * pool = (iot_threadpool_t*) calloc (1, sizeof (*pool));
  pool->threads = (iot_thread_t*) calloc (num_threads, sizeof (iot_thread_t));

  pthread_mutex_init (&pool->mutex, NULL);
  pthread_mutex_lock (&pool->mutex);
  atomic_store (&pool->running, true);
  jobqueue_init (&pool->jobqueue);
  pthread_cond_init (&pool->cond, NULL);

  /* Create and start threads */
  for (uint32_t n = 0; n < num_threads; n++)
  {
    pthread_attr_t attr;
    iot_thread_t * th = &pool->threads[n];
    th->pool = pool;
    th->id = n;

    pthread_attr_init (&attr);
#ifdef __ZEPHYR__
    struct sched_param schedparam = { .sched_priority = CONFIG_NUM_COOP_PRIORITIES - 1 };
    pthread_attr_setstack (&attr, thread_stacks[n], STACK_SIZE);
    pthread_attr_setschedparam (&attr, &schedparam);
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
#endif
    pthread_create (&th->pthread, &attr, (void*) thread_do, th);
    pthread_detach (th->pthread);

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
void iot_thpool_add_work (iot_threadpool_t * pool, void (*function) (void*), void* arg, const int * priority)
{
  iot_job_t * job = (iot_job_t*) malloc (sizeof (*job));
  job->function = function;
  job->arg = arg;
  job->priority = priority ? *priority : 0;
  job->prio_set = (priority != NULL);
  jobqueue_push (&pool->jobqueue, job);
}

/* Wait until all jobs have finished */
void iot_thpool_wait (iot_threadpool_t * pool)
{
  pthread_mutex_lock (&pool->mutex);
  while (atomic_load (&pool->jobqueue.jobs) || atomic_load (&pool->num_threads_working))
  {
    pthread_cond_wait (&pool->cond, &pool->mutex);
  }
  pthread_mutex_unlock (&pool->mutex);
}

#ifdef __ZEPHYR__
static void time (time_t *t)
{
  *t = k_uptime_get ();
}

static double difftime (time_t end, time_t start)
{
  return (end - start) / 1000.0;
}
#endif

/* Destroy the threadpool */
void iot_thpool_destroy (iot_threadpool_t * pool)
{
  if (pool)
  {
    pthread_mutex_lock (&pool->mutex);

    /* End each thread 's infinite loop */
    atomic_store (&pool->running, false);

    /* Poll remaining threads */
    while (atomic_load (&pool->num_threads_alive))
    {
      pthread_cond_broadcast (&pool->jobqueue.cond);
      sleep (1);
    }

    /* Cleanup */
    jobqueue_fini (&pool->jobqueue);
    pthread_cond_destroy (&pool->cond);
    pthread_mutex_unlock (&pool->mutex);
    pthread_mutex_destroy (&pool->mutex);
    free (pool->threads);
    free (pool);
  }
}

uint32_t iot_thpool_num_threads_working (iot_threadpool_t * pool)
{
  return (uint32_t) atomic_load (&pool->num_threads_working);
}

/* ============================ THREAD ============================== */

/* What each thread is doing
*
* In principle this is an endless loop. The only time this loop gets interuppted is once
* iot_thpool_destroy() is invoked or the program exits.
*
* @param  thread        thread that will run this function
* @return nothing
*/
static void * thread_do (iot_thread_t * th)
{
  iot_threadpool_t * pool = th->pool;

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
    /* Read job from queue and execute it */
    pthread_mutex_lock (&pool->jobqueue.mutex);
    iot_job_t * job = jobqueue_pull_locked (&pool->jobqueue);
    if (job)
    {
      pthread_mutex_unlock (&pool->jobqueue.mutex);
      atomic_fetch_add (&pool->num_threads_working, 1);
      (job->function) (job->arg);
      free (job);
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

static void jobqueue_init (iot_jobqueue_t * jobqueue)
{
  pthread_mutex_init (&jobqueue->mutex, NULL);
  pthread_cond_init (&jobqueue->cond, NULL);
}

static void jobqueue_fini (iot_jobqueue_t * jobqueue)
{
  pthread_mutex_lock (&jobqueue->mutex);
  while (atomic_load (&jobqueue->jobs))
  {
    free (jobqueue_pull_locked (jobqueue));
  }
  pthread_mutex_unlock (&jobqueue->mutex);
  pthread_cond_destroy (&jobqueue->cond);
  pthread_mutex_destroy (&jobqueue->mutex);
}

/* Add new job to queue, ordered by priority */
static void jobqueue_push (iot_jobqueue_t * jobqueue, iot_job_t * job)
{
  pthread_mutex_lock (&jobqueue->mutex);
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

static struct iot_job_t * jobqueue_pull_locked (iot_jobqueue_t * jobqueue)
{
  iot_job_t* job = jobqueue->front;

  switch (atomic_load (&jobqueue->jobs))
  {
    case 0:  /* if no jobs in queue */
      break;
    case 1:  /* if one job in queue */
      jobqueue->front = NULL;
      jobqueue->rear  = NULL;
      atomic_store (&jobqueue->jobs, 0);
      break;
    default: /* if >1 jobs in queue */
      jobqueue->front = job->prev;
      atomic_fetch_add (&jobqueue->jobs, -1);
      /* more than one job in queue -> post it */
      pthread_cond_signal (&jobqueue->cond);
  }
  return job;
}
