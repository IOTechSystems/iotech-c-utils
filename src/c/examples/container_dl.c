//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "mycomponent.h"

/* Configuration loader function */

static char * config_loader (const char * name, void * from);

int main (void)
{
  iot_container_config_t config = { config_loader, NULL };
  iot_container_t * container = iot_container_alloc ("main");

  iot_init ();
  iot_component_factory_add (iot_logger_factory ());
  iot_container_init (container, &config);

  /* Start everything */

  iot_container_start (container);
  sleep (2);

  /* Stop everything and clean up */

  iot_container_stop (container);
  iot_container_free (container);
  iot_fini ();
  return 0;
}

/* Example component JSON configurations */

static const char * main_config =
"{"
  "\"logger\":\"IOT::Logger\","
  "\"mycomp\":\"IOT::MyComponent\""
"}";

static const char * logger_config =
"{"
  "\"Name\":\"console\","
  "\"Level\":\"Info\","
  "\"Start\": true"
"}";

static const char * my_config =
"{"
  "\"Library\":\"libmycomponent.so\","
  "\"Factory\":\"my_component_factory\","
  "\"MyLogger\":\"logger\""
"}";

/* Configuration loader function */

static char * config_loader (const char * name, void * from)
{
  if (strcmp (name, "main") == 0) return strdup (main_config);
  if (strcmp (name, "logger") == 0) return strdup (logger_config);
  if (strcmp (name, "mycomp") == 0) return strdup (my_config);
  return NULL;
}
