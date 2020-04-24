/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Custom component boilerplate function implementations */

#include "mycomponent.h"

/* Implementation struct, first member must be the base component type iot_component_t */
struct my_component_t
{
  iot_component_t component;
  iot_logger_t * logger;
};

/* Allocation function, takes as arguments all required component attributes */
extern my_component_t * my_component_alloc (iot_logger_t * logger)
{
  iot_log_trace (logger, "my_component_alloc()");
  my_component_t * mycomp = calloc (1, sizeof (*mycomp));
  iot_component_init (&mycomp->component, my_component_factory (), (iot_component_start_fn_t) my_component_start, (iot_component_stop_fn_t) my_component_stop);
  mycomp->logger = logger;
  iot_logger_add_ref (logger);
  return mycomp;
}

/* Increment component reference count. Delegated to base component */
extern void my_component_add_ref (my_component_t * comp)
{
  iot_component_add_ref (&comp->component);
}

/* Component free function,  Only actually free if reference count is zero. */
extern void my_component_free (my_component_t * comp)
{
  if (comp && iot_component_dec_ref (&comp->component))
  {
    iot_log_trace (comp->logger, "my_component_free()");
    iot_logger_free (comp->logger);
    iot_component_fini (&comp->component);
    free (comp);
  }
}

/* Start component and update state */
extern bool my_component_start (my_component_t * comp)
{
  iot_log_trace (comp->logger, "my_component_start()");
  iot_component_set_running (&comp->component);
  return true;
}

/* Start component and update state */
extern void my_component_stop (my_component_t * comp)
{
  iot_log_trace (comp->logger, "my_component_stop()");
  iot_component_set_stopped (&comp->component);
}

/* Component creation and configuration function, called from factory. */
static iot_component_t * my_component_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * logger = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "MyLogger"));
  return (iot_component_t*) my_component_alloc (logger);
}

/* Function to return static component factory. Used by container. */
extern const iot_component_factory_t * my_component_factory (void)
{
  static iot_component_factory_t factory = { MY_COMPONENT_TYPE, my_component_config, (iot_component_free_fn_t) my_component_free, NULL , NULL};
  return &factory;
}
