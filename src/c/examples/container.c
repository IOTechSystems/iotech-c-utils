//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/iot.h"

/* Configuration loader function */

static char * config_loader (const char * name, void * from);

/* Boilerplate code for a component */

#define MY_COMPONENT_TYPE "MyComponent"

typedef struct my_component_t
{
  iot_component_t component;
  iot_logger_t * logger;
} my_component_t;

/* Basic exported component functions */

extern my_component_t * my_component_alloc (iot_logger_t * logger);
extern void my_component_add_ref (my_component_t * comp);
extern void my_component_free (my_component_t * comp);
extern bool my_component_start (my_component_t * comp);
extern void my_component_stop (my_component_t * comp);
extern const iot_component_factory_t * my_component_factory (void);

int main (void)
{
  iot_container_config_t config = { config_loader, NULL, NULL };
  iot_container_t * container = iot_container_alloc ();
  iot_component_t * logger;
  iot_data_t * data = iot_data_from_json ("{\"Level\":\"Trace\"}");
  iot_init ();
  iot_container_add_factory (container, iot_logger_factory ());
  iot_container_add_factory (container, iot_threadpool_factory ());
  iot_container_add_factory (container, iot_scheduler_factory ());
  iot_container_add_factory (container, my_component_factory ());
  iot_container_init (container, "main", &config);
  iot_container_start (container);
  sleep (2);
  logger = iot_container_find (container, "logger");
  iot_component_reconfig (logger, container, data);
  iot_data_free (data);
  iot_container_stop (container);
  iot_container_free (container);
  iot_fini ();
  return 0;
}

/* Custom component boilerplate function implementations */

extern my_component_t * my_component_alloc (iot_logger_t * logger)
{
  printf ("MyComponent alloc\n");
  my_component_t * mycomp = calloc (1, sizeof (*mycomp));
  iot_component_init (&mycomp->component, my_component_factory (), (iot_component_start_fn_t) my_component_start, (iot_component_stop_fn_t) my_component_stop);
  mycomp->logger = logger;
  iot_logger_add_ref (logger);
  return mycomp;
}

extern void my_component_add_ref (my_component_t * comp)
{
  iot_component_add_ref (&comp->component);
}

extern void my_component_free (my_component_t * comp)
{
  if (comp && iot_component_dec_ref (&comp->component))
  {
    printf ("MyComponent free\n");
    iot_logger_free (comp->logger);
    iot_component_fini (&comp->component);
    free (comp);
  }
}

extern bool my_component_start (my_component_t * comp)
{
  printf ("MyComponent started\n");
  iot_component_set_running (&comp->component);
  return true;
}

extern void my_component_stop (my_component_t * comp)
{
  printf ("MyComponent stopped\n");
  iot_component_set_stopped (&comp->component);
}

static iot_component_t * my_component_config (iot_container_t * cont, const iot_data_t * map)
{
  printf ("MyComponent config\n");
  const char * name = iot_data_string_map_get_string (map, "MyLogger");
  iot_logger_t * logger = (iot_logger_t*) iot_container_find (cont, name);
  return (iot_component_t*) my_component_alloc (logger);
}

extern const iot_component_factory_t * my_component_factory (void)
{
  static iot_component_factory_t factory = { MY_COMPONENT_TYPE, my_component_config, (iot_component_free_fn_t) my_component_free, NULL };
  return &factory;
}

/* Example component JSON configurations */

static const char * main_config =
"{"
  "\"file_logger\":\"IOT::Logger\","
  "\"logger\":\"IOT::Logger\","
  "\"pool\":\"IOT::ThreadPool\","
  "\"scheduler\":\"IOT::Scheduler\","
  "\"mycomp\":\"MyComponent\""
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

static const char * my_config =
"{"
  "\"MyLogger\":\"logger\""
"}";

/* Configuration loader function */

static char * config_loader (const char * name, void * from)
{
  if (strcmp (name, "main") == 0) return (char*) main_config;
  if (strcmp (name, "file_logger") == 0) return (char*) file_logger_config;
  if (strcmp (name, "logger") == 0) return (char*) logger_config;
  if (strcmp (name, "pool") == 0) return (char*) pool_config;
  if (strcmp (name, "scheduler") == 0) return (char*) sched_config;
  if (strcmp (name, "mycomp") == 0) return (char*) my_config;
  return NULL;
}
