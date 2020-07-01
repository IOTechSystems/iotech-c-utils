/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "iot/config.h"
#include "iot/container.h"

static void iot_config_error (iot_logger_t * logger, const char * type, const char * key)
{
  if (logger == NULL) logger = iot_logger_default ();
  iot_log_error (logger, "Failed to resolve %s configuration value for: %s", type, key);
}

bool iot_config_i64 (const iot_data_t * map, const char * key, int64_t * val, iot_logger_t * logger)
{
  assert (map && key && val);
  const iot_data_t * data = iot_data_string_map_get (map, key);

  if (data && (iot_data_type (data) == IOT_DATA_INT64))
  {
    *val = iot_data_i64 (data);
    return true;
  }
  iot_config_error (logger, "int64_t", key);
  return false;
}

bool iot_config_bool (const iot_data_t * map, const char * key, bool * val, iot_logger_t * logger)
{
  assert (map && key && val);
  const iot_data_t * data = iot_data_string_map_get (map, key);

  if (data && (iot_data_type (data) == IOT_DATA_BOOL))
  {
    *val = iot_data_bool (data);
    return true;
  }
  iot_config_error (logger, "bool", key);
  return false;
}

bool iot_config_f64 (const iot_data_t * map, const char * key, double * val, iot_logger_t * logger)
{
  assert (map && key && val);
  const iot_data_t * data = iot_data_string_map_get (map, key);

  if (data && (iot_data_type (data) == IOT_DATA_FLOAT64))
  {
    *val = iot_data_f64 (data);
    return true;
  }
  iot_config_error (logger, "double", key);
  return false;
}

const char * iot_config_string (const iot_data_t * map, const char * key, bool alloc, iot_logger_t * logger)
{
  assert (map && key);
  const iot_data_t * data = iot_data_string_map_get (map, key);
  const char * val = NULL;

  if (data && (iot_data_type (data) == IOT_DATA_STRING))
  {
    val = iot_data_string (data);
    if (alloc) val = strdup (val);
  }
  if (! val) iot_config_error (logger, "string", key);
  return val;
}

iot_component_t * iot_config_component (const iot_data_t * map, const char * key, iot_container_t * container, iot_logger_t * logger)
{
  assert (map && key && container);
  iot_component_t * comp = NULL;
  const char * name = iot_config_string (map, key, false, logger);
  if (name)
  {
    comp = iot_container_find_component (container, name);
    if (! comp)
    {
      if (logger == NULL) logger = iot_logger_default ();
      iot_log_error (logger, "Failed to resolve component: %s in container", name);
    }
  }
  return comp;
}
