//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/scheduler.h"
#include "iot/bus.h"

#define IOT_BUS_DEFAULT_INTERVAL 500000000
#define IOT_BUS_DEFAULT_THREADS 2

typedef struct iot_bus_topic_t
{
  struct iot_bus_topic_t * next;
  char * name;
  int priority;
  bool prio_set;
} iot_bus_topic_t;

typedef struct iot_bus_match_t
{
  iot_bus_sub_t * sub;
  struct iot_bus_match_t * next;
} iot_bus_match_t;

struct iot_bus_pub_t
{
  iot_bus_t * bus;
  struct iot_bus_pub_t * next;
  const iot_bus_topic_t * topic;
  void * self;
  iot_bus_match_t * matches;
  iot_data_pub_cb_fn_t callback;
  iot_schedule_t * sc;
  atomic_uint_fast32_t refs;
};

struct iot_bus_sub_t
{
  iot_bus_t * bus;
  struct iot_bus_sub_t * next;
  void * self;
  char * pattern;
  iot_data_sub_fn_t callback;
  atomic_uint_fast32_t refs;
};

struct iot_bus_t
{
  iot_bus_sub_t * subscribers;
  iot_bus_pub_t * publishers;
  iot_bus_topic_t * topics;
  iot_threadpool_t * threadpool;
  iot_scheduler_t * scheduler;
  uint64_t interval;
  pthread_rwlock_t lock;
};

typedef struct iot_bus_job_t
{
  iot_bus_sub_t * sub;
  iot_bus_pub_t * pub;
  iot_data_t * data;
} iot_bus_job_t;

static iot_bus_topic_t * iot_bus_topic_find_locked (iot_bus_t * cd, const char * name)
{
  iot_bus_topic_t * topic = cd->topics;
  while (topic)
  {
    if (strcmp (topic->name, name) == 0) break;
    topic = topic->next;
  }
  return topic;
}

static iot_bus_topic_t * iot_bus_topic_create_locked (iot_bus_t * cd, const char * name, const int * prio)
{
  iot_bus_topic_t * topic = iot_bus_topic_find_locked (cd, name);
  if (topic == NULL)
  {
    topic = malloc (sizeof (*topic));
    topic->name = iot_strdup (name);
    topic->next = cd->topics;
    cd->topics = topic;
  }
  topic->prio_set = (prio != NULL);
  topic->priority = prio ? *prio : 0;
  return topic;
}

static void iot_bus_sub_free_locked (iot_bus_sub_t * sub)
{
  iot_bus_sub_t * prev = NULL;
  iot_bus_sub_t * iter = sub->bus->subscribers;
  iot_bus_pub_t * pub = sub->bus->publishers;

  while (iter)
  {
    if (iter == sub)
    {
      if (prev)
      {
        prev->next = iter->next;
      }
      else
      {
        sub->bus->subscribers = iter->next;
      }
      break;
    }
    prev = iter;
    iter = iter->next;
  }

  while (pub)
  {
    iot_bus_match_t * mprev = NULL;
    iot_bus_match_t * match = pub->matches;
    while (match)
    {
      if (match->sub == sub)
      {
        if (mprev)
        {
          mprev->next = match->next;
        }
        else
        {
          pub->matches = match->next;
        }
        free (match);
      }
      mprev = match;
      match = match->next;
    }
    pub = pub->next;
  }
  free (sub->pattern);
  free (sub);
}

static void iot_bus_pub_free_locked (iot_bus_pub_t * pub)
{
  iot_bus_match_t * match;
  iot_bus_pub_t * prev = NULL;
  iot_bus_pub_t * iter = pub->bus->publishers;

  while (iter)
  {
    if (iter == pub)
    {
      if (prev)
      {
        prev->next = iter->next;
      }
      else
      {
        pub->bus->publishers = iter->next;
      }
      break;
    }
    prev = iter;
    iter = iter->next;
  }

  while ((match = pub->matches))
  {
    pub->matches = match->next;
    free (match);
  }
  free (pub);
}

static iot_bus_sub_t * iot_bus_find_sub_locked (iot_bus_t * cd, void * self, const char * pattern, iot_bus_sub_t ** prev)
{
  iot_bus_sub_t * sub = cd->subscribers;
  while (sub)
  {
    if (sub->self == self && strcmp (sub->pattern, pattern) == 0)
    {
      break;
    }
    if (prev) *prev = sub;
    sub = sub->next;
  }
  return sub;
}

static iot_bus_pub_t * iot_bus_find_pub_locked (iot_bus_t * cd, void * self, const char * topic, iot_bus_pub_t ** prev)
{
  iot_bus_pub_t * pub = cd->publishers;
  while (pub)
  {
    if (pub->self == self && strcmp (pub->topic->name, topic) == 0)
    {
      break;
    }
    if (prev) *prev = pub;
    pub = pub->next;
  }
  return pub;
}

static bool iot_bus_topic_match (const char * topic, const char * pattern)
{
  bool match = false;
  char *psave, *tsave, *ttok, *ptok;
  char * top = iot_strdup (topic);
  char * pat = iot_strdup (pattern);
  ptok = pat;
  ttok = top;

  while (! match)
  {
#ifdef __ZEPHYR__
    ptok = iot_ctok_r (ptok, '/', &tsave);
    ttok = iot_ctok_r (ttok, '/', &psave);
#else
    ptok = strtok_r (ptok, "/", &tsave);
    ttok = strtok_r (ttok, "/", &psave);
#endif
    if (ptok == NULL || ttok == NULL)
    {
      match = (ptok == ttok);
      break;
    }
    if (*ptok == '#') // Match all wildcard
    {
      match = true;
      break;
    }
    if (*ptok != '+') // Match local wildcard
    {
      if (strcmp (ttok, ptok) != 0)
      {
        break;
      }
    }
    ptok = NULL;
    ttok = NULL;
  }
  free (top);
  free (pat);
  return match;
}

static void iot_bus_match_locked (iot_bus_pub_t * pub, iot_bus_sub_t * sub)
{
  if (iot_bus_topic_match (pub->topic->name, sub->pattern))
  {
    iot_bus_match_t * match = malloc (sizeof (*match));
    match->sub = sub;
    match->next = pub->matches;
    pub->matches = match;
  }
}

static void iot_bus_sched_fn (void * arg)
{
  iot_bus_pub_t * pub = (iot_bus_pub_t*) arg;
  iot_bus_publish (pub, (pub->callback) (pub->self), true);
}

iot_bus_t * iot_bus_alloc (void)
{
  iot_bus_t * cd = calloc (1, sizeof (*cd));
  pthread_rwlock_init (&cd->lock, NULL);
  return cd;
}

void iot_bus_topic_create (iot_bus_t * cd, const char * name, const int * prio)
{
  assert (cd && name);
  pthread_rwlock_wrlock (&cd->lock);
  iot_bus_topic_create_locked (cd, name, prio);
  pthread_rwlock_unlock (&cd->lock);
}

void iot_bus_init (iot_bus_t * cd, const char * json_config)
{
  assert (cd);
  uint32_t threads = IOT_BUS_DEFAULT_THREADS;

  pthread_rwlock_wrlock (&cd->lock);
  cd->interval = IOT_BUS_DEFAULT_INTERVAL;
  if (json_config)
  {
    iot_data_t * config = iot_data_from_json (json_config);
    const iot_data_t * value = iot_data_string_map_get (config, "Interval");
    if (value) cd->interval = (uint64_t) iot_data_i64 (value) * 1000;
    value = iot_data_string_map_get (config, "Threads");
    if (value) threads = (uint32_t) iot_data_i64 (value);
    value = iot_data_string_map_get (config, "Topics");
    if (value)
    {
      iot_data_array_iter_t iter;
      iot_data_array_iter (value, &iter);
      while (iot_data_array_iter_next (&iter))
      {
        const iot_data_t * map = iot_data_array_iter_value (&iter);
        value = iot_data_string_map_get (map, "Topic");
        const char * name = iot_data_string (value);
        value = iot_data_string_map_get (map, "Priority");
        int prio = (int) iot_data_i64 (value);
        iot_bus_topic_create_locked (cd, name, &prio);
      }
    }
    iot_data_free (config);
  }
  cd->threadpool = iot_threadpool_init (threads, NULL);
  cd->scheduler = iot_scheduler_init (cd->threadpool);
  pthread_rwlock_unlock (&cd->lock);
}

void iot_bus_start (iot_bus_t * cd)
{
  assert (cd);
  iot_scheduler_start (cd->scheduler);
}

void iot_bus_stop (iot_bus_t * cd)
{
  assert (cd);
  iot_scheduler_stop (cd->scheduler);
}

void iot_bus_free (iot_bus_t * cd)
{
  if (cd)
  {
    iot_bus_topic_t * topic;
    while (cd->publishers)
    {
      pthread_rwlock_wrlock (&cd->lock);
      iot_bus_pub_free_locked (cd->publishers);
      pthread_rwlock_unlock (&cd->lock);
    }
    while (cd->subscribers)
    {
      pthread_rwlock_wrlock (&cd->lock);
      iot_bus_sub_free_locked (cd->subscribers);
      pthread_rwlock_unlock (&cd->lock);
    }
    while ((topic = cd->topics))
    {
      cd->topics = topic->next;
      free (topic->name);
      free (topic);
    }
    iot_scheduler_fini (cd->scheduler);
    sleep (1);
    iot_threadpool_destroy (cd->threadpool);
    pthread_rwlock_destroy (&cd->lock);
    free (cd);
  }
}

static void iot_bus_match_sub_locked (iot_bus_t * cd, iot_bus_sub_t * sub)
{
  iot_bus_pub_t * pub = cd->publishers;
  while (pub)
  {
    iot_bus_match_locked (pub, sub);
    pub = pub->next;
  }
}

static void iot_bus_match_pub_locked (iot_bus_t * cd, iot_bus_pub_t * pub)
{
  iot_bus_sub_t * sub = cd->subscribers;
  while (sub)
  {
    iot_bus_match_locked (pub, sub);
    sub = sub->next;
  }
}

iot_bus_sub_t * iot_bus_sub_alloc (iot_bus_t * cd, void * self, iot_data_sub_fn_t callback, const char * pattern)
{
  assert (cd && callback && pattern);

  pthread_rwlock_wrlock (&cd->lock);
  iot_bus_sub_t * sub = iot_bus_find_sub_locked (cd, self, pattern, NULL);
  if (sub == NULL)
  {
    sub = calloc (1, sizeof (*sub));
    sub->self = self;
    sub->bus = cd;
    sub->pattern = iot_strdup (pattern);
    sub->callback = callback;
    if (cd->subscribers)
    {
      sub->next = cd->subscribers;
    }
    atomic_store (&sub->refs, 1);
    cd->subscribers = sub;
    iot_bus_match_sub_locked (cd, sub);
  }
  pthread_rwlock_unlock (&cd->lock);
  return sub;
}

void iot_bus_sub_free (iot_bus_sub_t * sub)
{
  if (sub && atomic_fetch_add (&sub->refs, -1) <= 1)
  {
    pthread_rwlock_t * lock = &sub->bus->lock;
    pthread_rwlock_wrlock (lock);
    iot_bus_sub_free_locked (sub);
    pthread_rwlock_unlock (lock);
  }
}

iot_bus_pub_t * iot_bus_pub_alloc (iot_bus_t * cd, void * self, iot_data_pub_cb_fn_t callback, const char * topic)
{
  assert (cd && topic);
  pthread_rwlock_wrlock (&cd->lock);
  iot_bus_pub_t * pub = iot_bus_find_pub_locked (cd, self, topic, NULL);
  if (pub == NULL)
  {
    pub = calloc (1, sizeof (*pub));
    pub->self = self;
    pub->bus = cd;
    pub->topic = iot_bus_topic_create_locked (cd, topic, NULL);
    pub->next = cd->publishers;
    atomic_store (&pub->refs, 1);
    cd->publishers = pub;
    if (callback)
    {
      pub->callback = callback;
      pub->sc = iot_schedule_create (cd->scheduler, iot_bus_sched_fn, pub, cd->interval, 0, 0, pub->topic->prio_set ? &pub->topic->priority : NULL);
      iot_schedule_add (cd->scheduler, pub->sc);
    }
    iot_bus_match_pub_locked (cd, pub);
  }
  pthread_rwlock_unlock (&cd->lock);
  return pub;
}

void iot_bus_pub_free (iot_bus_pub_t * pub)
{
  if (pub && atomic_fetch_add (&pub->refs, -1) <= 1)
  {
    pthread_rwlock_t *lock = &pub->bus->lock;
    if (pub->sc)
    {
      iot_schedule_remove (pub->bus->scheduler, pub->sc);
    }
    pthread_rwlock_wrlock (lock);
    iot_bus_pub_free_locked (pub);
    pthread_rwlock_unlock (lock);
  }
}

static void iot_bus_publish_job (void * arg)
{
  iot_bus_job_t * job = (iot_bus_job_t*) arg;
  (job->sub->callback) (job->data, job->sub->self, job->pub->topic->name);
  iot_bus_pub_free (job->pub);
  iot_bus_sub_free (job->sub);
  iot_data_free (job->data);
  free (job);
}

void iot_bus_publish (iot_bus_pub_t * pub, iot_data_t * data, bool sync)
{
  assert (pub && data);

  pthread_rwlock_rdlock (&pub->bus->lock);
  iot_bus_match_t * match = pub->matches;
  while (match)
  {
    if (sync)
    {
      (match->sub->callback) (data, match->sub->self, pub->topic->name);
    }
    else
    {
      iot_bus_job_t * job = malloc (sizeof (*job));
      job->pub = pub;
      job->sub = match->sub;
      job->data = data;
      iot_data_addref (data);
      atomic_fetch_add (&pub->refs, 1);
      atomic_fetch_add (&job->sub->refs, 1);
      iot_threadpool_add_work (pub->bus->threadpool, iot_bus_publish_job, job, job->pub->topic->prio_set ? &job->pub->topic->priority : NULL);
    }
    match = match->next;
  }
  pthread_rwlock_unlock (&pub->bus->lock);
  iot_data_free (data);
}
