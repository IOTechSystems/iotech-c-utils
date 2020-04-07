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
  pthread_rwlock_t lock;
};

static const iot_component_factory_t * iot_component_factories = NULL;
static pthread_mutex_t iot_component_factories_mutex = PTHREAD_MUTEX_INITIALIZER;

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
  iot_component_t * comp = iot_container_find (cont, comp_name);
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

iot_container_t * iot_container_alloc (void)
{
  iot_container_t * cont = calloc (1, sizeof (*cont));
  cont->components = calloc (IOT_COMPONENT_DELTA, sizeof (iot_component_holder_t));
  cont->ccount = 0;
  cont->csize = IOT_COMPONENT_DELTA;
  pthread_rwlock_init (&cont->lock, NULL);
  return cont;
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

#ifdef IOT_BUILD_DYNAMIC_LOAD

  // pre-pass to find the factory to be added to support dynamic loading of libraries
  while (iot_data_map_iter_next (&iter))
  {
    const char *cname = iot_data_map_iter_string_key(&iter);
    const iot_component_factory_t *factory = NULL;
    void *handle = NULL;

    const char * ctype = iot_data_map_iter_string_value (&iter);
    factory = iot_component_factory_find (ctype);
    if (!factory)
    {
      config = (conf->load) (cname, conf->from);
      if (config)
      {
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
      free (config);
    }
  }
#endif

  while (iot_data_map_iter_next (&iter))
  {
    const char * cname = iot_data_map_iter_string_key (&iter);
    const char * ctype = iot_data_map_iter_string_value (&iter);
    const iot_component_factory_t * factory = iot_component_factory_find (ctype);
    if (factory)
    {
      config = (conf->load) (cname, conf->from);
      if (config)
      {
        iot_component_create_ch (cont, cname, factory, config, true);
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

void iot_component_factory_add (const iot_component_factory_t * factory)
{
  assert (factory);
  pthread_mutex_lock (&iot_component_factories_mutex);
  if (iot_component_factory_find_locked (factory->type) == NULL)
  {
    ((iot_component_factory_t*) factory)->next = iot_component_factories;
    iot_component_factories = factory;
  }
  pthread_mutex_unlock (&iot_component_factories_mutex);
}

extern const iot_component_factory_t * iot_component_factory_find (const char * type)
{
  pthread_mutex_lock (&iot_component_factories_mutex);
  const iot_component_factory_t * factory = iot_component_factory_find_locked (type);
  pthread_mutex_unlock (&iot_component_factories_mutex);
  return factory;
}

void iot_container_add_comp (iot_container_t * cont, const char * ctype, const char *cname, const char * config)
{
  assert (cont && config);

  const iot_component_factory_t *factory = NULL;
  void *handle = NULL;
  iot_data_t *cmap = NULL;

  /* For dynamically loaded components */
  factory = iot_component_factory_find (ctype);
  if (!factory)
  {
    cmap = iot_data_from_json(config);
    /* check for dynamic loading of library */
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
            iot_component_factory_add(factory);
            iot_container_add_handle(cont, handle);
          }
          else
          {
            fprintf(stderr, "ERROR: Invalid configuration, Incorrect Factory name\n");
          }
        }
        else // if !handle
        {
          fprintf(stderr, "ERROR: Incomplete configuration, Factory name not available\n");
          dlclose(handle);
        }
      }
    }
    iot_data_free (cmap);
  }
  /* instantiate the component */
  factory != NULL  ? iot_component_create_ch (cont, cname, factory, config, false) : fprintf (stderr, "ERROR: factory not available, cannot add a component\n");
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

void iot_container_rm_comp (iot_container_t *cont, const char * cname)
{
  uint32_t index = 0;
  iot_component_holder_t * ch = iot_get_component_holder (cont, cname, &index);
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

void iot_container_start_comp (iot_container_t * cont, const char * cname)
{
  iot_component_holder_t * ch = iot_get_component_holder (cont, cname, NULL);
  assert (ch);

  ch->component->start_fn (ch->component);
}

void iot_container_stop_comp (iot_container_t * cont, const char * cname)
{
  iot_component_holder_t * ch = iot_get_component_holder (cont, cname, NULL);
  assert (ch);

  ch->component->stop_fn (ch->component);
}

void iot_container_configure_comp (iot_container_t * cont, const char * cname, const char * config_file)
{
  iot_component_holder_t * ch = iot_get_component_holder (cont, cname, NULL);
  assert (ch);

  iot_data_t * iot_data = iot_data_from_json (config_file);
  ch->factory->reconfig_fn (ch->component, cont, iot_data);
  iot_data_free (iot_data);
}

iot_data_t * iot_container_ls_comp (iot_container_t *cont)
{
  assert (cont);
  iot_data_t * comp_map = iot_data_alloc_map (IOT_DATA_STRING);
  for (uint32_t i = 0; i < cont->ccount; i++)
  {
    iot_data_string_map_add (comp_map, "name", iot_data_alloc_string (cont->components[i]->name, IOT_DATA_REF));
    iot_data_string_map_add (comp_map, "type", iot_data_alloc_string (cont->components[i]->factory->type, IOT_DATA_REF));
    iot_data_string_map_add (comp_map, "state", iot_data_alloc_ui8 (cont->components[i]->component->state));
  }
  return comp_map;
}