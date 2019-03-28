#include "iot/component.h"

extern bool iot_component_start (iot_component_t * comp);
extern bool iot_component_stop (iot_component_t * comp);

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

bool iot_component_stop (iot_component_t * comp)
{
  bool ret = true;

  if (comp->state == IOT_COMPONENT_RUNNING)
  {
    if ((ret = (comp->stop_fn) (comp)))
    {
      comp->state = IOT_COMPONENT_STOPPED;
    }
  }
  return ret;
}