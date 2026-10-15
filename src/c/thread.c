//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "thread-impl.h"

#ifdef __ZEPHYR__

#define IOT_ZEPHYR_STACK_WAIT_MS 1
#define IOT_ZEPHYR_STACK_RETRIES 100

typedef struct zephyr_thread_wrap
{
  void * arg;
  void *(*func) (void *);
  k_tid_t thread;           /* Set when function has returned, the thread may still be using the stack until terminated */
  bool used;                /* Whether stack in use */
} zephyr_thread_wrap;

static K_THREAD_STACK_ARRAY_DEFINE (zephyr_thread_stacks, IOT_ZEPHYR_MAX_THREADS, IOT_ZEPHYR_STACK_SIZE);
static K_MUTEX_DEFINE (zephyr_stack_mutex);
static zephyr_thread_wrap zephyr_thread_wraps[IOT_ZEPHYR_MAX_THREADS];

static void * zephyr_func_wrapper (void * data)
{
  zephyr_thread_wrap * wrap = (zephyr_thread_wrap*) data;
  void * result = wrap->func (wrap->arg);
  k_mutex_lock (&zephyr_stack_mutex, K_FOREVER);
  wrap->thread = k_current_get (); // Stack released once thread has terminated, as still in use while exiting
  k_mutex_unlock (&zephyr_stack_mutex);
  return result;
}

// Allocate a stack, returning wrapper for the stack or NULL if none available. Called with stack mutex held.

static zephyr_thread_wrap * zephyr_stack_alloc (bool * exiting)
{
  *exiting = false;
  for (uint32_t i = 0; i < IOT_ZEPHYR_MAX_THREADS; i++)
  {
    zephyr_thread_wrap * wrap = &zephyr_thread_wraps[i];
    if (wrap->used && wrap->thread)
    {
      if (k_thread_join (wrap->thread, K_NO_WAIT) == 0) // Thread terminated, so stack free
      {
        wrap->used = false;
      }
      else
      {
        *exiting = true;
      }
    }
    if (! wrap->used)
    {
      wrap->used = true;
      wrap->thread = NULL;
      return wrap;
    }
  }
  return NULL;
}

#endif

static int iot_thread_max_priority;
static int iot_thread_min_priority;
static pthread_once_t iot_thread_priority_once = PTHREAD_ONCE_INIT;

static void iot_thread_priority_init (void)
{
  iot_thread_max_priority = sched_get_priority_max (SCHED_FIFO);
  iot_thread_min_priority = sched_get_priority_min (SCHED_FIFO);
}

bool iot_thread_priority_valid (int priority)
{
  pthread_once (&iot_thread_priority_once, iot_thread_priority_init); // Range set once, safely if called concurrently
  return (priority >= iot_thread_min_priority && priority <= iot_thread_max_priority);
}

static bool iot_thread_create_impl (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity, iot_logger_t * logger, bool joinable)
{
  int ret;
  pthread_attr_t attr;
  pthread_t id;
  const int detach = joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;

  if (tid == NULL) tid = &id;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, detach);
#ifndef __ZEPHYR__
  pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
#endif

#ifdef IOT_HAS_CPU_AFFINITY
  if (affinity > -1 && affinity < sysconf (_SC_NPROCESSORS_ONLN))
  {
    cpu_set_t cpus;
    CPU_ZERO (&cpus);
    CPU_SET (affinity, &cpus);
    ret = pthread_attr_setaffinity_np (&attr, sizeof (cpu_set_t), &cpus);
    if (ret != 0) iot_log_warn (logger, "pthread_attr_setaffinity_np failed ret: %d", ret);
  }
#endif

#ifdef __ZEPHYR__

  zephyr_thread_wrap * wrapper;
  bool exiting;
  for (uint32_t tries = 0; ; tries++)
  {
    k_mutex_lock (&zephyr_stack_mutex, K_FOREVER);
    wrapper = zephyr_stack_alloc (&exiting);
    k_mutex_unlock (&zephyr_stack_mutex);
    if (wrapper || ! exiting || tries == IOT_ZEPHYR_STACK_RETRIES) break;
    k_sleep (K_MSEC (IOT_ZEPHYR_STACK_WAIT_MS)); // Wait for exiting thread to terminate and release its stack
  }
  if (wrapper == NULL)
  {
    iot_log_error (logger, "iot_thread_create: no free thread stack (maximum %d threads)", IOT_ZEPHYR_MAX_THREADS);
    pthread_attr_destroy (&attr);
    return false;
  }
  wrapper->arg = arg;
  wrapper->func = func;
  arg = wrapper;
  func = zephyr_func_wrapper;
  void * stack = zephyr_thread_stacks[wrapper - zephyr_thread_wraps];

#else
  if (iot_thread_priority_valid (priority) && (geteuid () == 0)) // No guarantee that can set RT policies in container
#endif
  {
    struct sched_param param;
#ifdef __ZEPHYR__
    param.sched_priority = (priority != IOT_THREAD_NO_PRIORITY) ? priority : (CONFIG_NUM_COOP_PRIORITIES - 1);
    pthread_attr_setstack (&attr, stack, IOT_ZEPHYR_STACK_SIZE);
#else
    param.sched_priority = priority;
#endif
    /* If priority set, also set FIFO scheduling */

    ret = pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    if (ret != 0) iot_log_warn (logger, "pthread_attr_setschedpolicy failed ret: %d", ret);
    ret = pthread_attr_setschedparam (&attr, &param);
    if (ret != 0) iot_log_warn (logger, "pthread_attr_setschedparam failed ret: %d", ret);
  }
  ret = pthread_create (tid, &attr, func, arg);
#ifdef __ZEPHYR__
  if (ret != 0)
  {
    k_mutex_lock (&zephyr_stack_mutex, K_FOREVER);
    wrapper->used = false; // Release stack for reuse, as no thread created
    k_mutex_unlock (&zephyr_stack_mutex);
  }
#else
  if (ret != 0) // Retry without setting priority (e.g. if not permitted), keeping detach state and affinity
  {
    iot_log_warn (logger, "pthread_create failed ret: %d, retrying without priority", ret);
    pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED);
    ret = pthread_create (tid, &attr, func, arg);
  }
  if (ret != 0) // Retry with default attributes (e.g. affinity not permitted), keeping detach state
  {
    pthread_attr_t dflt;
    iot_log_warn (logger, "pthread_create failed ret: %d, retrying without affinity", ret);
    pthread_attr_init (&dflt);
    pthread_attr_setdetachstate (&dflt, detach);
    ret = pthread_create (tid, &dflt, func, arg);
    pthread_attr_destroy (&dflt);
  }
#endif
  if (ret != 0) iot_log_error (logger, "pthread_create failed ret: %d", ret);
  pthread_attr_destroy (&attr);

#ifdef _ALPINE_
  if ((ret == 0) && (affinity > -1 && affinity < sysconf (_SC_NPROCESSORS_ONLN)))
  {
    cpu_set_t cpus;
    CPU_ZERO (&cpus);
    CPU_SET (affinity, &cpus);
    ret = pthread_setaffinity_np (*tid, sizeof (cpu_set_t), &cpus);
    if (ret != 0)
    {
      iot_log_warn (logger, "pthread_setaffinity_np failed ret: %d", ret);
      ret = 0; // As thread created do not return false, just warn
    }
  }
#endif

  return ret == 0;
}

bool iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity, iot_logger_t * logger)
{
  return iot_thread_create_impl (tid, func, arg, priority, affinity, logger, false);
}

bool iot_thread_create_joinable (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity, iot_logger_t * logger)
{
  return iot_thread_create_impl (tid, func, arg, priority, affinity, logger, true);
}

int iot_thread_get_priority (pthread_t thread)
{
  struct sched_param param = { 0 };
  int policy;
  return (pthread_getschedparam (thread, &policy, &param) == 0) ? param.sched_priority : IOT_THREAD_NO_PRIORITY;
}

int iot_thread_current_get_priority (void)
{
  return iot_thread_get_priority (pthread_self ());
}

bool iot_thread_set_priority (pthread_t thread, int priority)
{
  struct sched_param param = { .sched_priority = priority };
  return (pthread_setschedparam (thread, SCHED_FIFO, &param) == 0);
}

bool iot_thread_current_set_priority (int priority)
{
  return iot_thread_set_priority (pthread_self (), priority);
}

void iot_mutex_init (pthread_mutex_t * mutex)
{
  assert (mutex);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init (&attr);
#ifndef NDEBUG
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK);
#endif
#ifdef IOT_HAS_PTHREAD_MUTEXATTR_SETPROTOCOL
  pthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_INHERIT); // Note: Supported on Alpine but broken
#endif
  pthread_mutex_init (mutex, &attr);
  pthread_mutexattr_destroy (&attr);
}
