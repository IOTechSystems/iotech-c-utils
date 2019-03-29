#include "iot/component.h"

extern void iot_component_start (iot_component_t * comp);
extern void iot_component_stop (iot_component_t * comp);

void iot_component_start (iot_component_t * comp)
{
  if (comp->state == IOT_COMPONENT_STOPPED)
  {
    if ((comp->start_fn) (comp))
    {
      comp->state = IOT_COMPONENT_RUNNING;
    }
  }
}

void iot_component_stop (iot_component_t * comp)
{
  if (comp->state == IOT_COMPONENT_RUNNING)
  {
    (comp->stop_fn) (comp);
    comp->state = IOT_COMPONENT_STOPPED;
  }
}