/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "iot/config.h"
#include "iot/container.h"

static const iot_data_t * iot_config_get_type (const iot_data_t * map, const char * key, iot_data_type_t type, iot_logger_t * logger)
{
  assert (map && key);
  const iot_data_t * data = iot_data_string_map_get (map, key);
  if ((data == NULL) || (iot_data_type (data) != type))
  {
    if (logger == NULL) logger = iot_logger_default ();
    iot_log_error (logger, "Failed to resolve %s configuration value for: %s", iot_data_type_string (type), key);
    data = NULL;
  }
  return data;
}

bool iot_config_i64 (const iot_data_t * map, const char * key, int64_t * val, iot_logger_t * logger)
{
  assert (val);
  const iot_data_t * data = iot_config_get_type (map, key, IOT_DATA_INT64, logger);
  if (data) *val = iot_data_i64 (data);
  return (data != NULL);
}

bool iot_config_bool (const iot_data_t * map, const char * key, bool * val, iot_logger_t * logger)
{
  assert (val);
  const iot_data_t * data = iot_config_get_type (map, key, IOT_DATA_BOOL, logger);
  if (data) *val = iot_data_bool (data);
  return (data != NULL);
}

bool iot_config_f64 (const iot_data_t * map, const char * key, double * val, iot_logger_t * logger)
{
  assert (val);
  const iot_data_t * data = iot_config_get_type (map, key, IOT_DATA_FLOAT64, logger);
  if (data) *val = iot_data_f64 (data);
  return (data != NULL);
}

const char * iot_config_string (const iot_data_t * map, const char * key, bool alloc, iot_logger_t * logger)
{
  const iot_data_t * data = iot_config_get_type (map, key, IOT_DATA_STRING, logger);
  const char * val = NULL;

  if (data)
  {
    val = iot_data_string (data);
    if (alloc) val = strdup (val);
  }
  return val;
}

extern const char * iot_config_string_default (const iot_data_t * map, const char * key, const char * def, bool alloc)
{
  assert (map && key && def);
  const char * val = iot_data_string_map_get_string (map, key);
  if (val == NULL) val = def;
  if (alloc && val) val = strdup (val);
  return val;
}

extern const iot_data_t * iot_config_map (const iot_data_t * map, const char * key, iot_logger_t * logger)
{
  return iot_config_get_type (map, key, IOT_DATA_MAP, logger);
}

extern const iot_data_t * iot_config_vector (const iot_data_t * map, const char * key, iot_logger_t * logger)
{
  return iot_config_get_type (map, key, IOT_DATA_VECTOR, logger);
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
