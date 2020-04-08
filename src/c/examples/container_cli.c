//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "mycomponent.h"

static const char * my_config =
        "{"
        "\"MyLogger\":\"logger\""
        "}";

/* Configuration loader function */
static char * config_loader (const char * name, const char * uri);

int main (void)
{
  iot_container_config_t config = { config_loader, NULL };
  iot_component_t * logger;
  iot_data_t * reconfig;
  iot_data_map_iter_t iter;

  iot_container_t * container = iot_container_alloc ("main");
  iot_init ();

  /* Set configuration mechanism */
  iot_container_config (&config);

  iot_component_factory_add (iot_logger_factory ());
  iot_component_factory_add (iot_threadpool_factory ());
  iot_component_factory_add (iot_scheduler_factory ());
  iot_component_factory_add (my_component_factory ());

  /* Create components from configuration files */
  iot_container_init (container);

  iot_container_add_component (container, MY_COMPONENT_TYPE, "mycomp", my_config);

  iot_container_start (container);

  iot_component_info_t * components = iot_container_ls_component (container);
  for (int index = 0; index < components->count; index++)
  {
    printf ("name: %s\n", components->componentInfo[index]->name);
    printf ("type: %s\n", components->componentInfo[index]->type);
    printf ("state: %d\n", components->componentInfo[index]->state);
    free (components->componentInfo[index]->name);
    free (components->componentInfo[index]->type);
    free (components->componentInfo[index]);
  }
  free (components->componentInfo);
  free (components);

  iot_data_t * containers = iot_container_ls_containers ();
  uint32_t containers_length = iot_data_map_size (containers);
  iot_data_map_iter (containers, &iter);

  while (iot_data_map_iter_next (&iter) && (containers_length--))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    const iot_data_t * value = iot_data_map_iter_value (&iter);

    printf ("key: %s\t  value: %s\n", (char *)iot_data_string (key), (char *)iot_data_string (value));
  }
  iot_data_free (containers);

  sleep (5);

  logger = iot_container_find_component (container, "logger");
  reconfig = iot_data_from_json ("{\"Level\":\"Trace\"}");
  iot_component_reconfig (logger, container, reconfig);
  iot_data_free (reconfig);

  /* Stop everything and clean up */
  iot_container_rm_component (container, "mycomp");

  iot_container_stop (container);
  iot_container_free (container);
  iot_fini ();
  return 0;
}

static const char * main_config =
        "{"
        "\"file_logger\":\"IOT::Logger\","
        "\"logger\":\"IOT::Logger\","
        "\"pool\":\"IOT::ThreadPool\","
        "\"scheduler\":\"IOT::Scheduler\","
        "\"mycomp\":\"IOT::MyComponent\""
        "}";

static const char * file_logger_config =
        "{"
        "\"Name\":\"file\","
        "\"To\":\"file:./log.log\","
        "\"Level\":\"Warn\""
        "}";

static const char * logger_config =
        "{"
        "\"Name\":\"console\","
        "\"Level\":\"Info\","
        "\"Next\":\"file_logger\""
        "}";

static const char * pool_config =
        "{"
        "\"Threads\":2,"
        "\"MaxJobs\":10,"
        "\"ShutdownDelay\":500,"
        "\"Logger\":\"logger\""
        "}";

static const char * sched_config =
        "{"
        "\"ThreadPool\":\"pool\""
        "}";



/* Configuration loader function */

static char * config_loader (const char * name, const char * uri)
{
  if (strcmp (name, "main") == 0) return strdup (main_config);
  if (strcmp (name, "file_logger") == 0) return strdup (file_logger_config);
  if (strcmp (name, "logger") == 0) return strdup (logger_config);
  if (strcmp (name, "pool") == 0) return strdup (pool_config);
  if (strcmp (name, "scheduler") == 0) return strdup (sched_config);
  return NULL;
}
