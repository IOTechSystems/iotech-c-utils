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

static char * config_loader (const char * name, void * from);

int main (void)
{
  iot_container_config_t config = { config_loader, NULL };
  iot_container_t * container = iot_container_alloc ();
  iot_init ();

  /* Add factories for supported component types */
  /* 2 ways of adding components :
     - static: add factory before init as shown below, call to add_comp creates a component holder
     - dynamic: Update config to provide library and factory path. call to add_comp will add_factory and create a component holder
       within a container
   */

  iot_component_factory_add (iot_logger_factory ());
  iot_component_factory_add (iot_threadpool_factory ());
  iot_component_factory_add (iot_scheduler_factory ());
  iot_component_factory_add (my_component_factory ());

  /* Create components from configuration files */
  iot_container_init (container, "main", &config);

  iot_container_add_comp (container, MY_COMPONENT_TYPE, "mycomp", my_config);

  iot_container_start_comp (container, "mycomp");
  iot_container_start (container);

  iot_data_t * comp = iot_container_ls_comp (container);
  uint32_t map_length = iot_data_map_size (comp);

  iot_data_map_iter_t iter;
  iot_data_map_iter (comp, &iter);

  while (iot_data_map_iter_next (&iter) && (map_length--))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    const iot_data_t * value = iot_data_map_iter_value (&iter);

    printf ("key: %s\t", (char *)iot_data_string (key));
    switch (iot_data_type (value))
    {
      case IOT_DATA_STRING:
      {
        printf ("value: %s\n", (char *)iot_data_string (value));
        break;
      }

      case IOT_DATA_UINT8:
      {
        printf ("value: %d\n", iot_data_ui8 (value));
        break;
      }

      default: printf ("unsupported\n");
      break;
    }
  }
  iot_data_free (comp);

  sleep (5);

  const char *reconfig = "{\"Level\":\"Trace\"}";
  iot_container_configure_comp (container, "logger", reconfig);

  /* Stop everything and clean up */
  iot_container_stop_comp (container, "mycomp");
  iot_container_rm_comp (container, "mycomp");

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

static char * config_loader (const char * name, void * from)
{
  if (strcmp (name, "main") == 0) return strdup (main_config);
  if (strcmp (name, "file_logger") == 0) return strdup (file_logger_config);
  if (strcmp (name, "logger") == 0) return strdup (logger_config);
  if (strcmp (name, "pool") == 0) return strdup (pool_config);
  if (strcmp (name, "scheduler") == 0) return strdup (sched_config);
//  if (strcmp (name, "mycomp") == 0) return strdup (my_config);
  return NULL;
}
