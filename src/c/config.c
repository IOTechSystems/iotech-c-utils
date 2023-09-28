/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "iot/config.h"
#include "iot/container.h"

typedef struct iot_parsed_holder_t
{
  char * parsed;
  size_t size;
  size_t len;
} iot_parsed_holder_t;

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

static bool iot_config_cast (const iot_data_t * map, const char * key, void * val, iot_data_type_t type, iot_logger_t * logger)
{
  assert (map && key && val);
  const iot_data_t * data = iot_data_string_map_get (map, key);
  bool ret = (data && iot_data_cast (data, type, val));
  if (! ret)
  {
    iot_log_error (logger, "Failed to resolve %s configuration value for: %s", iot_data_type_string (type), key);
  }
  return ret;
}

bool iot_config_i64 (const iot_data_t * map, const char * key, int64_t * val, iot_logger_t * logger)
{
  return iot_config_cast (map, key, val, IOT_DATA_INT64, logger);
}

bool iot_config_ui64 (const iot_data_t * map, const char * key, uint64_t * val, iot_logger_t * logger)
{
  return iot_config_cast (map, key, val, IOT_DATA_UINT64, logger);
}

bool iot_config_i32 (const iot_data_t * map, const char * key, int32_t * val, iot_logger_t * logger)
{
  return iot_config_cast (map, key, val, IOT_DATA_INT32, logger);
}

bool iot_config_ui32 (const iot_data_t * map, const char * key, uint32_t * val, iot_logger_t * logger)
{
  return iot_config_cast (map, key, val, IOT_DATA_UINT32, logger);
}

bool iot_config_bool (const iot_data_t * map, const char * key, bool * val, iot_logger_t * logger)
{
  return iot_config_cast (map, key, val, IOT_DATA_BOOL, logger);
}

bool iot_config_f64 (const iot_data_t * map, const char * key, double * val, iot_logger_t * logger)
{
  return iot_config_cast (map, key, val, IOT_DATA_FLOAT64, logger);
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
  assert (map && key);
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
    if (!comp)
    {
      if (logger == NULL) logger = iot_logger_default ();
      iot_log_error (logger, "Failed to resolve component: %s in container", name);
    }
  }
  return comp;
}

#define IOT_MAX_ENV_LEN 64

static void iot_update_parsed (iot_parsed_holder_t * holder, const char * str, size_t len)
{
  holder->len += len;
  if (holder->len > holder->size)
  {
    holder->size = holder->len;
    holder->parsed = realloc (holder->parsed, holder->size);
  }
  memcpy (holder->parsed + holder->len - len, str, len);
}

char * iot_config_substitute_env (const char *str, iot_logger_t *logger)
{
  char * result = NULL;
  iot_parsed_holder_t holder = {.parsed = NULL, .size = 0, .len = 0};

  if (str)
  {
    const char * start = str;
    const char * end;
    char key[IOT_MAX_ENV_LEN];

    holder.size = strlen (str);
    holder.parsed = malloc (holder.size);

    while (*start)
    {
      if (start[0] == '$' && start[1] == '{') // Look for "${"
      {
        if ((end = strchr (start, '}'))) // Look for "}"
        {
          if (end == start + 2) // Look for error case "${}"
          {
            if (logger == NULL) logger = iot_logger_default ();
            iot_log_error (logger, "${}: bad substitution in config");
            free (holder.parsed);
            goto FAIL;
          }
          size_t len = (size_t) ((end - start) - 2);
          if (len >= IOT_MAX_ENV_LEN)
          {
            if (logger == NULL) logger = iot_logger_default ();
            iot_log_error (logger, "Environment variable name is greater than max length of %d", IOT_MAX_ENV_LEN-1);
            free (holder.parsed);
            goto FAIL;
          }
          strncpy (key, start + 2, len);
          key[len] = '\0';
          const char *env = getenv (key);
          if (env)
          {
            iot_update_parsed (&holder, env, strlen (env));
          }
          else
          {
            if (logger == NULL) logger = iot_logger_default ();
            iot_log_error (logger, "Unable to resolve environment variable: %s", key);
            free (holder.parsed);
            goto FAIL;
          }
          start = end + 1;
          continue;
        }
      }
      iot_update_parsed (&holder, start, 1u);
      start++;
    }
    iot_update_parsed (&holder, start, 1u);
    result = holder.parsed;
  }

FAIL:
  return result;
}
