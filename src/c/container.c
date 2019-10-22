//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/container.h"
#include "iot/logger.h"
#ifdef IOT_BUILD_DYNAMIC_LOAD
#include <dlfcn.h>
#endif

#define IOT_COMPONENT_DELTA 4

typedef struct iot_factory_holder_t
{
  struct iot_factory_holder_t * next;
  const iot_component_factory_t * factory;
} iot_factory_holder_t;

typedef struct iot_dlhandle_holder_t
{
  struct iot_dlhandle_holder_t * next;
  void * load_handle;
} iot_dlhandle_holder_t;

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
  iot_dlhandle_holder_t * handles;
  iot_component_holder_t ** components;
  uint32_t ccount;
  uint32_t csize;
  pthread_rwlock_t lock;
};

iot_container_t * iot_container_alloc (void)
{
  iot_container_t * cont = calloc (1, sizeof (*cont));
  cont->components = calloc (IOT_COMPONENT_DELTA, sizeof (iot_component_holder_t));
  cont->ccount = 0;
  cont->csize = IOT_COMPONENT_DELTA;
  pthread_rwlock_init (&cont->lock, NULL);
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

#ifdef IOT_BUILD_DYNAMIC_LOAD
static void iot_container_add_handle (iot_container_t * cont,  void * handle)
{
  assert (cont && handle);
  iot_dlhandle_holder_t * holder = malloc (sizeof (*holder));
  holder->load_handle = handle;
  pthread_rwlock_wrlock(&cont->lock);
  holder->next = cont->handles;
  cont->handles = holder;
  pthread_rwlock_unlock(&cont->lock);
}
#endif

bool iot_container_init (iot_container_t * cont, const char * name, iot_container_config_t * conf)
{
  assert (conf);
  bool ret = true;
  char * config = (conf->load) (name, conf->from);
  assert (config);
  iot_data_t * map = iot_data_from_json (config);
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  free (config);

  //pre-pass to find the factory to be added to support dynamic loading of libraries
#ifdef IOT_BUILD_DYNAMIC_LOAD
  while (iot_data_map_iter_next(&iter))
  {
    const char *cname = iot_data_map_iter_string_key(&iter);
    const iot_component_factory_t *factory = NULL;
    void *handle = NULL;

    const char * ctype = iot_data_map_iter_string_value (&iter);
    factory = iot_container_find_factory_locked (cont, ctype);
    if (!factory)
    {
      config = (conf->load)(cname, conf->from);
      if (config)
      {
        iot_data_t *cmap = iot_data_from_json(config);
        const iot_data_t *value = iot_data_string_map_get(cmap, "Library");
        if (value)
        {
          const char *library_name = (const char *) (iot_data_string(value));
          handle = dlopen(library_name, RTLD_LAZY);
          if (handle)
          {
            //find the symbol and call config_fn
            value = iot_data_string_map_get(cmap, "Factory");
            if (value)
            {
              const char *factory_name = (const char *) (iot_data_string(value));
              const iot_component_factory_t *(*factory_fn)(void);
              factory_fn = dlsym(handle, factory_name);
              if (factory_fn)
              {
                factory = factory_fn();
                iot_container_add_factory (cont, factory);
                iot_container_add_handle (cont, handle);
              }
              else
              {
                fprintf (stderr, "ERROR: Invalid configuration, Incorrect Factory name\n");
              }
            }
            else
            {
              fprintf (stderr, "ERROR: Incomplete configuration, Factory name not available\n");
              dlclose (handle);
            }
          }
          else
          {
            fprintf (stderr, "ERROR: Incomplete configuration, Library name not available\n");
          }
        }
        iot_data_free (cmap);
      }
      free (config);
    }
  }
#endif

  while (iot_data_map_iter_next (&iter))
  {
    const char * cname = iot_data_map_iter_string_key (&iter);
    const char * ctype = iot_data_map_iter_string_value (&iter);
    const iot_component_factory_t * factory = iot_container_find_factory_locked (cont, ctype);
    if (factory)
    {
      config = (conf->load) (cname, conf->from);
      if (config)
      {
        iot_data_t * cmap = iot_data_from_json (config);
        iot_component_t * comp = (factory->config_fn) (cont, cmap);
        iot_data_free (cmap);
        free (config);
        if (comp)
        {
          iot_component_holder_t * ch = malloc (sizeof (*ch));
          ch->component = comp;
          ch->name = iot_strdup (cname);
          ch->factory = factory;
          if ((cont->ccount + 1) == cont->csize)
          {
            cont->csize += IOT_COMPONENT_DELTA;
            cont->components = realloc (cont->components, cont->csize * sizeof (iot_component_holder_t));
          }
          cont->components[cont->ccount++] = ch;
        }
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
    iot_component_holder_t * ch = cont->components[--cont->ccount]; // Free in reverse of declaration order (dependents last)
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
#ifdef IOT_BUILD_DYNAMIC_LOAD
  while (cont->handles)
  {
    iot_dlhandle_holder_t *dl_handles = cont->handles;
    cont->handles = dl_handles->next;
    dlclose (dl_handles->load_handle);
    free (dl_handles);
  }
#endif
  pthread_rwlock_destroy (&cont->lock);
  free (cont);
}

bool iot_container_start (iot_container_t * cont)
{
  bool ret = true;
  pthread_rwlock_rdlock (&cont->lock);
  for (uint32_t i = 0; i < cont->ccount; i++) // Start in declaration order (dependents first)
  {
    iot_component_t * comp = cont->components[i]->component;
    ret = ret && (comp->start_fn) (comp);
  }
  pthread_rwlock_unlock (&cont->lock);
  return ret;
}

void iot_container_stop (iot_container_t * cont)
{
  pthread_rwlock_rdlock (&cont->lock);
  for (int32_t i = cont->ccount - 1; i >= 0; i--) // Stop in reverse of declaration order (dependents last)
  {
    iot_component_t * comp = cont->components[i]->component;
    (comp->stop_fn) (comp);
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
  iot_component_t * comp = NULL;
  if (name && (name[0] != '\0'))
  {
    pthread_rwlock_rdlock (&cont->lock);
    for (uint32_t i = 0; i < cont->ccount; i++)
    {
      if (strcmp (cont->components[i]->name, name) == 0)
      {
        comp = cont->components[i]->component;
        break;
      }
    }
    pthread_rwlock_unlock (&cont->lock);
  }
  return comp;
}
