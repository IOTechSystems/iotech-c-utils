//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/thread.h"

static int iot_thread_fifo_max_priority = -2;
static int iot_thread_fifo_min_priority = -2;

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

int iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity)
{
  int ret;
  pthread_attr_t attr;

  if (iot_thread_fifo_max_priority == -2)
  {
    iot_thread_fifo_max_priority = sched_get_priority_max (SCHED_FIFO);
    iot_thread_fifo_min_priority = sched_get_priority_min (SCHED_FIFO);
  }

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

#if defined (_GNU_SOURCE) && ! defined (__LIBMUSL__)
  if (affinity > -1 && affinity < sysconf (_SC_NPROCESSORS_ONLN))
  {
    cpu_set_t cpus;
    CPU_ZERO (&cpus);
    CPU_SET (affinity, &cpus);
    pthread_attr_setaffinity_np (&attr, sizeof (cpu_set_t), &cpus);
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
  if (priority >= iot_thread_fifo_min_priority && priority <= iot_thread_fifo_max_priority)
#endif
  {
    struct sched_param param;
#ifdef __ZEPHYR__
    param.sched_priority = priority ? *priority : CONFIG_NUM_COOP_PRIORITIES - 1;
    pthread_attr_setstack (&attr, stack, IOT_ZEPHYR_STACK_SIZE);
#else
    param.sched_priority = priority;
#endif
    /* If priority set, also set FIFO scheduling */

    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    pthread_attr_setschedparam (&attr, &param);
  }
  ret = pthread_create (tid, &attr, func, arg);
  pthread_attr_destroy (&attr);

#if defined (_GNU_SOURCE) && defined (__LIBMUSL__)
  if ((ret == 0) && (affinity > -1 && affinity < sysconf (_SC_NPROCESSORS_ONLN)))
  {
    cpu_set_t cpus;
    CPU_ZERO (&cpus);
    CPU_SET (affinity, &cpus);
    pthread_setaffinity_np (*tid, sizeof (cpu_set_t), &cpus);
  }
#endif

  return ret;
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
#ifndef __ZEPHYR__
  pthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_INHERIT);
#endif
  pthread_mutex_init (mutex, &attr);
  pthread_mutexattr_destroy (&attr);
}
