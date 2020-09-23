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

typedef struct iot_component_holder_t
{
  iot_component_t * component;
  char * name;
  const iot_component_factory_t * factory;
  struct iot_component_holder_t * next;
  struct iot_component_holder_t * prev;
} iot_component_holder_t;

struct iot_container_t
{
  iot_logger_t * logger;
  iot_component_holder_t * head;
  iot_component_holder_t * tail;
  iot_container_t * next;
  iot_container_t * prev;
  char * name;
  pthread_rwlock_t lock;
};

typedef struct iot_parsed_holder_t
{
  char * parsed;
  size_t size;
  size_t len;
} iot_parsed_holder_t;

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

#define IOT_MAX_ENV_LEN 64

static void iot_update_parsed (iot_parsed_holder_t * holder, const char * str, size_t len)
{
  holder->len += len;
  if (holder->len > holder->size)
  {
    holder->size = holder->len;
    holder->parsed = realloc (holder->parsed, holder->size);
  }
  memcpy (holder->parsed + holder->len - len, str, len);
}

/* Replace ${VALUE} in configuration string with corresponding environment variable */

static iot_data_t * iot_component_config_to_map (const char * config, iot_logger_t * logger)
{
  iot_data_t * map = NULL;
  iot_parsed_holder_t holder = { .parsed = NULL, .size = 0, .len = 0 };

  if (config)
  {
    const char * start = config;
    const char * end;
    char key [IOT_MAX_ENV_LEN];

    holder.size = strlen (config);
    holder.parsed = malloc (holder.size);

    while (*start)
    {
      if (start[0] == '$' && start[1] == '{') // Look for "${"
      {
        if ((end = strchr (start, '}'))) // Look for "}"
        {
          size_t len = (size_t) ((end - start) - 2);
          strncpy (key, start + 2, len);
          key[len] = '\0';
          const char * env = getenv (key);
          if (env)
          {
            iot_update_parsed (&holder, env, strlen (env));
          }
          else
          {
            iot_log_error (logger, "Unable to resolve environment variable: %s from configuration", key);
            goto fail;
          }
          start = end + 1;
          continue;
        }
      }
      iot_update_parsed (&holder, start, 1u);
      start++;
    }
    iot_update_parsed (&holder, start, 1u);
    map = iot_data_from_json (holder.parsed);
  }

fail:

  free (holder.parsed);
  return map;
}

/*
 * Create a component instance from it's factory with a json configuration.
 *
 * TODO: Add support for environment variable substitution in configuration json
 */

static void iot_component_create (iot_container_t * cont, const char * name, const iot_component_factory_t * factory, const char * config)
{
  iot_component_t * comp = NULL;
  iot_data_t * map = iot_component_config_to_map (config, cont->logger);
  if (map)
  {
    comp = (factory->config_fn) (cont, map);
    iot_data_free (map);
    if (comp)
    {
      iot_component_holder_t *ch = calloc (1, sizeof (*ch));
      ch->component = comp;
      ch->name = strdup (name);
      ch->factory = factory;
      if (cont->head == NULL) // First list element
      {
        cont->head = ch;
        cont->tail = ch;
      }
      else // Add to tail of list
      {
        cont->tail->next = ch;
        ch->prev = cont->tail;
        cont->tail = ch;
      }
    }
  }
  if (comp == NULL) iot_log_warn (cont->logger, "Container: %s Failed to create component: %s", cont->name, name);
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

static iot_component_holder_t * iot_container_find_holder_locked (iot_container_t * cont, const char * name)
{
  assert (cont && name);
  iot_component_holder_t * holder = cont->head;
  while (holder)
  {
    if (strcmp (holder->name, name) == 0)
    {
      break;
    }
    holder = holder->next;
  }
  return holder;
}

#ifdef IOT_BUILD_DYNAMIC_LOAD

static const iot_component_factory_t * iot_container_try_load_component (iot_container_t * cont, const char * config)
{
  const iot_component_factory_t * result = NULL;
  iot_data_t * cmap = iot_component_config_to_map (config, cont->logger);
  if (cmap)
  {
    const char * library = iot_data_string_map_get_string (cmap, "Library");
    const char * factory = iot_data_string_map_get_string (cmap, "Factory");
    if (library && factory)
    {
      void *handle = dlopen (library, RTLD_LAZY);
      if (handle)
      {
        const iot_component_factory_t *(*factory_fn) (void) = dlsym (handle, factory);
        if (factory_fn)
        {
          result = factory_fn ();
          iot_component_factory_add (result);
        }
        else
        {
          iot_log_error (cont->logger, "Invalid configuration, Could not find Factory: %s in Library: %s", factory, library);
          dlclose (handle);
        }
      }
      else
      {
        iot_log_error (cont->logger, "Invalid configuration, Could not dynamically load Library: %s", library);
      }
    }
    iot_data_free (cmap);
  }
  return result;
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
    cont->name = strdup (name);
    cont->logger = iot_logger_default ();
    iot_logger_start (cont->logger);
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

  char * config = (iot_config->load) (cont->name, iot_config->uri);
  iot_data_t * map = iot_component_config_to_map (config, cont->logger);
  free (config);

  if (map)
  {
    const iot_component_factory_t * factory;
    const char * cname;
    const char * ctype;
    iot_data_map_iter_t iter;
    iot_data_map_iter (map, &iter);

#ifdef IOT_BUILD_DYNAMIC_LOAD

    // pre-pass to find the factory to be added to support dynamic loading of libraries
    while (iot_data_map_iter_next (&iter))
    {
      cname = iot_data_map_iter_string_key (&iter);
      ctype = iot_data_map_iter_string_value (&iter);
      config = (iot_config->load) (cname, iot_config->uri);
      factory = iot_component_factory_find (ctype);

      if ((!factory) && (config))
      {
        iot_container_try_load_component (cont, config);
      }
      free (config);
    }
#endif

    while (iot_data_map_iter_next (&iter))
    {
      cname = iot_data_map_iter_string_key (&iter);
      ctype = iot_data_map_iter_string_value (&iter);
      factory = iot_component_factory_find (ctype);
      if (factory)
      {
        config = (iot_config->load) (cname, iot_config->uri);
        if (config)
        {
          iot_component_create (cont, cname, factory, config);
          free (config);
        }
      }
      else
      {
        iot_log_warn (cont->logger, "Failed to find factory for type: %s", ctype);
      }
    }
    iot_data_free (map);
  }
  return (map != NULL);
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
    while (cont->head)
    {
      iot_component_holder_t * holder = cont->head;
      (holder->factory->free_fn) (holder->component);
      free (holder->name);
      cont->head = holder->next;
      free (holder);
    }
    pthread_rwlock_destroy (&cont->lock);
    free (cont->name);
    free (cont);
  }
}

void iot_container_start (iot_container_t * cont)
{
  pthread_rwlock_rdlock (&cont->lock);
  iot_component_holder_t * holder = cont->head;
  while (holder) // Start in declaration order (dependents first)
  {
    (holder->component->start_fn) (holder->component);
    holder = holder->next;
  }
  pthread_rwlock_unlock (&cont->lock);
}

void iot_container_stop (iot_container_t * cont)
{
  pthread_rwlock_rdlock (&cont->lock);
  iot_component_holder_t * holder = cont->tail;
  while (holder) // Stop in reverse of declaration order (dependents last)
  {
    (holder->component->stop_fn) (holder->component);
    holder = holder->prev;
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
  assert (cont && ctype && cname && config);

  const iot_component_factory_t * factory = iot_component_factory_find (ctype);

#ifdef IOT_BUILD_DYNAMIC_LOAD
  if (!factory)
  {
    factory = iot_container_try_load_component (cont, config);
  }
#endif
  if (factory)
  {
    iot_component_create (cont, cname, factory, config);
  }
  else
  {
    iot_log_error (cont->logger, "Could not find or load Factory: %s", ctype);
  }
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
    iot_component_holder_t * holder = iot_container_find_holder_locked (cont, name);
    if (holder) comp = holder->component;
    pthread_rwlock_unlock (&cont->lock);
  }
  return comp;
}

static inline void iot_container_remove_holder_locked (iot_container_t * cont, iot_component_holder_t * holder)
{
  if (holder->next)
  {
    holder->next->prev = holder->prev;
  }
  else
  {
    cont->tail = holder->prev;
  }
  if (holder->prev)
  {
    holder->prev->next = holder->next;
  }
  else
  {
    cont->head = holder->next;
  }
}

void iot_container_delete_component (iot_container_t * cont, const char * name)
{
  assert (cont && name);
  pthread_rwlock_wrlock (&cont->lock);
  iot_component_holder_t * holder = iot_container_find_holder_locked (cont, name);
  if (holder)
  {
    iot_container_remove_holder_locked (cont, holder);
    if (holder->component->state != IOT_COMPONENT_STOPPED)
    {
      holder->component->stop_fn (holder->component);
    }
    holder->factory->free_fn (holder->component);
    free (holder->name);
    free (holder);
  }
  pthread_rwlock_unlock (&cont->lock);
}

iot_component_info_t * iot_container_list_components (iot_container_t * cont)
{
  assert (cont);
  iot_component_info_t * info = calloc (1, sizeof (*info));
  pthread_rwlock_rdlock (&cont->lock);
  iot_component_holder_t * holder = cont->head;
  while (holder)
  {
    iot_component_data_t * data = malloc (sizeof (*data));
    data->name = strdup (holder->name);
    data->type = strdup (holder->factory->type);
    data->state = holder->component->state;
    data->next = info->data;
    info->data = data;
    info->count++;
    holder = holder->next;
  }
  pthread_rwlock_unlock (&cont->lock);
  return info;
}

iot_data_t * iot_container_list_containers ()
{
  iot_container_t * cont;
  uint32_t index = 0;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);

  pthread_mutex_lock (&iot_container_mutex);
  cont = iot_containers;
  while (cont)
  {
    iot_data_t * val = iot_data_alloc_string (cont->name, IOT_DATA_REF);
    iot_data_t * key = iot_data_alloc_ui32 (index++);
    iot_data_map_add (map, key, val);
    cont = cont->next;
  }
  pthread_mutex_unlock (&iot_container_mutex);
  return map;
}
