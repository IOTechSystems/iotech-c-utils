#include "iot/component.h"
#include "iot/thread.h"

extern void iot_component_init (iot_component_t * component, iot_component_start_fn_t start, iot_component_stop_fn_t stop)
{
  assert (component && start && stop);
  component->start_fn = start;
  component->stop_fn = stop;
  iot_mutex_init (&component->mutex);
  pthread_cond_init (&component->cond, NULL);
  atomic_store (&component->refs, 1);
}

void iot_component_fini (iot_component_t * component)
{
  pthread_cond_destroy (&component->cond);
  pthread_mutex_destroy (&component->mutex);
}

void iot_component_add_ref (iot_component_t * component)
{
  atomic_fetch_add (&component->refs, 1);
}

bool iot_component_dec_ref (iot_component_t * component)
{
  return (atomic_fetch_add (&component->refs, -1) <= 1);
}

static bool iot_component_set_state (iot_component_t * component, uint32_t state)
{
  assert (component);
  bool valid;
  bool changed = false;
  pthread_mutex_lock (&component->mutex);
  switch (state)
  {
    case IOT_COMPONENT_STOPPED:
    case IOT_COMPONENT_RUNNING: valid = (component->state != IOT_COMPONENT_DELETED); break;
    case IOT_COMPONENT_DELETED: valid = (component->state != IOT_COMPONENT_RUNNING); break;
    default: assert (0);
  }
  if (valid)
  {
    changed = component->state == state;
    component->state = state;
    pthread_cond_broadcast (&component->cond);
  }
  pthread_mutex_unlock (&component->mutex);
  return changed;
}

extern iot_component_state_t iot_component_wait (iot_component_t * component, uint32_t states)
{
  iot_component_state_t state = iot_component_wait_and_lock (component, states);
  pthread_mutex_unlock (&component->mutex);
  return state;
}

extern iot_component_state_t iot_component_wait_and_lock (iot_component_t * component, uint32_t states)
{
  assert (component);
  pthread_mutex_lock (&component->mutex);
  while ((component->state & states) == 0)
  {
    pthread_cond_wait (&component->cond, &component->mutex);
  }
  return component->state;
}

iot_component_state_t iot_component_lock (iot_component_t * component)
{
  pthread_mutex_lock (&component->mutex);
  return component->state;
}

iot_component_state_t iot_component_unlock (iot_component_t * component)
{
  iot_component_state_t state = component->state;
  pthread_mutex_unlock (&component->mutex);
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