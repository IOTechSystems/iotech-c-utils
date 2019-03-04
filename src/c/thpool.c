/* ********************************
 * Author:       Johan Hanssen Seferidis
 * License:       MIT
 * Description:  Library providing a threading pool where you can add
 *               work. For usage, check the thpool.h file or README.md
 *
 *//** @file thpool.h *//*
 *
 ********************************/

#include "iot/thpool.h"
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
static K_THREAD_STACK_ARRAY_DEFINE(thread_stacks, MAX_THREADS, STACK_SIZE);
#endif

/* ========================== STRUCTURES ============================ */

/* Binary semaphore */
typedef struct iot_bsem
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool val;
} iot_bsem;

/* Job */
typedef struct iot_job
{
  struct iot_job * prev;               /* pointer to previous job   */
  void (*function) (void* arg);        /* function pointer          */
  void * arg;                          /* function's argument       */
} iot_job;

/* Job queue */
typedef struct iot_jobqueue
{
  pthread_mutex_t rwmutex;             /* used for queue r/w access */
  iot_job * front;                     /* pointer to front of queue */
  iot_job * rear;                      /* pointer to rear  of queue */
  iot_bsem * has_jobs;                 /* flag as binary semaphore  */
  volatile unsigned len;               /* number of jobs in queue   */
} iot_jobqueue;

/* Thread */
typedef struct iot_thread
{
  unsigned id;                         /* friendly id               */
  pthread_t pthread;                   /* pointer to actual thread  */
  struct iot_threadpool * thpool;      /* thread pool               */
} iot_thread;

/* Threadpool */
typedef struct iot_threadpool
{
  iot_thread **   threads;               /* pointer to threads        */
  volatile unsigned num_threads_alive;   /* threads currently alive   */
  volatile unsigned num_threads_working; /* threads currently working */
  pthread_mutex_t  thcount_lock;         /* used for thread count etc */
  pthread_cond_t  threads_all_idle;      /* signal to iot_thpool_wait */
  iot_jobqueue  jobqueue;                /* job queue                 */
  volatile bool running;                 /* state of pool             */
} iot_threadpool;


/* ========================== PROTOTYPES ============================ */

static void * thread_do (iot_thread * th);

static void jobqueue_init (iot_jobqueue * jobqueue);
static void jobqueue_clear (iot_jobqueue * jobqueue);
static void jobqueue_push (iot_jobqueue * jobqueue, iot_job * newjob);
static struct iot_job * jobqueue_pull (iot_jobqueue * jobqueue);
static void jobqueue_destroy (iot_jobqueue * jobqueue);

static void bsem_init (iot_bsem * bsem, bool value);
static void bsem_reset (iot_bsem * bsem);
static void bsem_post (iot_bsem * bsem);
static void bsem_post_all (iot_bsem * bsem);
static void bsem_wait (iot_bsem * bsem);


/* ========================== THREADPOOL ============================ */

/* Initialise thread pool */
iot_threadpool * iot_thpool_init (unsigned num_threads)
{
  iot_threadpool * thpool = (iot_threadpool*) calloc (1, sizeof (*thpool));
  thpool->running = true;
  jobqueue_init (&thpool->jobqueue);
  pthread_mutex_init (&(thpool->thcount_lock), NULL);
  pthread_cond_init (&thpool->threads_all_idle, NULL);

  /* Create and start threads */
  thpool->threads = (iot_thread**) malloc (num_threads * sizeof (iot_thread*));
  for (unsigned n = 0; n < num_threads; n++)
  {
    pthread_attr_t attr;
    iot_thread * th = (iot_thread*) calloc (1, sizeof (*th));
    th->thpool = thpool;
    th->id = n;
    thpool->threads[n] = th;

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

  /* Wait for threads to initialize */
  while (thpool->num_threads_alive != num_threads)
  {
    usleep (100000);
  }

  return thpool;
}

/* Add work to the thread pool */
void iot_thpool_add_work (iot_threadpool * thpool_p, void (*function_p)(void*), void* arg_p)
{
  iot_job * newjob = (iot_job*) malloc (sizeof (*newjob));
  newjob->function = function_p;
  newjob->arg = arg_p;
  jobqueue_push (&thpool_p->jobqueue, newjob);
}

/* Wait until all jobs have finished */
void iot_thpool_wait (iot_threadpool * thpool_p)
{
  pthread_mutex_lock (&thpool_p->thcount_lock);
  while (thpool_p->jobqueue.len || thpool_p->num_threads_working)
  {
    pthread_cond_wait (&thpool_p->threads_all_idle, &thpool_p->thcount_lock);
  }
  pthread_mutex_unlock (&thpool_p->thcount_lock);
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
void iot_thpool_destroy (iot_threadpool * thpool_p)
{
  /* No need to destory if it's NULL */
  if (thpool_p == NULL) return ;

  volatile unsigned threads_total = thpool_p->num_threads_alive;

  /* End each thread 's infinite loop */
  thpool_p->running = false;

  /* Give one second to kill idle threads */
  double TIMEOUT = 1.0;
  time_t start, end;
  double tpassed = 0.0;
  time (&start);
  while (tpassed < TIMEOUT && thpool_p->num_threads_alive)
  {
    bsem_post_all (thpool_p->jobqueue.has_jobs);
    time (&end);
    tpassed = difftime (end, start);
  }

  /* Poll remaining threads */
  while (thpool_p->num_threads_alive)
  {
    bsem_post_all (thpool_p->jobqueue.has_jobs);
    sleep (1);
  }

  /* Job queue cleanup */
  jobqueue_destroy (&thpool_p->jobqueue);
  /* Deallocs */
  for (unsigned n = 0; n < threads_total; n++)
  {
    free (thpool_p->threads[n]);
  }
  free (thpool_p->threads);
  free (thpool_p);
}

unsigned iot_thpool_num_threads_working (iot_threadpool * thpool)
{
  return thpool->num_threads_working;
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
static void* thread_do (iot_thread * th)
{
  iot_threadpool * thpool = th->thpool;

#if defined (__linux__)
  char thread_name[128] = {0};
  sprintf (thread_name, "thread-pool-%u", th->id);
  /* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
  prctl (PR_SET_NAME, thread_name);
#endif

  /* Mark thread as alive (initialized) */
  pthread_mutex_lock (&thpool->thcount_lock);
  thpool->num_threads_alive += 1;
  pthread_mutex_unlock (&thpool->thcount_lock);

  while (thpool->running)
  {
    bsem_wait (thpool->jobqueue.has_jobs);
    if (thpool->running)
    {
      pthread_mutex_lock (&thpool->thcount_lock);
      thpool->num_threads_working++;
      pthread_mutex_unlock (&thpool->thcount_lock);

      /* Read job from queue and execute it */
      iot_job * job = jobqueue_pull (&thpool->jobqueue);
      if (job)
      {
        (job->function) (job->arg);
        free (job);
      }

      pthread_mutex_lock (&thpool->thcount_lock);
      if (--thpool->num_threads_working == 0)
      {
        pthread_cond_signal (&thpool->threads_all_idle);
      }
      pthread_mutex_unlock (&thpool->thcount_lock);
    }
  }
  pthread_mutex_lock (&thpool->thcount_lock);
  thpool->num_threads_alive--;
  pthread_mutex_unlock (&thpool->thcount_lock);

  return NULL;
}


/* ============================ JOB QUEUE =========================== */


/* Initialize queue */
static void jobqueue_init (iot_jobqueue * jobqueue)
{
  jobqueue->len = 0;
  jobqueue->front = NULL;
  jobqueue->rear  = NULL;
  jobqueue->has_jobs = (iot_bsem*) malloc (sizeof (iot_bsem));
  pthread_mutex_init (&(jobqueue->rwmutex), NULL);
  bsem_init (jobqueue->has_jobs, false);
}

/* Clear the queue */
static void jobqueue_clear (iot_jobqueue * jobqueue)
{
  while (jobqueue->len)
  {
    free (jobqueue_pull (jobqueue));
  }
  jobqueue->front = NULL;
  jobqueue->rear  = NULL;
  bsem_reset (jobqueue->has_jobs);
}


/* Add (allocated) job to queue
 */
static void jobqueue_push (iot_jobqueue * jobqueue, iot_job * newjob)
{
  pthread_mutex_lock (&jobqueue->rwmutex);
  newjob->prev = NULL;

  if (jobqueue->len == 0)
  {
    jobqueue->front = newjob;
  }
  else
  {
    jobqueue->rear->prev = newjob;
  }
  jobqueue->rear = newjob;
  jobqueue->len++;

  bsem_post (jobqueue->has_jobs);
  pthread_mutex_unlock (&jobqueue->rwmutex);
}

static struct iot_job * jobqueue_pull (iot_jobqueue * jobqueue)
{
  pthread_mutex_lock (&jobqueue->rwmutex);
  iot_job* job = jobqueue->front;

  switch (jobqueue->len)
  {
    case 0:  /* if no jobs in queue */
      break;
    case 1:  /* if one job in queue */
      jobqueue->front = NULL;
      jobqueue->rear  = NULL;
      jobqueue->len = 0;
      break;
    default: /* if >1 jobs in queue */
      jobqueue->front = job->prev;
      jobqueue->len--;
      /* more than one job in queue -> post it */
      bsem_post (jobqueue->has_jobs);
  }

  pthread_mutex_unlock (&jobqueue->rwmutex);
  return job;
}

/* Free all queue resources back to the system */
static void jobqueue_destroy (iot_jobqueue * jobqueue)
{
  jobqueue_clear (jobqueue);
  free (jobqueue->has_jobs);
}


/* ======================== SYNCHRONISATION ========================= */

/* Init semaphore to 1 or 0 */
static void bsem_init (iot_bsem * bsem, bool value)
{
  pthread_mutex_init (&(bsem->mutex), NULL);
  pthread_cond_init (&(bsem->cond), NULL);
  bsem->val = value;
}

/* Reset semaphore */
static void bsem_reset (iot_bsem * bsem)
{
  bsem_init (bsem, false);
}

/* Post to at least one thread */
static void bsem_post (iot_bsem * bsem)
{
  pthread_mutex_lock (&bsem->mutex);
  bsem->val = true;
  pthread_cond_signal (&bsem->cond);
  pthread_mutex_unlock (&bsem->mutex);
}

/* Post to all threads */
static void bsem_post_all (iot_bsem * bsem)
{
  pthread_mutex_lock (&bsem->mutex);
  bsem->val = true;
  pthread_cond_broadcast (&bsem->cond);
  pthread_mutex_unlock (&bsem->mutex);
}

/* Wait on semaphore until semaphore true */
static void bsem_wait (iot_bsem * bsem)
{
  pthread_mutex_lock (&bsem->mutex);
  while (! bsem->val)
  {
    pthread_cond_wait (&bsem->cond, &bsem->mutex);
  }
  bsem->val = false;
  pthread_mutex_unlock (&bsem->mutex);
}
