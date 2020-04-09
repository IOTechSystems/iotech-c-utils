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
  iot_dlhandle_holder_t * handles;
  iot_component_holder_t ** components;
  uint32_t ccount;
  uint32_t csize;
  char * name;
  pthread_rwlock_t lock;
  iot_container_t * next;
  iot_container_t * prev;
};

static const iot_component_factory_t * iot_component_factories = NULL;
static iot_container_t * iot_containers = NULL;
static const iot_container_config_t * iot_config = NULL;
#ifdef __ZEPHYR__
  static PTHREAD_MUTEX_DEFINE (iot_container_mutex);
#else
  static pthread_mutex_t iot_container_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static iot_container_t * iot_container_find_locked (const char * name)
{
  assert (name);
  iot_container_t * cont = iot_containers;
  while (cont)
  {
    if (strcmp (cont->name, name) == 0) break;
    cont = cont->next;
  }
  return cont;
}

static void iot_component_create_ch (iot_container_t * cont, const char *cname, const iot_component_factory_t * factory, const char * config, bool init)
{
  iot_data_t * cmap = iot_data_from_json (config);
  iot_component_t * comp = (factory->config_fn) (cont, cmap);
  iot_data_free (cmap);
  if (comp)
  {
    iot_component_holder_t * ch = malloc (sizeof (*ch));
    ch->component = comp;
    ch->name = strdup (cname);
    ch->factory = factory;
    if ((cont->ccount + 1) == cont->csize)
    {
      init == true ? cont->csize += IOT_COMPONENT_DELTA : cont->csize++;
      cont->components = realloc (cont->components, cont->csize * sizeof (iot_component_holder_t));
    }
    cont->components[cont->ccount++] = ch;
  }
}

static const iot_component_factory_t * iot_component_factory_find_locked (const char * type)
{
  assert (type);
  const iot_component_factory_t * iter = iot_component_factories;
  while (iter)
  {
    if (strcmp (iter->type, type) == 0) break;
    iter = iter->next;
  }
  return iter;
}

static iot_component_holder_t * iot_get_component_holder (iot_container_t * cont, const char * comp_name, uint32_t * index)
{
  assert (cont && comp_name);
  iot_component_holder_t * ch = NULL;
  iot_component_t * comp = iot_container_find_component (cont, comp_name);
  if (comp)
  {
    int i = 0;
    while (cont->ccount)
    {
      ch = cont->components[i++];
      if (strcmp (ch->name, comp_name) == 0)
      {
        if (index)
        {
          *index = i; //return index
        }
        break;
      }
    }
  }
  return ch;
}

#ifdef IOT_BUILD_DYNAMIC_LOAD
static void iot_container_add_handle (iot_container_t * cont,  void * handle)
{
  assert (cont && handle);
  iot_dlhandle_holder_t * holder = malloc (sizeof (*holder));
  holder->load_handle = handle;
  pthread_rwlock_wrlock (&cont->lock);
  holder->next = cont->handles;
  cont->handles = holder;
  pthread_rwlock_unlock (&cont->lock);
}

static void iot_container_load_component (iot_container_t * cont, const char * config)
{
  void *handle = NULL;
  const iot_component_factory_t *factory = NULL;

  iot_data_t * cmap = iot_data_from_json (config);
  const iot_data_t * value = iot_data_string_map_get (cmap, "Library");
  if (value)
  {
    const char *library_name = (const char *) (iot_data_string (value));
    handle = dlopen (library_name, RTLD_LAZY);
    if (handle)
    {
      // find the symbol and call config_fn
      value = iot_data_string_map_get (cmap, "Factory");
      if (value)
      {
        const char * factory_name = (const char *) (iot_data_string (value));
        const iot_component_factory_t *(*factory_fn) (void);
        factory_fn = dlsym (handle, factory_name);
        if (factory_fn)
        {
          factory = factory_fn ();
          iot_component_factory_add (factory);
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
#endif

void iot_container_config (iot_container_config_t * conf)
{
  assert (conf);
  iot_config = conf;
}

iot_container_t * iot_container_alloc (const char * name)
{
  iot_container_t * cont = NULL;
  pthread_mutex_lock (&iot_container_mutex);
  if (iot_container_find_locked (name) == NULL)
  {
    cont = calloc (1, sizeof (*cont));
    cont->components = calloc (IOT_COMPONENT_DELTA, sizeof (iot_component_holder_t));
    cont->csize = IOT_COMPONENT_DELTA;
    cont->name = strdup (name);
    pthread_rwlock_init (&cont->lock, NULL);
    cont->next = iot_containers;
    if (iot_containers) iot_containers->prev = cont;
    iot_containers = cont;
  }
  pthread_mutex_unlock (&iot_container_mutex);
  return cont;
}

bool iot_container_init (iot_container_t * cont)
{
  assert (iot_config && cont);
  bool ret = true;
  char * config = (iot_config->load) (cont->name, iot_config->uri);
  assert (config);
  iot_data_t * map = iot_data_from_json (config);
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  free (config);

#ifdef IOT_BUILD_DYNAMIC_LOAD

  // pre-pass to find the factory to be added to support dynamic loading of libraries
  while (iot_data_map_iter_next (&iter))
  {
    const char *cname = iot_data_map_iter_string_key(&iter);
    const iot_component_factory_t *factory = NULL;

    const char * ctype = iot_data_map_iter_string_value (&iter);
    config = (iot_config->load) (cname, iot_config->uri);
    factory = iot_component_factory_find (ctype);

    if ((!factory) && (config))
    {
      iot_container_load_component (cont, config);
    }
    free (config);
  }
#endif

  while (iot_data_map_iter_next (&iter))
  {
    const char * cname = iot_data_map_iter_string_key (&iter);
    const char * ctype = iot_data_map_iter_string_value (&iter);
    const iot_component_factory_t * factory = iot_component_factory_find (ctype);
    if (factory)
    {
      config = (iot_config->load) (cname, iot_config->uri);
      if (config)
      {
        iot_component_create_ch (cont, cname, factory, config, true);
      }
      free (config);
    }
  }
  iot_data_free (map);
  return ret;
}

void iot_container_free (iot_container_t * cont)
{
  if (cont)
  {
    pthread_mutex_lock (&iot_container_mutex);
    if (cont->next) cont->next->prev = cont->prev;
    if (cont->prev)
    {
      cont->prev->next = cont->next;
    }
    else
    {
      iot_containers = cont->next;
    }
    pthread_mutex_unlock (&iot_container_mutex);
    while (cont->ccount)
    {
      iot_component_holder_t *ch = cont->components[--cont->ccount]; // Free in reverse of declaration order (dependents last)
      (ch->factory->free_fn) (ch->component);
      free (ch->name);
      free (ch);
    }
    free (cont->components);
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
    free (cont->name);
    free (cont);
  }
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

void iot_component_factory_add (const iot_component_factory_t * factory)
{
  assert (factory);
  pthread_mutex_lock (&iot_container_mutex);
  if (iot_component_factory_find_locked (factory->type) == NULL)
  {
    ((iot_component_factory_t*) factory)->next = iot_component_factories;
    iot_component_factories = factory;
  }
  pthread_mutex_unlock (&iot_container_mutex);
}

extern const iot_component_factory_t * iot_component_factory_find (const char * type)
{
  pthread_mutex_lock (&iot_container_mutex);
  const iot_component_factory_t * factory = iot_component_factory_find_locked (type);
  pthread_mutex_unlock (&iot_container_mutex);
  return factory;
}

void iot_container_add_component (iot_container_t * cont, const char * ctype, const char *cname, const char * config)
{
  assert (cont && config);

  const iot_component_factory_t *factory = NULL;
  factory = iot_component_factory_find (ctype);
  
  /* For dynamically loaded components */
#ifdef IOT_BUILD_DYNAMIC_LOAD
  if (!factory)
  {
    iot_container_load_component (cont, config);
  }
#endif
  /* instantiate the component */
  factory != NULL  ? iot_component_create_ch (cont, cname, factory, config, false) : fprintf (stderr, "ERROR: factory not available, cannot add a component\n");
}

extern iot_container_t * iot_container_find (const char * name)
{
  pthread_mutex_lock (&iot_container_mutex);
  iot_container_t * cont = iot_container_find_locked (name);
  pthread_mutex_unlock (&iot_container_mutex);
  return cont;
}

iot_component_t * iot_container_find_component (iot_container_t * cont, const char * name)
{
  assert (cont);
  iot_component_t * comp = NULL;
  if (name)
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

void iot_container_delete_component (iot_container_t *cont, const char * name)
{
  uint32_t index = 0;
  iot_component_holder_t * ch = iot_get_component_holder (cont, name, &index);
  assert (ch);
  if (ch->component->state != IOT_COMPONENT_STOPPED)
  {
    ch->component->stop_fn (ch->component);
  }
  ch->factory->free_fn (ch->component);

  /* reindex */
  if (index != cont->ccount) // not a last added component
  {
    for (int i = index - 1; i < cont->ccount; i++)
    {
      cont->components[i] = cont->components[i+1];
    }
  }
  cont->ccount--;

  free (ch->name);
  free (ch);
}

iot_component_info_t * iot_container_list_components (iot_container_t * cont)
{
  assert (cont);
  iot_component_info_t * components = calloc (1, sizeof (*components));

  components->count = cont->ccount;
  components->componentInfo = calloc (cont->ccount, sizeof (iot_component_data_t));

  for (int index = 0; index < cont->ccount; index++)
  {
    iot_component_data_t * component_data = malloc (sizeof (*component_data));
    component_data->name = strdup (cont->components[index]->name);
    component_data->type = strdup (cont->components[index]->factory->type);
    component_data->state = cont->components[index]->component->state;
    components->componentInfo[index] = component_data;
  }
  return components;
}

iot_data_t * iot_container_list_containers ()
{
  iot_container_t * cont = iot_containers;
  iot_data_t * cont_map = iot_data_alloc_map (IOT_DATA_STRING);

  while (cont)
  {
    iot_data_string_map_add (cont_map, "name", iot_data_alloc_string (cont->name, IOT_DATA_REF));
    cont = cont->next;
  }
  return cont_map;
}
