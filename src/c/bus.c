//
// Copyright (c) 2019 IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/scheduler.h"
#include "iot/bus.h"
#include "iot/container.h"
#include "iot/thread.h"

#define IOT_BUS_DEFAULT_INTERVAL 500000 // usecs

typedef struct iot_bus_sub_match_t
{
  iot_bus_sub_t * sub;
  struct iot_bus_sub_match_t * next;
} iot_bus_sub_match_t;

typedef struct iot_bus_topic_t
{
  struct iot_bus_topic_t * next;
  char * name;
  int priority;
  bool prio_set;
  bool retain;
  uint64_t count;
  iot_data_t * last;
  iot_bus_sub_match_t * matches;
  pthread_mutex_t mutex;
} iot_bus_topic_t;

typedef struct iot_bus_topic_match_t
{
  iot_bus_topic_t * topic;
  uint64_t count;
  struct iot_bus_topic_match_t * next;
} iot_bus_topic_match_t;

struct iot_bus_pub_t
{
  iot_bus_t * bus;
  struct iot_bus_pub_t * next;
  iot_bus_topic_t * topic;
  void * self;
  iot_data_pub_cb_fn_t callback;
  iot_schedule_t * schedule;
  atomic_uint_fast32_t refs;
};

struct iot_bus_sub_t
{
  iot_bus_t * bus;
  struct iot_bus_sub_t * next;
  void * self;
  char * pattern;
  iot_bus_topic_match_t * matches;
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

static iot_bus_topic_t * iot_bus_topic_create_locked (iot_bus_t * bus, const char * name, bool retain, const int * prio, bool * exists)
{
  iot_bus_topic_t * topic = bus->topics;
  while (topic)
  {
    if (strcmp (topic->name, name) == 0) break;
    topic = topic->next;
  }
  if (exists) *exists = topic != NULL;
  if (topic == NULL)
  {
    topic = calloc (1, sizeof (*topic));
    iot_mutex_init (&topic->mutex);
    topic->name = iot_strdup (name);
    topic->prio_set = (prio != NULL);
    topic->priority = prio ? *prio : 0;
    topic->retain = retain;
    topic->next = bus->topics;
    bus->topics = topic;
  }
  return topic;
}

static void iot_bus_sub_free_locked (iot_bus_sub_t * sub)
{
  iot_bus_sub_t * prev = NULL;
  iot_bus_sub_t * iter = sub->bus->subscribers;
  iot_bus_topic_t * topic = sub->bus->topics;

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
  while (topic)
  {
    iot_bus_sub_match_t * mprev = NULL;
    iot_bus_sub_match_t * match = topic->matches;
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
          topic->matches = match->next;
        }
        free (match);
        break;
      }
      mprev = match;
      match = match->next;
    }
    topic = topic->next;
  }
  while (sub->matches)
  {
    iot_bus_topic_match_t * tm = sub->matches;
    sub->matches = tm->next;
    free (tm);
  }
  free (sub->pattern);
  free (sub);
}

static void iot_bus_pub_free_locked (iot_bus_pub_t * pub)
{
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
  free (pub);
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
    if ((match = (*ptok == '#'))) break; // Multi level wildcard
    if ((*ptok != '+') && (strcmp (ttok, ptok) != 0)) break; // Single level wildcard
    ptok = NULL;
    ttok = NULL;
  }
  free (top);
  free (pat);
  return match;
}

static inline bool iot_bus_topic_priority_less (iot_bus_topic_t * t1, iot_bus_topic_t * t2)
{
  return (t1->prio_set) ? ((t2->prio_set) ? (t1->priority < t2->priority) : false) : (t2->prio_set);
}

static inline void iot_bus_match_sub_topic (iot_bus_sub_t * sub, iot_bus_topic_t * topic)
{
  iot_bus_topic_match_t * match = sub->matches;
  while (match)
  {
    if (match->topic == topic) break;
    match = match->next;
  }
  if (match == NULL) // Add match to subscriber ordering by topic priority (highest first)
  {
    iot_bus_topic_match_t * nm = malloc (sizeof (*match));
    iot_bus_topic_match_t * prev = NULL;
    nm->count = 0;
    nm->topic = topic;
    match = sub->matches;
    while (true)
    {
      if (match && iot_bus_topic_priority_less (topic, match->topic))
      {
        prev = match;
        match = match->next;
      }
      else
      {
        if (prev)
        {
          prev->next = nm;
          nm->next = match;
        }
        else
        {
          nm->next = sub->matches;
          sub->matches = nm;
        }
        break;
      }
    }
  }
}

// Match publishers with subscribers (by topic) unless same self

static void iot_bus_match_locked (iot_bus_pub_t * pub, iot_bus_sub_t * sub)
{
  iot_bus_topic_t * topic = pub->topic;
  if (((pub->self == NULL) || (pub->self != sub->self)) && iot_bus_topic_match (topic->name, sub->pattern))
  {
    iot_bus_sub_match_t * match = malloc (sizeof (*match));
    match->sub = sub;
    match->next = topic->matches;
    topic->matches = match;
    iot_bus_match_sub_topic (sub, topic);
  }
}

static void iot_bus_sched_fn (iot_bus_pub_t * pub)
{
  iot_data_t * data = (pub->callback) (pub->self);
  if (data) // Ignore if no data
  {
    iot_bus_pub_push (pub, data, true);
  }
}

iot_bus_t * iot_bus_alloc (iot_scheduler_t * scheduler, iot_threadpool_t * pool, uint64_t default_poll_interval)
{
  iot_bus_t * bus = calloc (1, sizeof (*bus));
  pthread_rwlock_init (&bus->lock, NULL);
  bus->component.start_fn = (iot_component_start_fn_t) iot_bus_start;
  bus->component.stop_fn = (iot_component_stop_fn_t) iot_bus_stop;
  bus->interval = default_poll_interval * 1000;
  bus->scheduler = scheduler;
  bus->threadpool = pool;
  return bus;
}

void iot_bus_topic_create (iot_bus_t * bus, const char * name, bool retain, const int * prio)
{
  assert (bus && name);
  pthread_rwlock_wrlock (&bus->lock);
  iot_bus_topic_create_locked (bus, name, retain, prio, NULL);
  pthread_rwlock_unlock (&bus->lock);
}

bool iot_bus_start (iot_bus_t * bus)
{
  pthread_rwlock_wrlock (&bus->lock);
  if (bus->component.state != IOT_COMPONENT_RUNNING)
  {
    bus->component.state = IOT_COMPONENT_RUNNING;
  }
  pthread_rwlock_unlock (&bus->lock);
  return true;
}

void iot_bus_stop (iot_bus_t * bus)
{
  pthread_rwlock_wrlock (&bus->lock);
  if (bus->component.state != IOT_COMPONENT_STOPPED)
  {
    bus->component.state = IOT_COMPONENT_STOPPED;
  }
  pthread_rwlock_unlock (&bus->lock);
}

void iot_bus_free (iot_bus_t * bus)
{
  if (bus)
  {
    iot_bus_stop (bus);
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
      iot_data_free (topic->last);
      pthread_mutex_destroy (&topic->mutex);
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
  assert (bus && pattern);

  iot_bus_sub_t * sub = calloc (1, sizeof (*sub));
  sub->self = self;
  sub->bus = bus;
  sub->pattern = iot_strdup (pattern);
  sub->callback = callback;
  pthread_rwlock_wrlock (&bus->lock);
  if (bus->subscribers)
  {
    sub->next = bus->subscribers;
  }
  atomic_store (&sub->refs, 1);
  bus->subscribers = sub;
  iot_bus_match_sub_locked (bus, sub);
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

iot_data_t * iot_bus_sub_pull (iot_bus_sub_t * sub)
{
  iot_data_t * ret = NULL;
  assert (sub);

  pthread_rwlock_rdlock (&sub->bus->lock);
  iot_bus_topic_match_t * match = sub->matches;
  while (match && (ret == NULL))
  {
    iot_bus_topic_t * topic = match->topic;
    if (topic->retain)
    {
      pthread_mutex_lock (&topic->mutex);
      if (match->count < topic->count)
      {
        match->count = topic->count;
        ret = topic->last;
        if (ret) iot_data_addref (ret);
      }
      pthread_mutex_unlock (&topic->mutex);
    }
    match = match->next;
  }
  pthread_rwlock_unlock (&sub->bus->lock);

  return ret;
}

iot_bus_pub_t * iot_bus_pub_alloc (iot_bus_t * bus, void * self, iot_data_pub_cb_fn_t callback, const char * topic_name)
{
  assert (bus && topic_name);
  bool existing;
  iot_bus_pub_t * pub = calloc (1, sizeof (*pub));
  pub->self = self;
  pub->bus = bus;
  pthread_rwlock_wrlock (&bus->lock);
  pub->topic = iot_bus_topic_create_locked (bus, topic_name, false, NULL, &existing);
  pub->next = bus->publishers;
  atomic_store (&pub->refs, 1);
  bus->publishers = pub;
  if (callback && bus->scheduler)
  {
    pub->callback = callback;
    pub->schedule = iot_schedule_create (bus->scheduler, (iot_schedule_fn_t) iot_bus_sched_fn, pub, bus->interval, 0, 0, pub->topic->prio_set ? &pub->topic->priority : NULL);
    iot_schedule_add (bus->scheduler, pub->schedule);
  }
  if (! existing)
  {
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
    pthread_rwlock_wrlock (lock);
    if (pub->schedule)
    {
      iot_schedule_remove (pub->bus->scheduler, pub->schedule);
    }
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

void iot_bus_pub_push (iot_bus_pub_t * pub, iot_data_t * data, bool sync)
{
  assert (pub && data);

  iot_bus_topic_t * topic = pub->topic;
  if (topic->retain) // If retaining data on topic, save current sample
  {
    iot_data_addref (data);
    pthread_mutex_lock (&topic->mutex);
    iot_data_t * last = topic->last;
    topic->count++;
    topic->last = data;
    iot_data_free (last);
    pthread_mutex_unlock (&topic->mutex);
  }

  pthread_rwlock_rdlock (&pub->bus->lock);
  iot_bus_sub_match_t * match = topic->matches;
  while (match)
  {
    if (sync)
    {
      (match->sub->callback) (data, match->sub->self, topic->name);
    }
    else
    {
      if (pub->bus->threadpool)
      {
        iot_bus_job_t *job = malloc (sizeof (*job));
        job->pub = pub;
        job->sub = match->sub;
        job->data = data;
        iot_data_addref (data);
        atomic_fetch_add (&pub->refs, 1);
        atomic_fetch_add (&job->sub->refs, 1);
        iot_threadpool_add_work (pub->bus->threadpool, iot_bus_publish_job, job, topic->prio_set ? &topic->priority : NULL);
      }
      else
      {
        break;
      }
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
  iot_scheduler_t * scheduler = (iot_scheduler_t*) iot_container_find (cont, name);
  name = iot_data_string_map_get_string (map, "ThreadPool");
  iot_threadpool_t * pool = (iot_threadpool_t*) iot_container_find (cont, name);
  iot_bus_t * bus = iot_bus_alloc (scheduler, pool, interval);
  value = iot_data_string_map_get (map, "Topics");
  if (value)
  {
    iot_data_array_iter_t iter;
    iot_data_array_iter (value, &iter);
    while (iot_data_array_iter_next (&iter))
    {
      const iot_data_t * mp = iot_data_array_iter_value (&iter);
      assert (mp);
      name = iot_data_string_map_get_string (mp, "Topic");
      value = iot_data_string_map_get (mp, "Priority");
      assert (name && value);
      int prio = (int) iot_data_i64 (value);
      value = iot_data_string_map_get (mp, "Retain");
      bool retain = value ? iot_data_bool (value) : false;
      iot_bus_topic_create_locked (bus, name, retain, &prio, NULL);
    }
  }
  return &bus->component;
}

const iot_component_factory_t * iot_bus_factory (void)
{
  static iot_component_factory_t factory = { IOT_BUS_TYPE, iot_bus_config, (iot_component_free_fn_t) iot_bus_free };
  return &factory;
}
