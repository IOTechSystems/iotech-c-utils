#include <pthread.h>
#include "iot/scheduler.h"
#include "iot/coredata.h"

#define IOT_COREDATA_DEFAULT_INTERVAL 500000000
#define IOT_COREDATA_DEFAULT_THREADS 2

typedef struct iot_coredata_pubsub_t
{
  iot_coredata_t * coredata;
  struct iot_coredata_pubsub_t * next;
  struct iot_coredata_pubsub_t * prev;
  void * self;
}
iot_coredata_pubsub_t;

struct iot_coredata_pub_t
{
  iot_coredata_pubsub_t base;
  char * topic;
  iot_data_pub_cb_fn_t callback;
  iot_schedule sc;
};

struct iot_coredata_sub_t
{
  iot_coredata_pubsub_t base;
  char * pattern;
  iot_data_sub_fn_t callback;
};

typedef struct iot_coredata_match_t
{
  iot_coredata_pubsub_t * pub;
  iot_coredata_pubsub_t * sub;
  struct iot_coredata_match_t * next;
}
iot_coredata_match_t;

struct iot_coredata_t
{
  iot_coredata_sub_t * subscribers;
  iot_coredata_pub_t * publishers;
  iot_coredata_match_t * matches;
  threadpool thpool;
  iot_scheduler scheduler;
  uint64_t interval;
  pthread_rwlock_t lock;
};

static void iot_coredata_unmatch_locked (iot_coredata_t * cd, iot_coredata_pubsub_t * base, bool subscriber)
{
  iot_coredata_match_t * match = cd->matches;
  iot_coredata_match_t * prev = NULL;
  iot_coredata_match_t * old;

  while (match)
  {
    if (subscriber ? (match->sub == base) : (match->pub == base))
    {
      if (prev)
      {
        prev->next = match->next;
      }
      else
      {
        cd->matches = match->next;
      }
      old = match;
      match = match->next;
      free (old);
    }
    else
    {
      prev = match;
      match = match->next;
    }
  }
}

static void iot_coredata_sub_free_locked (iot_coredata_sub_t * sub)
{
  iot_coredata_t * cd = sub->base.coredata;
  if (sub->base.prev)
  {
    sub->base.prev->next = sub->base.next;
  }
  else
  {
    cd->subscribers = (iot_coredata_sub_t*) sub->base.next;
  }
  if (sub->base.next)
  {
    sub->base.next->prev = sub->base.prev;
  }
  iot_coredata_unmatch_locked (cd, &sub->base, true);
  free (sub->pattern);
  free (sub);
}

static void iot_coredata_pub_free_locked (iot_coredata_pub_t * pub)
{
  iot_coredata_t * cd = pub->base.coredata;
  if (pub->base.prev)
  {
    pub->base.prev->next = pub->base.next;
  }
  else
  {
    cd->publishers = (iot_coredata_pub_t*) pub->base.next;
  }
  if (pub->base.next)
  {
    pub->base.next->prev = pub->base.prev;
  }
  iot_coredata_unmatch_locked (cd, &pub->base, false);
  free (pub->topic);
  free (pub);
}

static iot_coredata_sub_t * iot_coredata_find_sub_locked (iot_coredata_t * cd, void * self, const char * pattern, iot_coredata_sub_t ** prev)
{
  iot_coredata_sub_t * sub = cd->subscribers;
  while (sub)
  {
    if (sub->base.self == self && strcmp (sub->pattern, pattern) == 0)
    {
      break;
    }
    if (prev) *prev = sub;
    sub = (iot_coredata_sub_t*) sub->base.next;
  }
  return sub;
}

static iot_coredata_pub_t * iot_coredata_find_pub_locked (iot_coredata_t * cd, void * self, const char * topic, iot_coredata_pub_t ** prev)
{
  iot_coredata_pub_t * pub = cd->publishers;
  while (pub)
  {
    if (pub->base.self == self && strcmp (pub->topic, topic) == 0)
    {
      break;
    }
    if (prev) *prev = pub;
    pub = (iot_coredata_pub_t*) pub->base.next;
  }
  return pub;
}

static inline bool iot_coredata_pubsub_match_locked (iot_coredata_pub_t * pub, iot_coredata_sub_t * sub)
{
  return strcmp (pub->topic, sub->pattern) == 0; // TODO: Implement pattern match
}

static void iot_coredata_sched_fn (void * arg)
{
  iot_coredata_pub_t * pub = (iot_coredata_pub_t*) arg;
  iot_data_t * data = (pub->callback) (pub->base.self);
  iot_coredata_publish (pub, data);
}

iot_coredata_t * iot_coredata_alloc (void)
{
  iot_coredata_t * cd = calloc (1, sizeof (*cd));
  pthread_rwlock_init (&cd->lock, NULL);
  return cd;
}

void iot_coredata_init (iot_coredata_t * cd, iot_data_t * config)
{
  assert (cd);
  uint32_t threads = IOT_COREDATA_DEFAULT_THREADS;
  cd->interval = IOT_COREDATA_DEFAULT_INTERVAL;
  if (config)
  {
    const iot_data_t * value = iot_data_string_map_get (config, "Interval");
    if (value)
    {
      cd->interval = iot_data_ui64 (value);
    }
    value = iot_data_string_map_get (config, "Threads");
    if (value)
    {
      threads = iot_data_ui32 (value);
    }
  }
  cd->thpool = iot_thpool_init (threads);
  cd->scheduler = iot_scheduler_init (&cd->thpool);
}

void iot_coredata_start (iot_coredata_t * cd)
{
  assert (cd);
  iot_scheduler_start (cd->scheduler);
}

void iot_coredata_stop (iot_coredata_t * cd)
{
  assert (cd);
  iot_scheduler_stop (cd->scheduler);
}

void iot_coredata_free (iot_coredata_t * cd)
{
  if (cd)
  {
    while (cd->publishers)
    {
      pthread_rwlock_wrlock (&cd->lock);
      iot_coredata_pub_free_locked (cd->publishers);
      pthread_rwlock_unlock (&cd->lock);
    }
    while (cd->subscribers)
    {
      pthread_rwlock_wrlock (&cd->lock);
      iot_coredata_sub_free_locked (cd->subscribers);
      pthread_rwlock_unlock (&cd->lock);
    }
    iot_scheduler_fini (cd->scheduler);
    sleep (1);
    iot_thpool_destroy (cd->thpool);
    pthread_rwlock_destroy (&cd->lock);
    free (cd);
  }
}

static void iot_coredata_match_sub_locked (iot_coredata_t * cd, iot_coredata_sub_t * sub)
{
  iot_coredata_match_t * match;
  iot_coredata_pub_t * pub = cd->publishers;
  while (pub)
  {
    if (iot_coredata_pubsub_match_locked (pub, sub))
    {
      match = malloc (sizeof (*match));
      match->pub = &pub->base;
      match->sub = &sub->base;
      match->next = cd->matches;
      cd->matches = match;
    }
    pub = (iot_coredata_pub_t*) pub->base.next;
  }
}

static void iot_coredata_match_pub_locked (iot_coredata_t * cd, iot_coredata_pub_t * pub)
{
  iot_coredata_match_t * match;
  iot_coredata_sub_t * sub = cd->subscribers;
  while (sub)
  {
    if (iot_coredata_pubsub_match_locked (pub, sub))
    {
      match = malloc (sizeof (*match));
      match->pub = &pub->base;
      match->sub = &sub->base;
      match->next = cd->matches;
      cd->matches = match;
    }
    sub = (iot_coredata_sub_t*) sub->base.next;
  }
}

iot_coredata_sub_t * iot_coredata_sub_alloc (iot_coredata_t * cd, void * self, iot_data_sub_fn_t callback, const char * pattern)
{
  assert (callback);
  assert (pattern);
  pthread_rwlock_wrlock (&cd->lock);
  iot_coredata_sub_t * sub = iot_coredata_find_sub_locked (cd, self, pattern, NULL);
  if (sub == NULL)
  {
    sub = calloc (1, sizeof (*sub));
    sub->base.self = self;
    sub->base.coredata = cd;
    sub->pattern = strdup (pattern);
    sub->callback = callback;
    if (cd->subscribers)
    {
      sub->base.next = &cd->subscribers->base;
      cd->subscribers->base.prev = &sub->base;
    }
    cd->subscribers = sub;
    iot_coredata_match_sub_locked (cd, sub);
  }
  pthread_rwlock_unlock (&cd->lock);
  return sub;
}

void iot_coredata_sub_free (iot_coredata_sub_t * sub)
{
  if (sub)
  {
    pthread_rwlock_wrlock (&sub->base.coredata->lock);
    iot_coredata_sub_free_locked (sub);
    pthread_rwlock_unlock (&sub->base.coredata->lock);
  }
}

iot_coredata_pub_t * iot_coredata_pub_alloc (iot_coredata_t * cd, void * self, iot_data_pub_cb_fn_t callback, const char * topic)
{
  assert (topic);
  pthread_rwlock_wrlock (&cd->lock);
  iot_coredata_pub_t * pub = iot_coredata_find_pub_locked (cd, self, topic, NULL);
  if (pub == NULL)
  {
    pub = calloc (1, sizeof (*pub));
    pub->base.self = self;
    pub->base.coredata = cd;
    pub->topic = strdup (topic);
    pub->callback = callback;
    if (cd->publishers)
    {
      pub->base.next = &cd->publishers->base;
      cd->publishers->base.prev = &pub->base;
    }
    cd->publishers = pub;
    if (callback)
    {
      pub->callback = callback;
      pub->sc = iot_schedule_create (cd->scheduler, iot_coredata_sched_fn, pub, cd->interval, 0, 0);
      iot_schedule_add (cd->scheduler, pub->sc);
    }
    iot_coredata_match_pub_locked (cd, pub);
  }
  pthread_rwlock_unlock (&cd->lock);
  return pub;
}

void iot_coredata_pub_free (iot_coredata_pub_t * pub)
{
  if (pub)
  {
    if (pub->sc)
    {
      iot_schedule_remove (pub->base.coredata->scheduler, pub->sc);
    }
    pthread_rwlock_wrlock (&pub->base.coredata->lock);
    iot_coredata_pub_free_locked (pub);
    pthread_rwlock_unlock (&pub->base.coredata->lock);
  }
}

void iot_coredata_publish (iot_coredata_pub_t * pub, iot_data_t * data)
{
  assert (pub);
  assert (data);
  iot_coredata_t * cd = pub->base.coredata;
  iot_coredata_match_t * match;
  iot_coredata_sub_t * sub;
  pthread_rwlock_rdlock (&cd->lock);
  match = cd->matches;
  while (match)
  {
    if (match->pub == &pub->base)
    {
      sub = (iot_coredata_sub_t*) match->sub;
      (sub->callback) (data, sub->base.self, pub->topic);
    }
    match = match->next;
  }
  pthread_rwlock_unlock (&cd->lock);
  iot_data_free (data);
}
