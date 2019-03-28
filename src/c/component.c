#include "iot/component.h"

extern bool iot_component_start (iot_component_t * comp);
extern void iot_component_stop (iot_component_t * comp);

bool iot_component_start (iot_component_t * comp)
{
  bool ret = true;

  if (comp->state == IOT_COMPONENT_STOPPED)
  {
    if ((ret = (comp->start_fn) (comp)))
    {
      comp->state = IOT_COMPONENT_RUNNING;
    }
  }
  return ret;
}

void iot_component_stop (iot_component_t * comp)
{
  if (comp->state == IOT_COMPONENT_RUNNING)
  {
    (comp->stop_fn) (comp);
    comp->state = IOT_COMPONENT_STOPPED;
  }
}