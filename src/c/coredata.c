#include "iot/scheduler.h"
#include "iot/coredata.h"

#define IOT_COREDATA_DEFAULT_INTERVAL 500000000
#define IOT_COREDATA_DEFAULT_THREADS 2

typedef struct iot_coredata_topic_t
{
  struct iot_coredata_topic_t * next;
  char * name;
  int priority;
  bool prio_set;
} iot_coredata_topic_t;

typedef struct iot_coredata_match_t
{
  iot_coredata_sub_t * sub;
  struct iot_coredata_match_t * next;
} iot_coredata_match_t;

struct iot_coredata_pub_t
{
  iot_coredata_t * coredata;
  struct iot_coredata_pub_t * next;
  const iot_coredata_topic_t * topic;
  void * self;
  iot_coredata_match_t * matches;
  iot_data_pub_cb_fn_t callback;
  iot_schedule_t * sc;
};

struct iot_coredata_sub_t
{
  iot_coredata_t * coredata;
  struct iot_coredata_sub_t * next;
  void * self;
  char * pattern;
  iot_data_sub_fn_t callback;
};

struct iot_coredata_t
{
  iot_coredata_sub_t * subscribers;
  iot_coredata_pub_t * publishers;
  iot_coredata_topic_t * topics;
  iot_threadpool * thpool;
  iot_scheduler_t * scheduler;
  uint64_t interval;
  pthread_rwlock_t lock;
};

static iot_coredata_topic_t * iot_coredata_topic_find_locked (iot_coredata_t * cd, const char * name)
{
  iot_coredata_topic_t * topic = cd->topics;
  while (topic)
  {
    if (strcmp (topic->name, name) == 0) break;
    topic = topic->next;
  }
  return topic;
}

static iot_coredata_topic_t * iot_coredata_topic_create_locked (iot_coredata_t * cd, const char * name, const int * prio)
{
  iot_coredata_topic_t * topic = iot_coredata_topic_find_locked (cd, name);
  if (topic == NULL)
  {
    topic = malloc (sizeof (*topic));
    topic->name = strdup (name);
    topic->prio_set = (prio != NULL);
    topic->priority = prio ? *prio : 0;
    topic->next = cd->topics;
    cd->topics = topic;
  }
  return topic;
}

static void iot_coredata_sub_free_locked (iot_coredata_sub_t * sub)
{
  iot_coredata_sub_t * prev = NULL;
  iot_coredata_sub_t * iter = sub->coredata->subscribers;
  iot_coredata_pub_t * pub = sub->coredata->publishers;

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
        sub->coredata->subscribers = iter->next;
      }
      break;
    }
    prev = iter;
    iter = iter->next;
  }

  while (pub)
  {
    iot_coredata_match_t * mprev = NULL;
    iot_coredata_match_t * match = pub->matches;
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

static void iot_coredata_pub_free_locked (iot_coredata_pub_t * pub)
{
  iot_coredata_match_t * match;
  iot_coredata_pub_t * prev = NULL;
  iot_coredata_pub_t * iter = pub->coredata->publishers;

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
        pub->coredata->publishers = iter->next;
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

static iot_coredata_sub_t * iot_coredata_find_sub_locked (iot_coredata_t * cd, void * self, const char * pattern, iot_coredata_sub_t ** prev)
{
  iot_coredata_sub_t * sub = cd->subscribers;
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

static iot_coredata_pub_t * iot_coredata_find_pub_locked (iot_coredata_t * cd, void * self, const char * topic, iot_coredata_pub_t ** prev)
{
  iot_coredata_pub_t * pub = cd->publishers;
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

static inline bool iot_coredata_pubsub_match_locked (iot_coredata_pub_t * pub, iot_coredata_sub_t * sub)
{
  return strcmp (pub->topic->name, sub->pattern) == 0; // TODO: Implement pattern match
}

static void iot_coredata_sched_fn (void * arg)
{
  iot_coredata_pub_t * pub = (iot_coredata_pub_t*) arg;
  iot_data_t * data = (pub->callback) (pub->self);
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
  cd->scheduler = iot_scheduler_init (cd->thpool);
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
    iot_coredata_topic_t * topic;
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
    while ((topic = cd->topics))
    {
      cd->topics = topic->next;
      free (topic->name);
      free (topic);
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
      match->sub = sub;
      match->next = pub->matches;
    }
    pub = pub->next;
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
      match->sub = sub;
      match->next = pub->matches;
      pub->matches = match;
    }
    sub = sub->next;
  }
}

iot_coredata_sub_t * iot_coredata_sub_alloc (iot_coredata_t * cd, void * self, iot_data_sub_fn_t callback, const char * pattern)
{
  assert (cd && callback && pattern);

  pthread_rwlock_wrlock (&cd->lock);
  iot_coredata_sub_t * sub = iot_coredata_find_sub_locked (cd, self, pattern, NULL);
  if (sub == NULL)
  {
    sub = calloc (1, sizeof (*sub));
    sub->self = self;
    sub->coredata = cd;
    sub->pattern = strdup (pattern);
    sub->callback = callback;
    if (cd->subscribers)
    {
      sub->next = cd->subscribers;
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
    pthread_rwlock_t * lock = &sub->coredata->lock;
    pthread_rwlock_wrlock (lock);
    iot_coredata_sub_free_locked (sub);
    pthread_rwlock_unlock (lock);
  }
}

iot_coredata_pub_t * iot_coredata_pub_alloc (iot_coredata_t * cd, void * self, iot_data_pub_cb_fn_t callback, const char * topic)
{
  assert (cd && topic);
  pthread_rwlock_wrlock (&cd->lock);
  iot_coredata_pub_t * pub = iot_coredata_find_pub_locked (cd, self, topic, NULL);
  if (pub == NULL)
  {
    pub = calloc (1, sizeof (*pub));
    pub->self = self;
    pub->coredata = cd;
    pub->topic = iot_coredata_topic_create_locked (cd, topic, NULL);
    pub->next = cd->publishers;
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
    pthread_rwlock_t *lock = &pub->coredata->lock;
    if (pub->sc)
    {
      iot_schedule_remove (pub->coredata->scheduler, pub->sc);
    }
    pthread_rwlock_wrlock (lock);
    iot_coredata_pub_free_locked (pub);
    pthread_rwlock_unlock (lock);
  }
}

void iot_coredata_publish (iot_coredata_pub_t * pub, iot_data_t * data)
{
  assert (pub && data);

  pthread_rwlock_rdlock (&pub->coredata->lock);
  iot_coredata_match_t * match = pub->matches;
  while (match)
  {
    (match->sub->callback) (data, match->sub->self, pub->topic->name);
    match = match->next;
  }
  pthread_rwlock_unlock (&pub->coredata->lock);
  iot_data_free (data);
}
