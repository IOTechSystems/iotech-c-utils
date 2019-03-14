#include "iot/thread.h"

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

int iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, const int * prio)
{
  int ret;
  pthread_attr_t attr;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

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
  if (prio)
#endif
  {
    struct sched_param param;
#ifdef __ZEPHYR__
    param.sched_priority = prio ? *prio : CONFIG_NUM_COOP_PRIORITIES - 1;
    pthread_attr_setstack (&attr, stack, IOT_ZEPHYR_STACK_SIZE);
#else
    param.sched_priority = *prio;
#endif
    /* If priority set, also set FIFO scheduling */
    pthread_attr_setschedparam (&attr, &param);
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
  }
  ret = pthread_create (tid, &attr, func, arg);
  pthread_attr_destroy (&attr);

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

bool iot_thread_set_priority (pthread_t thread, int prio)
{
  bool result;
  struct sched_param param;
  int policy;

  result = (pthread_getschedparam (thread, &policy, &param) == 0);
  if (result && (prio >= sched_get_priority_min (policy)) && (prio <= sched_get_priority_max (policy)))
  {
    param.sched_priority = prio;
    result = (pthread_setschedparam (thread, policy, &param) == 0);
  }
  return result;
}

bool iot_thread_current_set_priority (int prio)
{
  return iot_thread_set_priority (pthread_self (), prio);
}