#include "iot/component.h"

extern void iot_component_init (iot_component_t * component, iot_component_start_fn_t start, iot_component_stop_fn_t stop)
{
  assert (component && start && stop);
  component->start_fn = start;
  component->stop_fn = stop;
  atomic_store (&component->refs, 1);
}

bool iot_component_free (iot_component_t * component)
{
  assert (component);
  return (atomic_fetch_add (&component->refs, -1) <= 1);
}

void iot_component_add_ref (iot_component_t * component)
{
  assert (component);
  atomic_fetch_add (&component->refs, 1);
}