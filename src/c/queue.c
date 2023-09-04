//
// Copyright (c) 2023 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/queue.h"

struct iot_queue_t
{
  iot_data_t *queue;
  pthread_mutex_t mtx;
  pthread_cond_t added;
  pthread_cond_t removed;
  uint32_t maxsize;
  bool running;
};

iot_queue_t *iot_queue_alloc (uint32_t maxsize)
{
  iot_queue_t *result = calloc (1, sizeof (iot_queue_t));
  result->queue = iot_data_alloc_list ();
  result->maxsize = maxsize;
  result->running = true;
  pthread_mutex_init (&result->mtx, NULL);
  pthread_cond_init (&result->added, NULL);
  pthread_cond_init (&result->removed, NULL);
  return result;
}

void iot_queue_stop (iot_queue_t *q)
{
  assert (q);
  pthread_mutex_lock (&q->mtx);
  q->running = false;
  pthread_cond_broadcast (&q->added);
  pthread_cond_broadcast (&q->removed);
  pthread_mutex_unlock (&q->mtx);
}

void iot_queue_free (iot_queue_t *q)
{
  if (q)
  {
    iot_data_free (q->queue);
    pthread_cond_destroy (&q->added);
    pthread_cond_destroy (&q->removed);
    pthread_mutex_destroy (&q->mtx);
    free (q);
  }
}

iot_data_t *iot_queue_try_dequeue (iot_queue_t *q)
{
  iot_data_t *result = NULL;
  assert (q);
  pthread_mutex_lock (&q->mtx);
  result = iot_data_list_tail_pop (q->queue);
  if (result)
  {
    pthread_cond_signal (&q->removed);
  }
  pthread_mutex_unlock (&q->mtx);
  return result;
}

iot_data_t *iot_queue_dequeue (iot_queue_t *q)
{
  iot_data_t *result;
  assert (q);
  pthread_mutex_lock (&q->mtx);
  result = iot_data_list_tail_pop (q->queue);
  while (result == NULL)
  {
    if (!q->running)
    {
      pthread_mutex_unlock (&q->mtx);
      return NULL;
    }
    pthread_cond_wait (&q->added, &q->mtx);
    result = iot_data_list_tail_pop (q->queue);
  }
  pthread_cond_signal (&q->removed);
  pthread_mutex_unlock (&q->mtx);
  return result;
}

bool iot_queue_try_enqueue (iot_queue_t *q, iot_data_t *element)
{
  bool result = false;
  assert (q);
  pthread_mutex_lock (&q->mtx);
  if (q->maxsize == 0 || iot_data_list_length (q->queue) < q->maxsize)
  {
    result = true;
    iot_data_list_head_push (q->queue, element);
    pthread_cond_signal (&q->added);
  }
  pthread_mutex_unlock (&q->mtx);
  return result;
}

void iot_queue_enqueue (iot_queue_t *q, iot_data_t *element)
{
  assert (q);
  pthread_mutex_lock (&q->mtx);
  while (q->maxsize && iot_data_list_length (q->queue) >= q->maxsize)
  {
    if (!q->running)
    {
      pthread_mutex_unlock (&q->mtx);
      return;
    }
    pthread_cond_wait (&q->removed, &q->mtx);
  }
  iot_data_list_head_push (q->queue, element);
  pthread_cond_signal (&q->added);
  pthread_mutex_unlock (&q->mtx);
}

uint32_t iot_queue_size (const iot_queue_t *q)
{
  uint32_t result;
  pthread_mutex_t *mtx = (pthread_mutex_t *)&q->mtx;
  assert (q);
  pthread_mutex_lock (mtx);
  result = iot_data_list_length (q->queue);
  pthread_mutex_unlock (mtx);
  return result;
}

uint32_t iot_queue_maxsize (const iot_queue_t *q)
{
  uint32_t result;
  pthread_mutex_t *mtx = (pthread_mutex_t *)&q->mtx;
  assert (q);
  pthread_mutex_lock (mtx);
  result = q->maxsize;
  pthread_mutex_unlock (mtx);
  return result;
}

void iot_queue_setmaxsize (iot_queue_t *q, uint32_t maxsize)
{
  assert (q);
  pthread_mutex_lock (&q->mtx);
  if (maxsize > q->maxsize || maxsize == 0)
  {
    pthread_cond_broadcast (&q->removed);
  }
  q->maxsize = maxsize;
  pthread_mutex_unlock (&q->mtx);
}
