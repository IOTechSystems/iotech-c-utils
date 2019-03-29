#include "iot/container.h"
#include "iot/data.h"
#include "iot/logger.h"

#define IOT_COMPONENT_DELTA 4

extern void iot_component_start (iot_component_t * comp);
extern void iot_component_stop (iot_component_t * comp);

typedef struct iot_factory_holder_t
{
  struct iot_factory_holder_t * next;
  const iot_component_factory_t * factory;
} iot_factory_holder_t;

typedef struct iot_component_holder_t
{
  iot_component_t * component;
  const iot_component_factory_t * factory;
  char * name;
} iot_component_holder_t;

struct iot_container_t
{
  iot_logger_t * logger;
  iot_factory_holder_t * factories;
  iot_component_holder_t ** components;
  iot_container_config_load_fn_t loader;
  uint32_t ccount;
  uint32_t csize;
  pthread_rwlock_t lock;
};

iot_container_t * iot_container_alloc (iot_container_config_load_fn_t loader)
{
  assert (loader);
  iot_container_t * cont = calloc (1, sizeof (*cont));
  cont->components = calloc (IOT_COMPONENT_DELTA, sizeof (iot_component_holder_t));
  cont->ccount = 0;
  cont->csize = IOT_COMPONENT_DELTA;
  pthread_rwlock_init (&cont->lock, NULL);
  cont->loader = loader;
  return cont;
}

static const iot_component_factory_t * iot_container_find_factory_locked (iot_container_t * cont, const char * type)
{
  iot_factory_holder_t * iter = cont->factories;
  while (iter)
  {
    if (strcmp (iter->factory->type, type) == 0) break;
    iter = iter->next;
  }
  return iter ? iter->factory : NULL;
}

bool iot_container_init (iot_container_t * cont, const char * name)
{
  bool ret = true;
  const char * config = (cont->loader) (name);
  assert (config);
  iot_data_t * map = iot_data_from_json (config);
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    const char * cname = iot_data_map_iter_string_key (&iter);
    const char * ctype = iot_data_map_iter_string_value (&iter);
    const iot_component_factory_t * factory = iot_container_find_factory_locked (cont, ctype);
    const char * cconf = (cont->loader) (cname);
    if (factory && cconf)
    {
      iot_data_t * cmap = iot_data_from_json (cconf);
      iot_component_t * comp = (factory->config_fn) (cont, cmap);
      iot_data_free (cmap);
      if (comp)
      {
        iot_component_holder_t * ch = malloc (sizeof (*ch));
        ch->component = comp;
        ch->name = iot_strdup (cname);
        ch->factory = factory;
        pthread_rwlock_wrlock (&cont->lock);
        if ((cont->ccount + 1) == cont->csize)
        {
          cont->csize += IOT_COMPONENT_DELTA;
          cont->components = realloc (cont->components, cont->csize * sizeof (iot_component_holder_t));
        }
        cont->components[cont->ccount++] = ch;
        pthread_rwlock_unlock (&cont->lock);
      }
    }
  }
  iot_data_free (map);
  return ret;
}

void iot_container_free (iot_container_t * cont)
{
  while (cont->ccount)
  {
    iot_component_holder_t * ch = cont->components[--cont->ccount];
    (ch->factory->free_fn) (ch->component);
    free (ch->name);
    free (ch);
  }
  free (cont->components);
  while (cont->factories)
  {
    iot_factory_holder_t * fh = cont->factories;
    cont->factories = fh->next;
    free (fh);
  }
  pthread_rwlock_destroy (&cont->lock);
  free (cont);
}

bool iot_container_start (iot_container_t * cont)
{
  bool ret = true;
  pthread_rwlock_rdlock (&cont->lock);
  for (uint32_t i = 0; i < cont->ccount; i++)
  {
    iot_component_start (cont->components[i]->component);
    ret = ret && (cont->components[i]->component->state == IOT_COMPONENT_RUNNING);
  }
  pthread_rwlock_unlock (&cont->lock);
  return ret;
}

void iot_container_stop (iot_container_t * cont)
{
  pthread_rwlock_rdlock (&cont->lock);
  for (int32_t i = cont->ccount - 1; i >= 0; i--)
  {
    iot_component_stop (cont->components[i]->component);
  }
  pthread_rwlock_unlock (&cont->lock);
}

void iot_container_add_factory (iot_container_t * cont, const iot_component_factory_t * factory)
{
  assert (cont && factory);
  iot_factory_holder_t * holder = malloc (sizeof (*holder));
  holder->factory = factory;
  pthread_rwlock_wrlock (&cont->lock);
  holder->next = cont->factories;
  cont->factories = holder;
  pthread_rwlock_unlock (&cont->lock);
}

iot_component_t * iot_container_find (iot_container_t * cont, const char * name)
{
  iot_component_holder_t * iter = NULL;
  pthread_rwlock_rdlock (&cont->lock);
  for (uint32_t i = 0; i < cont->ccount; i++)
  {
    iter = cont->components[i];
    if (strcmp (iter->name, name) == 0)
    {
      break;
    }
  }
  pthread_rwlock_unlock (&cont->lock);
  return iter ? iter->component : NULL;
}