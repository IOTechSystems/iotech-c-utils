//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/scheduler.h"
#include "iot/bus.h"
#include "iot/container.h"

#define IOT_BUS_DEFAULT_INTERVAL 500000000

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
  iot_component_t component;
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

static iot_bus_topic_t * iot_bus_topic_find_locked (iot_bus_t * bus, const char * name)
{
  iot_bus_topic_t * topic = bus->topics;
  while (topic)
  {
    if (strcmp (topic->name, name) == 0) break;
    topic = topic->next;
  }
  return topic;
}

static iot_bus_topic_t * iot_bus_topic_create_locked (iot_bus_t * bus, const char * name, const int * prio)
{
  iot_bus_topic_t * topic = iot_bus_topic_find_locked (bus, name);
  if (topic == NULL)
  {
    topic = malloc (sizeof (*topic));
    topic->name = iot_strdup (name);
    topic->next = bus->topics;
    bus->topics = topic;
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

static iot_bus_sub_t * iot_bus_find_sub_locked (iot_bus_t * bus, void * self, const char * pattern, iot_bus_sub_t ** prev)
{
  iot_bus_sub_t * sub = bus->subscribers;
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

static iot_bus_pub_t * iot_bus_find_pub_locked (iot_bus_t * bus, void * self, const char * topic, iot_bus_pub_t ** prev)
{
  iot_bus_pub_t * pub = bus->publishers;
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

  while (true)
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
  iot_bus_pub_t *pub = (iot_bus_pub_t *) arg;
  iot_bus_publish (pub, (pub->callback) (pub->self), true);
}

iot_bus_t * iot_bus_alloc (iot_scheduler_t * scheduler, uint64_t default_poll_interval)
{
  assert (scheduler);
  iot_bus_t * bus = calloc (1, sizeof (*bus));
  pthread_rwlock_init (&bus->lock, NULL);
  bus->component.start_fn = (iot_component_start_fn_t) iot_bus_start;
  bus->component.stop_fn = (iot_component_stop_fn_t) iot_bus_stop;
  bus->interval = default_poll_interval * 1000;
  bus->scheduler = scheduler;
  bus->threadpool = iot_scheduler_thread_pool (scheduler);
  return bus;
}

void iot_bus_topic_create (iot_bus_t * bus, const char * name, const int * prio)
{
  assert (bus && name);
  pthread_rwlock_wrlock (&bus->lock);
  iot_bus_topic_create_locked (bus, name, prio);
  pthread_rwlock_unlock (&bus->lock);
}

bool iot_bus_start (iot_bus_t * bus)
{
  assert (bus);
  iot_scheduler_start (bus->scheduler);
  return true;
}

bool iot_bus_stop (iot_bus_t * bus)
{
  assert (bus);
  iot_scheduler_stop (bus->scheduler);
  return true;
}

void iot_bus_free (iot_bus_t * bus)
{
  if (bus)
  {
    iot_bus_topic_t * topic;
    while (bus->publishers)
    {
      pthread_rwlock_wrlock (&bus->lock);
      iot_bus_pub_free_locked (bus->publishers);
      pthread_rwlock_unlock (&bus->lock);
    }
    while (bus->subscribers)
    {
      pthread_rwlock_wrlock (&bus->lock);
      iot_bus_sub_free_locked (bus->subscribers);
      pthread_rwlock_unlock (&bus->lock);
    }
    while ((topic = bus->topics))
    {
      bus->topics = topic->next;
      free (topic->name);
      free (topic);
    }
    pthread_rwlock_destroy (&bus->lock);
    free (bus);
  }
}

static void iot_bus_match_sub_locked (iot_bus_t * bus, iot_bus_sub_t * sub)
{
  iot_bus_pub_t * pub = bus->publishers;
  while (pub)
  {
    iot_bus_match_locked (pub, sub);
    pub = pub->next;
  }
}

static void iot_bus_match_pub_locked (iot_bus_t * bus, iot_bus_pub_t * pub)
{
  iot_bus_sub_t * sub = bus->subscribers;
  while (sub)
  {
    iot_bus_match_locked (pub, sub);
    sub = sub->next;
  }
}

iot_bus_sub_t * iot_bus_sub_alloc (iot_bus_t * bus, void * self, iot_data_sub_fn_t callback, const char * pattern)
{
  assert (bus && callback && pattern);

  pthread_rwlock_wrlock (&bus->lock);
  iot_bus_sub_t * sub = iot_bus_find_sub_locked (bus, self, pattern, NULL);
  if (sub == NULL)
  {
    sub = calloc (1, sizeof (*sub));
    sub->self = self;
    sub->bus = bus;
    sub->pattern = iot_strdup (pattern);
    sub->callback = callback;
    if (bus->subscribers)
    {
      sub->next = bus->subscribers;
    }
    atomic_store (&sub->refs, 1);
    bus->subscribers = sub;
    iot_bus_match_sub_locked (bus, sub);
  }
  pthread_rwlock_unlock (&bus->lock);
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

iot_bus_pub_t * iot_bus_pub_alloc (iot_bus_t * bus, void * self, iot_data_pub_cb_fn_t callback, const char * topic)
{
  assert (bus && topic);
  pthread_rwlock_wrlock (&bus->lock);
  iot_bus_pub_t * pub = iot_bus_find_pub_locked (bus, self, topic, NULL);
  if (pub == NULL)
  {
    pub = calloc (1, sizeof (*pub));
    pub->self = self;
    pub->bus = bus;
    pub->topic = iot_bus_topic_create_locked (bus, topic, NULL);
    pub->next = bus->publishers;
    atomic_store (&pub->refs, 1);
    bus->publishers = pub;
    if (callback)
    {
      pub->callback = callback;
      pub->sc = iot_schedule_create (bus->scheduler, iot_bus_sched_fn, pub, bus->interval, 0, 0, pub->topic->prio_set ? &pub->topic->priority : NULL);
      iot_schedule_add (bus->scheduler, pub->sc);
    }
    iot_bus_match_pub_locked (bus, pub);
  }
  pthread_rwlock_unlock (&bus->lock);
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

/* Container support */

static iot_component_t * iot_bus_config (iot_container_t * cont, const iot_data_t * map)
{
  const iot_data_t * value = iot_data_string_map_get (map, "Interval");
  uint64_t interval = value ? (uint64_t) iot_data_i64 (value) : IOT_BUS_DEFAULT_INTERVAL;
  const char * name = iot_data_string_map_get_string (map, "Scheduler");
  assert (name);
  iot_scheduler_t * scheduler = (iot_scheduler_t*) iot_container_find (cont, name);
  assert (scheduler);
  iot_bus_t * bus = iot_bus_alloc (scheduler, interval);
  value = iot_data_string_map_get (map, "Topics");
  if (value)
  {
    iot_data_array_iter_t iter;
    iot_data_array_iter (value, &iter);
    while (iot_data_array_iter_next (&iter))
    {
      const iot_data_t * map = iot_data_array_iter_value (&iter);
      name = iot_data_string_map_get_string (map, "Topic");
      value = iot_data_string_map_get (map, "Priority");
      int prio = (int) iot_data_i64 (value);
      iot_bus_topic_create_locked (bus, name, &prio);
    }
  }
  return &bus->component;
}

const iot_component_factory_t * iot_bus_factory (void)
{
  static iot_component_factory_t factory = { IOT_BUS_TYPE, iot_bus_config, (iot_component_free_fn_t) iot_bus_free };
  return &factory;
}
