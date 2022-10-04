/*
 * Copyright (c) 2020-2022
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/component.h"
#include "iot/thread.h"

#ifdef NDEBUG
#define IOT_RET_CHECK(n) n
#else
#define IOT_RET_CHECK(n) assert ((n) == 0)
#endif

void iot_component_init (iot_component_t * component, const iot_component_factory_t * factory, iot_component_start_fn_t start, iot_component_stop_fn_t stop)
{
  assert (component && start && stop);
  component->start_fn = start;
  component->stop_fn = stop;
  component->factory = factory;
  iot_mutex_init (&component->mutex);
  pthread_cond_init (&component->cond, NULL);
  atomic_store (&component->refs, 1u);
}

void iot_component_set_running_callback (iot_component_t * component, iot_component_running_fn_t fn)
{
  assert (component && fn);
  component->running_fn = fn;
}

bool iot_component_reconfig (iot_component_t * component, iot_container_t * cont, const iot_data_t * map)
{
  assert (component && cont && map);
  return (component->factory && component->factory->reconfig_fn) ? (component->factory->reconfig_fn) (component, cont, map) : false;
}

void iot_component_fini (iot_component_t * component)
{
  free (component->name);
  iot_data_free (component->config);
  pthread_cond_destroy (&component->cond);
  pthread_mutex_destroy (&component->mutex);
}

void iot_component_add_ref (iot_component_t * component)
{
  atomic_fetch_add (&component->refs, 1u);
}

bool iot_component_dec_ref (iot_component_t * component)
{
  return (atomic_fetch_sub (&component->refs, 1u) <= 1u);
}

static bool iot_component_set_state (iot_component_t * component, uint32_t state)
{
  assert (component);
  bool valid = false;
  bool changed = false;
  IOT_RET_CHECK (pthread_mutex_lock (&component->mutex));
  switch (state)
  {
    case IOT_COMPONENT_STOPPED:
    case IOT_COMPONENT_RUNNING:
    case IOT_COMPONENT_STARTING: valid = (component->state != IOT_COMPONENT_DELETED); break;
    case IOT_COMPONENT_DELETED: valid = (component->state != IOT_COMPONENT_RUNNING); break;
    default: break;
  }
  if (valid)
  {
    changed = component->state != state;
    component->state = state;
    IOT_RET_CHECK (pthread_cond_broadcast (&component->cond));
  }
  IOT_RET_CHECK (pthread_mutex_unlock (&component->mutex));
  return changed;
}

extern iot_component_state_t iot_component_wait (iot_component_t * component, uint32_t states)
{
  iot_component_state_t state = iot_component_wait_and_lock (component, states);
  IOT_RET_CHECK (pthread_mutex_unlock (&component->mutex));
  return state;
}

extern iot_component_state_t iot_component_wait_and_lock (iot_component_t * component, uint32_t states)
{
  assert (component);
  IOT_RET_CHECK (pthread_mutex_lock (&component->mutex));
  while ((component->state & states) == 0)
  {
    pthread_cond_wait (&component->cond, &component->mutex);
  }
  return component->state;
}

iot_component_state_t iot_component_lock (iot_component_t * component)
{
  IOT_RET_CHECK (pthread_mutex_lock (&component->mutex));
  return component->state;
}

iot_component_state_t iot_component_unlock (iot_component_t * component)
{
  iot_component_state_t state = component->state;
  IOT_RET_CHECK (pthread_mutex_unlock (&component->mutex));
  return state;
}

bool iot_component_set_running (iot_component_t * component)
{
  return iot_component_set_state (component, IOT_COMPONENT_RUNNING);
}

bool iot_component_set_stopped (iot_component_t * component)
{
  return iot_component_set_state (component, IOT_COMPONENT_STOPPED);
}

bool iot_component_set_deleted (iot_component_t * component)
{
  return iot_component_set_state (component, IOT_COMPONENT_DELETED);
}

bool iot_component_set_starting (iot_component_t * component)
{
  return iot_component_set_state (component, IOT_COMPONENT_STARTING);
}

extern iot_data_t * iot_component_read (const iot_component_t * component)
{
  assert (component);
  iot_data_t * data = iot_data_alloc_map (IOT_DATA_STRING);
  pthread_mutex_lock (&component->mutex);
  iot_data_map_add (data, IOT_DATA_STATIC (iot_data_consts.name), iot_data_alloc_string (component->name, IOT_DATA_REF));
  iot_data_map_add (data, IOT_DATA_STATIC (iot_data_consts.type), iot_data_alloc_string (component->factory->type, IOT_DATA_REF));
  iot_data_map_add (data, IOT_DATA_STATIC (iot_data_consts.state), iot_data_alloc_string (iot_component_state_name (component->state), IOT_DATA_REF));
  iot_data_map_add (data, IOT_DATA_STATIC (iot_data_consts.config), iot_data_add_ref (component->config));
  pthread_mutex_unlock (&component->mutex);
  return data;
}

extern const char * iot_component_state_name (iot_component_state_t state)
{
  switch (state)
  {
    case IOT_COMPONENT_INITIAL: return "Initial";
    case IOT_COMPONENT_STOPPED: return "Stopped";
    case IOT_COMPONENT_RUNNING: return "Running";
    case IOT_COMPONENT_DELETED: return "Deleted";
    case IOT_COMPONENT_STARTING: return "Starting";
    default: break;
  }
  return "Unknown";
}