//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "mycomponent.h"

/* Configuration loader function */
static char * config_loader (const char * name, const char * uri);

int main (void)
{
  iot_container_config_t config = { .load = config_loader, .uri = NULL, .save = NULL };
  iot_container_t * container = iot_container_alloc ("main");
  iot_component_t * logger;
  iot_data_t * reconfig;
  iot_init ();

  /* Set configuration mechanism */
  iot_container_config (&config);

  /* Add factories for supported component types */
  iot_component_factory_add (iot_logger_factory ());
  iot_component_factory_add (iot_threadpool_factory ());
  iot_component_factory_add (iot_scheduler_factory ());
  iot_component_factory_add (my_component_factory ());

  /* Create components from configuration files */
  iot_container_init (container);

  /* Start everything */
  iot_container_start (container);

  /* list components state */
  iot_component_info_t * info = iot_container_list_components (container);
  iot_component_data_t * data = info->data;
  while (data)
  {
    printf ("Component: %s type: %s state: %s\n", data->name, data->type, iot_component_state_name (data->state));
    data = data->next;
  }
  iot_component_info_free (info);

  sleep (5);

  /* Find instantiated component - the logger */
  logger = iot_container_find_component (container, "logger");

  /* Update logger configuration (what can be reconfigured depends on component) */
  reconfig = iot_data_from_json ("{\"Level\":\"Trace\"}");
  iot_component_reconfig (logger, container, reconfig);
  iot_data_free (reconfig);

  /* Stop everything and clean up */
  iot_container_stop (container);
  iot_container_free (container);
  iot_fini ();

  return 0;
}

/* Example component JSON configurations */

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
  "\"To\":\"file:./container.log\","
  "\"Level\":\"Trace\""
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

static const char * my_config =
"{"
  "\"MyLogger\":\"logger\""
"}";

/* Configuration loader function */

static char * config_loader (const char * name, const char * uri)
{
  (void) uri;
  if (strcmp (name, "main") == 0) return strdup (main_config);
  if (strcmp (name, "file_logger") == 0) return strdup (file_logger_config);
  if (strcmp (name, "logger") == 0) return strdup (logger_config);
  if (strcmp (name, "pool") == 0) return strdup (pool_config);
  if (strcmp (name, "scheduler") == 0) return strdup (sched_config);
  if (strcmp (name, "mycomp") == 0) return strdup (my_config);
  return NULL;
}
