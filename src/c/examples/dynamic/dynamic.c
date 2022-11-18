//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "mycomponent.h"

int main (int argc, char ** argv)
{
  /* Need directory path of configuration files as program argument */

  if (argc != 2)
  {
    fprintf (stderr, "Usage: %s <config directory>\n", argv[0]);
    return 1;
  }

  /* Create a config to load component configuration data from files */

  iot_container_config_t config = { .load = iot_store_config_loader, .uri = argv[1], .save = NULL };
  iot_container_t * container = iot_container_alloc ("main");

  iot_container_config (&config);
  iot_component_factory_add (iot_logger_factory ());
  iot_container_init (container);

  /* Start everything */

  iot_container_start (container);
  iot_wait_secs (2u);

  /* Stop everything and clean up */

  iot_container_stop (container);
  iot_container_free (container);
  return 0;
}
