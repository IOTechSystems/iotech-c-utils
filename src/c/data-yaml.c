//
// Copyright (c) 2023 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "data-impl.h"
#include <math.h>
#include <yaml.h>

static iot_data_t * iot_data_map_from_yaml (yaml_parser_t *parser, iot_data_t **exception);

static iot_data_t * iot_data_string_from_yaml (const yaml_event_t *event)
{
  return iot_data_alloc_string ((const char *)event->data.scalar.value, IOT_DATA_COPY);
}

static iot_data_t * iot_data_value_from_yaml (const yaml_event_t *event)
{
  iot_data_t *ret = NULL;
  const char *val = (const char *)event->data.scalar.value;
  if (event->data.scalar.style == YAML_PLAIN_SCALAR_STYLE)
  {
    if (strcmp (val, "true") == 0)
    {
      ret = iot_data_alloc_bool (true);
    }
    else if (strcmp (val, "false") == 0)
    {
      ret = iot_data_alloc_bool (false);
    }
    else if (strcmp (val, "null") == 0)
    {
      ret = iot_data_alloc_null ();
    }
    else if (*val)
    {
      char *e;
      if (*val == '-')
      {
        long long ll = strtoll (val, &e, 0);
        if (*e == '\0')
        {
          ret = iot_data_alloc_i64 (ll);
        }
      }
      if (ret == NULL)
      {
        unsigned long long ull = strtoull (val, &e, 0);
        if (*e == '\0')
        {
          ret = ull < INT64_MAX ? iot_data_alloc_i64 (ull) : iot_data_alloc_ui64 (ull);
        }
      }
      if (ret == NULL)
      {
        double d = strtod (val, &e);
        if (*e == '\0')
        {
          ret = iot_data_alloc_f64 (d);
        }
      }
    }
  }
  if (ret == NULL)
  {
    ret = iot_data_alloc_string (val, IOT_DATA_COPY);
  }
  return ret;
}

static iot_data_t * iot_data_vector_from_yaml (yaml_parser_t *parser, iot_data_t **exception)
{
  yaml_event_t event;
  bool done = false;
  uint32_t size = 0;
  iot_data_t *elem;
  iot_data_t * vec = iot_data_alloc_vector (size);
  do
  {
    elem = NULL;
    if (!yaml_parser_parse (parser, &event))
    {
      *exception = iot_data_alloc_string_fmt ("%s at line %zu", parser->problem, parser->problem_mark.line);
      break;
    }
    switch (event.type)
    {
      case YAML_SCALAR_EVENT:
        elem = iot_data_value_from_yaml (&event);
        break;
      case YAML_MAPPING_START_EVENT:
        elem = iot_data_map_from_yaml (parser, exception);
        break;
      case YAML_SEQUENCE_START_EVENT:
        elem = iot_data_vector_from_yaml (parser, exception);
        break;
      case YAML_SEQUENCE_END_EVENT:
        done = true;
        break;
      default:
        break;
    }
    if (elem)
    {
      iot_data_vector_resize (vec, size + 1);
      iot_data_vector_add (vec, size++, elem);
    }
    yaml_event_delete (&event);
  } while (!done && *exception == NULL);
  if (*exception)
  {
    iot_data_free (vec);
    return NULL;
  }
  else
  {
    return vec;
  }
}

static iot_data_t * iot_data_map_from_yaml (yaml_parser_t *parser, iot_data_t **exception)
{
  yaml_event_t event;
  bool done = false;
  iot_data_t * name = NULL;
  iot_data_t * elem = NULL;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  do
  {
    if (!yaml_parser_parse (parser, &event))
    {
      *exception = iot_data_alloc_string_fmt ("%s at line %zu", parser->problem, parser->problem_mark.line);
      break;
    }
    switch (event.type)
    {
      case YAML_SCALAR_EVENT:
        if (name)
        {
          elem = iot_data_value_from_yaml (&event);
        }
        else
        {
          name = iot_data_string_from_yaml (&event);
        }
        break;
      case YAML_MAPPING_START_EVENT:
        elem = iot_data_map_from_yaml (parser, exception);
        break;
      case YAML_SEQUENCE_START_EVENT:
        elem = iot_data_vector_from_yaml (parser, exception);
        break;
      case YAML_MAPPING_END_EVENT:
        done = true;
        break;
      default:
        break;
    }
    if (elem)
    {
      if (name)
      {
        iot_data_map_add (map, name, elem);
      }
      else
      {
        *exception = iot_data_alloc_string_fmt ("Unexpected (anonymous) %s in map at line %zu", event.type == YAML_MAPPING_START_EVENT ? "map" : "sequence", parser->mark.line);
        iot_data_free (elem);
      }
      name = NULL;
      elem = NULL;
    }
    yaml_event_delete (&event);
  } while (!done && *exception == NULL);
  iot_data_free (name);
  if (*exception)
  {
    iot_data_free (map);
    return NULL;
  }
  else
  {
    return map;
  }
}

iot_data_t * iot_data_from_yaml (const char * yaml, iot_data_t **exception)
{
  iot_data_t *result = NULL;
  yaml_parser_t parser;
  yaml_event_t event;
  bool done = false;
  *exception = NULL;
  yaml_parser_initialize (&parser);
  yaml_parser_set_input_string (&parser, (const yaml_char_t *)yaml, strlen (yaml));
  do
  {
    if (!yaml_parser_parse (&parser, &event))
    {
      *exception = iot_data_alloc_string_fmt ("%s at line %zu", parser.problem, parser.problem_mark.line);
      break;
    }
    switch (event.type)
    {
      case YAML_SCALAR_EVENT:
        result = iot_data_value_from_yaml (&event);
        done = true;
        break;
      case YAML_MAPPING_START_EVENT:
        result = iot_data_map_from_yaml (&parser, exception);
        done = true;
        break;
      case YAML_SEQUENCE_START_EVENT:
        result = iot_data_vector_from_yaml (&parser, exception);
        done = true;
        break;
      case YAML_STREAM_END_EVENT:
        done = true;
        break;
      default:
        break;
    }
    yaml_event_delete (&event);
  } while (!done);

  yaml_parser_delete (&parser);
  return result;
}
