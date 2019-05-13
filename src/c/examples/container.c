//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

/*
 * MQTT commented out, static build is not supported with mqtt
 * */

//#include "svc/export-mqtt.h"
#include "iot/component.h"
#include "iot/container.h"
#include "iot/logger.h"
#include "iot/bus.h"

static const char * config_loader (const char * name);

/* Boilerplate code for a component */

#define MY_COMPONENT_TYPE "MyComponent"

typedef struct my_component_t
{
  iot_component_t component;
  iot_logger_t * logger;
} my_component_t;

static bool my_component_start (my_component_t * comp)
{
  printf ("MyComponent started\n");
  return true;
}

static void my_component_stop (my_component_t * comp)
{
  printf ("MyComponent stopped\n");
}

static my_component_t * my_component_alloc (iot_logger_t * logger)
{
  printf ("MyComponent alloc\n");
  my_component_t * mycomp = calloc (1, sizeof (*mycomp));
  mycomp->component.start_fn = (iot_component_start_fn_t) my_component_start;
  mycomp->component.stop_fn = (iot_component_stop_fn_t) my_component_stop;
  mycomp->logger = logger;
  return mycomp;
}

static void my_component_free (my_component_t * comp)
{
  printf ("MyComponent free\n");
  free (comp);
}

static iot_component_t * my_component_config (iot_container_t * cont, const iot_data_t * map)
{
  printf ("MyComponent config\n");
  const char * name = iot_data_string_map_get_string (map, "MyLogger");
  iot_logger_t * logger = (iot_logger_t*) iot_container_find (cont, name);
  return (iot_component_t*) my_component_alloc (logger);
}

static const iot_component_factory_t * my_component_factory (void)
{
  static iot_component_factory_t factory = { MY_COMPONENT_TYPE, my_component_config, (iot_component_free_fn_t) my_component_free };
  return &factory;
}

int main (void)
{
  iot_container_t * container = iot_container_alloc (config_loader);
  iot_data_init ();
  iot_container_add_factory (container, iot_logger_factory ());
  iot_container_add_factory (container, iot_threadpool_factory ());
  iot_container_add_factory (container, iot_scheduler_factory ());
  iot_container_add_factory (container, iot_bus_factory ());
  //iot_container_add_factory (container, iot_mqtt_factory ());
  iot_container_add_factory (container, my_component_factory ());
  iot_container_init (container, "main");
  iot_container_start (container);
  sleep (2);
  iot_container_stop (container);
  iot_container_free (container);
  iot_data_fini ();
  return 0;
}

static const char * main_config =
"{"
  "\"logger\":\"IOT::Logger\","
  "\"pool\":\"IOT::ThreadPool\","
  "\"scheduler\":\"IOT::Scheduler\","
  "\"bus\":\"IOT::Bus\","
  "\"mqtt\":\"IOT::Mqtt\","
  "\"mycomp\":\"MyComponent\""
"}";

static const char * logger_config =
"{"
  "\"SubSystem\":\"example\""
"}";

static const char * pool_config =
"{"
  "\"Threads\":2,"
  "\"MaxJobs\":10"
"}";

static const char * sched_config =
"{"
  "\"ThreadPool\":\"pool\""
"}";

static const char * bus_config =
"{"
  "\"Interval\":100000,"
  "\"Scheduler\":\"scheduler\","
  "\"ThreadPool\":\"pool\","
  "\"Topics\": [{\"Topic\":\"test/tube\",\"Priority\":10,\"Retain\":true}]"
"}";

/*
static const char * mqtt_ssl_config =
  "{"
  "\"ssl\":\"true\","
  "\"Bus\":\"bus\","
  "\"keep_alive_interval\":100000,"
  "\"message_schematics\":1,"
  "\"persistance_type\":1,"
  "\"time_out\":10000L,"
  "\"address\":\"ssl://localhost:8883\","
  "\"client_id\":\"client1\","
  "\"username\":\"\","
  "\"password\":\"\","
  "\"topic_export_single\":\"test/export_topic\","
  "\"match\":\"test/#\","
  "\"enable_server_cert_auth\":100000,"
  "\"ssl_version\":3,"
  "\"trust_store\":\"/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/cacert.pem\","
  "\"key_store\":\"/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/localhost.pem\","
  "\"private_key_password\":\"\","
  "\"enabled_cipher_suites\":\"ALL\","
  "\"bus\":\"IOT::Bus\","
  "}";
*/
static const char * my_config =
"{"
  "\"MyLogger\":\"logger\""
"}";

/* Configuration resolver function */

static const char * config_loader (const char * name)
{
  if (strcmp (name, "main") == 0) return main_config;
  if (strcmp (name, "logger") == 0) return logger_config;
  if (strcmp (name, "pool") == 0) return pool_config;
  if (strcmp (name, "scheduler") == 0) return sched_config;
  if (strcmp (name, "bus") == 0) return bus_config;
  //if (strcmp (name, "mqtt") == 0) return mqtt_ssl_config;
  if (strcmp (name, "mycomp") == 0) return my_config;
  return NULL;
}