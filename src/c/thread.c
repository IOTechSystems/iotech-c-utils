//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/thread.h"

#ifdef _AZURESPHERE_
#define geteuid() (0)
#endif

#ifdef __ZEPHYR__

typedef struct zephyr_thread_wrap
{
  void * arg;
  void *(*func) (void *);
} zephyr_thread_wrap;

static K_THREAD_STACK_ARRAY_DEFINE (zephyr_thread_stacks, IOT_ZEPHYR_MAX_THREADS, IOT_ZEPHYR_STACK_SIZE);
static K_MUTEX_DEFINE (zephyr_stack_mutex);
static zephyr_thread_wrap zephyr_thread_wraps[IOT_ZEPHYR_MAX_THREADS];

static void * zephyr_func_wrapper (void * data)
{
  zephyr_thread_wrap * wrap = (zephyr_thread_wrap*) data;
  void * result = wrap->func (wrap->arg);
  k_mutex_lock (&zephyr_stack_mutex, K_FOREVER);
  wrap->func = NULL;
  k_mutex_unlock (&zephyr_stack_mutex);
  return result;
}

#endif

bool iot_thread_priority_valid (int priority)
{
  static int max_priority = -2;
  static int min_priority = -2;
  if (max_priority == -2)
  {
    max_priority = sched_get_priority_max (SCHED_FIFO);
    min_priority = sched_get_priority_min (SCHED_FIFO);
  }
  return (priority >= min_priority && priority <= max_priority);
}

bool iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity, iot_logger_t * logger)
{
  int ret;
  pthread_attr_t attr;
  pthread_t id;

  if (tid == NULL) tid = &id;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
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

  zephyr_thread_wrap * wrapper = NULL;
  void * stack = NULL;
  k_mutex_lock (&zephyr_stack_mutex, K_FOREVER);
  for (uint32_t i = 0; i < IOT_ZEPHYR_MAX_THREADS; i++)
  {
    if (zephyr_thread_wraps[i].func == NULL)
    {
      wrapper = &zephyr_thread_wraps[i];
      wrapper->arg = arg;
      wrapper->func = func;
      arg = wrapper;
      func = zephyr_func_wrapper;
      stack = zephyr_thread_stacks[i];
      break;
    }
  }
  k_mutex_unlock (&zephyr_stack_mutex);
  assert (wrapper);

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
  if (ret != 0)
  {
    ret = pthread_create (tid, NULL, func, arg);
    if (ret != 0) iot_log_error (logger, "pthread_create failed ret: %d", ret);
  }
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

int iot_thread_get_priority (pthread_t thread)
{
  struct sched_param param;
  int policy;
  pthread_getschedparam (thread, &policy, &param);
  return param.sched_priority;
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
