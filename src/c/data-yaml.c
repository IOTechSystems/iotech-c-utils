//
// Copyright (c) 2023 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "data-impl.h"
#include <math.h>
#include <errno.h>
#include <yaml.h>

#define IOT_YAML_TAG_PREFIX "tag:yaml.org,2002:"

typedef struct iot_yaml_ctx_t
{
  yaml_parser_t parser;
  iot_data_t * anchors;      // Map of anchor name to anchored node
  iot_data_t ** exception;
} iot_yaml_ctx_t;

static iot_data_t * iot_data_node_from_yaml (iot_yaml_ctx_t * ctx, const yaml_event_t * event, uint32_t depth);

static bool iot_yaml_parse (iot_yaml_ctx_t * ctx, yaml_event_t * event)
{
  if (! yaml_parser_parse (&ctx->parser, event))
  {
    *ctx->exception = iot_data_alloc_string_fmt ("%s at line %zu", ctx->parser.problem ? ctx->parser.problem : "YAML error", ctx->parser.problem_mark.line + 1u);
    return false;
  }
  return true;
}

static bool iot_yaml_digits (const char ** str, int (*is_digit) (int))
{
  const char * start = *str;
  while (is_digit ((unsigned char) **str)) (*str)++;
  return *str != start;
}

static int iot_yaml_is_octal (int c)
{
  return c >= '0' && c <= '7';
}

// Matches the YAML 1.2 core schema float form [-+]?(\.[0-9]+|[0-9]+(\.[0-9]*)?)([eE][-+]?[0-9]+)?

static bool iot_yaml_is_float (const char * str)
{
  if (*str == '-' || *str == '+') str++;
  if (*str == '.')
  {
    str++;
    if (! iot_yaml_digits (&str, isdigit)) return false;
  }
  else
  {
    if (! iot_yaml_digits (&str, isdigit)) return false;
    if (*str == '.')
    {
      str++;
      iot_yaml_digits (&str, isdigit);
    }
  }
  if (*str == 'e' || *str == 'E')
  {
    str++;
    if (*str == '-' || *str == '+') str++;
    if (! iot_yaml_digits (&str, isdigit)) return false;
  }
  return *str == '\0';
}

static bool iot_yaml_match (const char * str, const char * lower, const char * title, const char * upper)
{
  return strcmp (str, lower) == 0 || strcmp (str, title) == 0 || strcmp (str, upper) == 0;
}

// Resolves a plain scalar using the YAML 1.2 core schema. Returns NULL if the value is not null, boolean or numeric.

static iot_data_t * iot_data_typed_from_yaml (const char * val)
{
  if (iot_yaml_match (val, "null", "Null", "NULL") || strcmp (val, "~") == 0) return iot_data_null;
  if (iot_yaml_match (val, "true", "True", "TRUE")) return iot_data_true;
  if (iot_yaml_match (val, "false", "False", "FALSE")) return iot_data_false;

  const char * str = val;
  const char * end;
  int base = 10;
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'o'))
  {
    // Hexadecimal (0x) or octal (0o), always unsigned
    base = (str[1] == 'x') ? 16 : 8;
    str += 2;
    end = str;
    if (! iot_yaml_digits (&end, (base == 16) ? isxdigit : iot_yaml_is_octal) || *end != '\0') return NULL;
  }
  else
  {
    // Decimal, leading zeros do not denote octal
    end = (*str == '-' || *str == '+') ? str + 1 : str;
    if (! iot_yaml_digits (&end, isdigit) || *end != '\0') end = NULL;
  }
  if (end)
  {
    errno = 0;
    if (*str == '-')
    {
      long long ll = strtoll (str, NULL, base);
      if (errno == 0) return iot_data_alloc_i64 (ll);
    }
    else
    {
      unsigned long long ull = strtoull (str, NULL, base);
      if (errno == 0) return (ull <= INT64_MAX) ? iot_data_alloc_i64 ((int64_t) ull) : iot_data_alloc_ui64 (ull);
    }
    if (base != 10) return NULL; // Out of range hexadecimal or octal
  }
  if (iot_yaml_is_float (val)) return iot_data_alloc_f64 (strtod (val, NULL)); // Includes out of range decimal integers
  str = (*val == '-' || *val == '+') ? val + 1 : val;
  if (iot_yaml_match (str, ".inf", ".Inf", ".INF")) return iot_data_alloc_f64 ((*val == '-') ? -INFINITY : INFINITY);
  if (iot_yaml_match (val, ".nan", ".NaN", ".NAN") || strcmp (val, "nan") == 0 || strcmp (val, "-nan") == 0) return iot_data_alloc_f64 (NAN); // nan and -nan as written by iot_data_to_json in earlier versions
  return NULL;
}

static iot_data_t * iot_data_scalar_from_yaml (iot_yaml_ctx_t * ctx, const yaml_event_t * event)
{
  iot_data_t * ret = NULL;
  const char * val = (const char *) event->data.scalar.value;
  const char * tag = (const char *) event->data.scalar.tag;
  if (tag && strncmp (tag, IOT_YAML_TAG_PREFIX, sizeof (IOT_YAML_TAG_PREFIX) - 1u) == 0)
  {
    // Core schema tag, the value must match the tagged type
    tag += sizeof (IOT_YAML_TAG_PREFIX) - 1u;
    if (strcmp (tag, "str") != 0)
    {
      ret = iot_data_typed_from_yaml (val);
      iot_data_type_t type = iot_data_type (ret);
      bool valid;
      if (strcmp (tag, "float") == 0 && (type == IOT_DATA_INT64 || type == IOT_DATA_UINT64))
      {
        double d = (type == IOT_DATA_INT64) ? (double) iot_data_i64 (ret) : (double) iot_data_ui64 (ret);
        iot_data_free (ret);
        ret = iot_data_alloc_f64 (d);
        type = IOT_DATA_FLOAT64;
      }
      if (strcmp (tag, "null") == 0) valid = (type == IOT_DATA_NULL);
      else if (strcmp (tag, "bool") == 0) valid = (type == IOT_DATA_BOOL);
      else if (strcmp (tag, "int") == 0) valid = (type == IOT_DATA_INT64 || type == IOT_DATA_UINT64);
      else if (strcmp (tag, "float") == 0) valid = (type == IOT_DATA_FLOAT64);
      else valid = (ret == NULL); // Other tags, value kept as string
      if (! valid)
      {
        *ctx->exception = iot_data_alloc_string_fmt ("Invalid !!%s value \"%s\" at line %zu", tag, val, event->start_mark.line + 1u);
        iot_data_free (ret);
        return NULL;
      }
    }
  }
  else if (tag == NULL && event->data.scalar.style == YAML_PLAIN_SCALAR_STYLE && *val)
  {
    ret = iot_data_typed_from_yaml (val);
  }
  return ret ? ret : iot_data_alloc_string (val, IOT_DATA_COPY);
}

static iot_data_t * iot_data_vector_from_yaml (iot_yaml_ctx_t * ctx, uint32_t depth)
{
  yaml_event_t event;
  uint32_t size = 0u;
  iot_data_t * vec = iot_data_alloc_vector (4u);
  while (iot_yaml_parse (ctx, &event))
  {
    if (event.type == YAML_SEQUENCE_END_EVENT)
    {
      yaml_event_delete (&event);
      iot_data_vector_resize (vec, size);
      return vec;
    }
    iot_data_t * elem = iot_data_node_from_yaml (ctx, &event, depth);
    yaml_event_delete (&event);
    if (elem == NULL) break;
    if (size == iot_data_vector_size (vec)) iot_data_vector_resize (vec, size * 2u);
    iot_data_vector_add (vec, size++, elem);
  }
  iot_data_free (vec);
  return NULL;
}

// Adds entries from a merge key ("<<") source map that are not already in the map

static void iot_data_map_merge_from_yaml (iot_data_t * map, const iot_data_t * src)
{
  iot_data_map_iter_t iter;
  iot_data_map_iter (src, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    if (iot_data_map_get (map, key) == NULL)
    {
      iot_data_map_add (map, iot_data_add_ref (key), iot_data_add_ref (iot_data_map_iter_value (&iter)));
    }
  }
}

static bool iot_data_is_map_vector (const iot_data_t * data)
{
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (data, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    if (iot_data_type (iot_data_vector_iter_value (&iter)) != IOT_DATA_MAP) return false;
  }
  return true;
}

static iot_data_t * iot_data_map_from_yaml (iot_yaml_ctx_t * ctx, uint32_t depth)
{
  yaml_event_t event;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * merges = NULL;
  while (iot_yaml_parse (ctx, &event))
  {
    if (event.type == YAML_MAPPING_END_EVENT)
    {
      yaml_event_delete (&event);
      if (merges)
      {
        // Explicit keys take precedence over merged keys, then earlier merged maps over later ones
        iot_data_list_iter_t iter;
        iot_data_list_iter (merges, &iter);
        while (iot_data_list_iter_next (&iter))
        {
          const iot_data_t * src = iot_data_list_iter_value (&iter);
          if (iot_data_type (src) == IOT_DATA_MAP)
          {
            iot_data_map_merge_from_yaml (map, src);
          }
          else
          {
            iot_data_vector_iter_t mi;
            iot_data_vector_iter (src, &mi);
            while (iot_data_vector_iter_next (&mi)) iot_data_map_merge_from_yaml (map, iot_data_vector_iter_value (&mi));
          }
        }
        iot_data_free (merges);
      }
      return map;
    }
    iot_data_t * key = NULL;
    bool merge = false;
    size_t line = event.start_mark.line + 1u;
    if (event.type == YAML_SCALAR_EVENT)
    {
      const char * val = (const char *) event.data.scalar.value;
      merge = (event.data.scalar.style == YAML_PLAIN_SCALAR_STYLE && event.data.scalar.tag == NULL && strcmp (val, "<<") == 0);
      key = iot_data_alloc_string (val, IOT_DATA_COPY); // Keys are always strings
      if (event.data.scalar.anchor) iot_data_map_add (ctx->anchors, iot_data_alloc_string ((const char *) event.data.scalar.anchor, IOT_DATA_COPY), iot_data_add_ref (key));
    }
    else if (event.type == YAML_ALIAS_EVENT)
    {
      key = iot_data_node_from_yaml (ctx, &event, depth);
      if (key && iot_data_type (key) != IOT_DATA_STRING)
      {
        iot_data_free (key);
        key = NULL;
        *ctx->exception = iot_data_alloc_string_fmt ("Alias to non string map key at line %zu", line);
      }
    }
    else
    {
      *ctx->exception = iot_data_alloc_string_fmt ("Unsupported %s map key at line %zu", (event.type == YAML_MAPPING_START_EVENT) ? "map" : "sequence", line);
    }
    yaml_event_delete (&event);
    if (key == NULL) break;
    if (! iot_yaml_parse (ctx, &event))
    {
      iot_data_free (key);
      break;
    }
    iot_data_t * elem = iot_data_node_from_yaml (ctx, &event, depth);
    yaml_event_delete (&event);
    if (elem == NULL)
    {
      iot_data_free (key);
      break;
    }
    if (merge)
    {
      iot_data_free (key);
      if (iot_data_type (elem) != IOT_DATA_MAP && (iot_data_type (elem) != IOT_DATA_VECTOR || ! iot_data_is_map_vector (elem)))
      {
        iot_data_free (elem);
        *ctx->exception = iot_data_alloc_string_fmt ("Merge key value not a map or sequence of maps at line %zu", line);
        break;
      }
      if (merges == NULL) merges = iot_data_alloc_list ();
      iot_data_list_tail_push (merges, elem);
    }
    else
    {
      iot_data_map_add (map, key, elem);
    }
  }
  iot_data_free (merges);
  iot_data_free (map);
  return NULL;
}

// Converts the node starting with the given event, returns NULL with the exception set on error

static iot_data_t * iot_data_node_from_yaml (iot_yaml_ctx_t * ctx, const yaml_event_t * event, uint32_t depth)
{
  iot_data_t * ret = NULL;
  const yaml_char_t * anchor = NULL;
  switch (event->type)
  {
    case YAML_SCALAR_EVENT:
      anchor = event->data.scalar.anchor;
      ret = iot_data_scalar_from_yaml (ctx, event);
      break;
    case YAML_ALIAS_EVENT:
    {
      // Aliased nodes are shared, so repeated aliases do not expand memory use
      const iot_data_t * node = iot_data_string_map_get (ctx->anchors, (const char *) event->data.alias.anchor);
      if (node)
      {
        ret = iot_data_add_ref (node);
      }
      else
      {
        *ctx->exception = iot_data_alloc_string_fmt ("Undefined alias \"%s\" at line %zu", event->data.alias.anchor, event->start_mark.line + 1u);
      }
      break;
    }
    case YAML_MAPPING_START_EVENT:
    case YAML_SEQUENCE_START_EVENT:
      if (depth >= IOT_DATA_MAX_DEPTH) // Limit nesting so that recursion depth is bounded
      {
        *ctx->exception = iot_data_alloc_string_fmt ("Maximum nesting depth exceeded at line %zu", event->start_mark.line + 1u);
        break;
      }
      if (event->type == YAML_MAPPING_START_EVENT)
      {
        anchor = event->data.mapping_start.anchor;
        ret = iot_data_map_from_yaml (ctx, depth + 1u);
      }
      else
      {
        anchor = event->data.sequence_start.anchor;
        ret = iot_data_vector_from_yaml (ctx, depth + 1u);
      }
      break;
    default:
      *ctx->exception = iot_data_alloc_string_fmt ("Unexpected YAML event at line %zu", event->start_mark.line + 1u);
      break;
  }
  if (ret && anchor) // Anchor defined once node complete, so a node cannot alias itself
  {
    iot_data_map_add (ctx->anchors, iot_data_alloc_string ((const char *) anchor, IOT_DATA_COPY), iot_data_add_ref (ret));
  }
  return ret;
}

iot_data_t * iot_data_from_yaml (const char * yaml, iot_data_t **exception)
{
  assert (yaml && exception);
  iot_data_t * result = NULL;
  iot_yaml_ctx_t ctx;
  yaml_event_t event;
  bool done = false;
  bool doc_end = false;
  *exception = NULL;
  ctx.exception = exception;
  ctx.anchors = iot_data_alloc_map (IOT_DATA_STRING);
  yaml_parser_initialize (&ctx.parser);
  yaml_parser_set_input_string (&ctx.parser, (const yaml_char_t *) yaml, strlen (yaml));
  while (! done && iot_yaml_parse (&ctx, &event))
  {
    switch (event.type)
    {
      case YAML_STREAM_START_EVENT:
        break;
      case YAML_DOCUMENT_START_EVENT:
        if (doc_end) *exception = iot_data_alloc_string_fmt ("Multiple documents at line %zu", event.start_mark.line + 1u);
        break;
      case YAML_DOCUMENT_END_EVENT:
        doc_end = true;
        break;
      case YAML_STREAM_END_EVENT:
        done = true;
        break;
      default: // Root node, at most one as only one document is allowed
        result = iot_data_node_from_yaml (&ctx, &event, 1u);
        break;
    }
    yaml_event_delete (&event);
    if (*exception) break;
  }
  if (*exception)
  {
    iot_data_free (result);
    result = NULL;
  }
  iot_data_free (ctx.anchors);
  yaml_parser_delete (&ctx.parser);
  return result;
}
