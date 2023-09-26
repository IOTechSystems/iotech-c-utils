//
// Copyright (c) 2023 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/json.h"
#include "iot/base64.h"
#include "data-impl.h"
#include <math.h>

#define IOT_JSON_BUFF_SIZE 512u
#define IOT_VAL_BUFF_SIZE 31u

static iot_data_t * iot_data_value_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered, iot_data_t * cache);

static inline void iot_data_strcat (iot_string_holder_t * holder, const char * add)
{
  iot_data_strcat_escape (holder, add, true);
}

static inline void iot_data_add_quote (iot_string_holder_t * holder)
{
  iot_data_strcat_escape (holder, "\"", false);
}

static void iot_data_base64_encode (iot_string_holder_t * holder, const iot_data_t * array)
{
  uint32_t inLen = iot_data_array_size (array);
  if (inLen)
  {
    const uint8_t *data = iot_data_address (array);
    assert (strlen (holder->str) == (holder->size - holder->free - 1));
    size_t len = iot_b64_encodesize (inLen) - 1; /* Allow for string terminator */
    char *out;

    if (holder->free < len)
    {
      iot_data_holder_realloc (holder, len);
    }

    out = holder->str + holder->size - holder->free - 1;
    iot_b64_encode (data, inLen, out, holder->free + 1);
    holder->free -= len;
    assert (strlen (holder->str) == (holder->size - holder->free - 1));
  }
}

static void iot_data_dump_json_ptr (iot_string_holder_t * holder, const void * ptr, const iot_data_type_t type)
{
  if (holder->free < IOT_VAL_BUFF_SIZE)
  {
    iot_data_holder_realloc (holder, IOT_VAL_BUFF_SIZE);
  }
  char * buff = holder->str + holder->size - holder->free - 1;
  switch (type)
  {
    case IOT_DATA_INT8: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRId8 , *(const int8_t *) ptr); break;
    case IOT_DATA_UINT8: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRIu8, *(const uint8_t *) ptr); break;
    case IOT_DATA_INT16: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRId16, *(const int16_t *) ptr); break;
    case IOT_DATA_UINT16: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRIu16, *(const uint16_t *) ptr); break;
    case IOT_DATA_INT32: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRId32, *(const int32_t *) ptr); break;
    case IOT_DATA_UINT32: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRIu32, *(const uint32_t *) ptr); break;
    case IOT_DATA_INT64: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRId64, *(const int64_t *) ptr); break;
    case IOT_DATA_UINT64: snprintf (buff, IOT_VAL_BUFF_SIZE, "%" PRIu64, *(const uint64_t *) ptr); break;
    case IOT_DATA_FLOAT32:
      (fpclassify (*(const float*) ptr) == FP_INFINITE) ? snprintf (buff, IOT_VAL_BUFF_SIZE, "1e400") : snprintf (buff, IOT_VAL_BUFF_SIZE, "%.8e", *(const float*) ptr); break;
    case IOT_DATA_FLOAT64:
      (fpclassify (*(const double*) ptr) == FP_INFINITE) ? snprintf (buff, IOT_VAL_BUFF_SIZE, "1e800") : snprintf (buff, IOT_VAL_BUFF_SIZE, "%.16e", *(const double*) ptr); break;
    case IOT_DATA_NULL: strncpy (buff, "null", IOT_VAL_BUFF_SIZE); break;
    default: strncpy (buff, (*(const bool*) ptr) ? "true" : "false", IOT_VAL_BUFF_SIZE); break;
  }
  holder->free -= strlen (buff);
}

static void iot_data_dump_json (iot_string_holder_t * holder, const iot_data_t * data)
{
  switch (data->type)
  {
    case IOT_DATA_STRING:
      iot_data_add_quote (holder);
      iot_data_strcat (holder, iot_data_string (data));
      iot_data_add_quote (holder);
      break;
    case IOT_DATA_BINARY:
      iot_data_add_quote (holder);
      iot_data_base64_encode (holder, data);
      iot_data_add_quote (holder);
      break;
    case IOT_DATA_ARRAY:
    {
      iot_data_type_t type = iot_data_array_type (data);
      iot_data_array_iter_t iter;
      iot_data_array_iter (data, &iter);
      iot_data_strcat (holder, "[");
      while (iot_data_array_iter_next (&iter))
      {
        iot_data_dump_json_ptr (holder, iot_data_array_iter_value (&iter), type);
        if (iot_data_array_iter_has_next (&iter)) iot_data_strcat (holder, ",");
      }
      iot_data_strcat (holder, "]");
      break;
    }
    case IOT_DATA_MAP:
    {
      const iot_data_t * ordering = iot_data_get_metadata (data, IOT_DATA_STATIC (&iot_data_order));
      iot_data_map_iter_t iter;
      iot_data_vector_iter_t vec_iter = { 0 };
      bool first = true;
      if (ordering) iot_data_vector_iter (ordering, &vec_iter);
      iot_data_map_iter (data, &iter);
      iot_data_strcat (holder, "{");
      while (ordering ? iot_data_vector_iter_next (&vec_iter) : iot_data_map_iter_next (&iter))
      {
        const iot_data_t * key;
        const iot_data_t * value;
        if (ordering)
        {
          key = iot_data_vector_iter_value (&vec_iter);
          value = iot_data_map_get (data, key);
        }
        else
        {
          key = iot_data_map_iter_key (&iter);
          value = iot_data_map_iter_value (&iter);
        }
        if (value)
        {
          if (! first) iot_data_strcat (holder, ",");
          if (iot_data_type (key) != IOT_DATA_STRING) iot_data_add_quote (holder);
          iot_data_dump_json (holder, key);
          if (iot_data_type (key) != IOT_DATA_STRING) iot_data_add_quote (holder);
          iot_data_strcat (holder, ":");
          iot_data_dump_json (holder, value);
          first = false;
        }
      }
      iot_data_strcat (holder, "}");
      break;
    }
    case IOT_DATA_VECTOR:
    {
      iot_data_vector_iter_t iter;
      iot_data_vector_iter (data, &iter);
      iot_data_strcat (holder, "[");
      while (iot_data_vector_iter_next (&iter))
      {
        iot_data_dump_json (holder, iot_data_vector_iter_value (&iter));
        if (iot_data_vector_iter_has_next (&iter)) iot_data_strcat (holder, ",");
      }
      iot_data_strcat (holder, "]");
      break;
    }
    case IOT_DATA_LIST:
    {
      iot_data_list_iter_t iter;
      iot_data_list_iter (data, &iter);
      iot_data_strcat (holder, "[");
      while (iot_data_list_iter_next (&iter))
      {
        iot_data_dump_json (holder, iot_data_list_iter_value (&iter));
        if (iot_data_list_iter_has_next (&iter)) iot_data_strcat (holder, ",");
      }
      iot_data_strcat (holder, "]");
      break;
    }
    case IOT_DATA_POINTER: break;
    default: iot_data_dump_json_ptr (holder, iot_data_address (data), data->type); break;
  }
}

extern char * iot_data_to_json (const iot_data_t * data)
{
  return iot_data_to_json_with_size (data, IOT_JSON_BUFF_SIZE);
}

extern char * iot_data_to_json_with_size (const iot_data_t * data, uint32_t size)
{
  assert (data && size > 0);
  return iot_data_to_json_with_buffer (data, malloc (size), size);
}

extern char * iot_data_to_json_with_buffer (const iot_data_t * data, char * buff, uint32_t size)
{
  iot_string_holder_t holder;
  assert (data && buff && size > 0);
  holder.str = buff;
  holder.size = size;
  holder.free = size - 1; // Allowing for string terminator
  *buff = 0;
  iot_data_dump_json (&holder, data);
  return holder.str;
}

static char * iot_data_string_from_json_token (const char * json, const iot_json_tok_t * token)
{
  size_t len = (size_t) (token->end - token->start);
  char * str = calloc (1u, len + 1u);
  if (token->type == IOT_JSON_STRING_ESC)
  {
    const char *src = json + token->start;
    char *dst = str;
    while (src < json + token->start + len)
    {
      if (*src == '\\')
      {
        switch (*++src)
        {
          case 'b': *dst++ = '\b'; break;
          case 'f': *dst++ = '\f'; break;
          case 'r': *dst++ = '\r'; break;
          case 'n': *dst++ = '\n'; break;
          case 't': *dst++ = '\t'; break;
          case 'u':
            src += 3;
            if (src + 1 < json + token->start + len)
            {
              *dst = *(src++) == '0' ? 0u : 0x10u; // 0 or 1
              *dst++ |= *src <= '9' ? *src - '0' : tolower (*src) - 'a' + 10;
            }
            break;
          default:
            *dst++ = *src;
        }
        src++;
      }
      else
      {
        *dst++ = *src++;
      }
    }
  }
  else
  {
    memcpy (str, json + token->start, len);
  }
  return str;
}

static iot_data_t * iot_data_string_from_json (iot_json_tok_t ** tokens, const char * json, iot_data_t * cache)
{
  iot_data_t * str = iot_data_alloc_string (iot_data_string_from_json_token (json, *tokens), IOT_DATA_TAKE);
  (*tokens)++;
  const iot_data_t * cached = iot_data_map_get (cache, str);
  if (cached)
  {
    iot_data_free (str);
    str = (iot_data_t*) cached;
  }
  else
  {
    iot_data_map_add (cache, str, iot_data_add_ref (str));
  }
  return iot_data_add_ref (str);
}

static iot_data_t * iot_data_primitive_from_json (iot_json_tok_t ** tokens, const char * json)
{
  iot_data_t * ret = NULL;
  char * str = iot_data_string_from_json_token (json, *tokens);
  (*tokens)++;
  switch (str[0])
  {
    case 't': case 'f': ret = iot_data_alloc_bool (str[0] == 't'); break; // true/false
    case 'n': ret = iot_data_alloc_null (); break; // null
    default: // Handle all floating point numbers as doubles, all integers as int64_t unless to big in which case as uint64_t
    {
      if (strchr (str, '.') || strchr (str, 'e') || strchr (str, 'E'))
      {
        ret = iot_data_alloc_f64 (strtod (str, NULL));
      }
      else if (strchr (str, '-'))
      {
        int64_t i64;
        if (sscanf (str,"%"SCNd64, &i64) == 1) ret = iot_data_alloc_i64 (i64);
      }
      else
      {
        uint64_t ui64;
        if (sscanf (str,"%"SCNu64, &ui64) == 1)
        {
          ret = (ui64 <= INT64_MAX) ? iot_data_alloc_i64 ((int64_t) ui64) : iot_data_alloc_ui64 (ui64);
        }
      }
      break;
    }
  }
  free (str);
  return ret;
}

static iot_data_t * iot_data_map_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered, iot_data_t * cache)
{
  uint32_t elements = (*tokens)->size;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * ordering = ordered ? iot_data_alloc_vector (elements) : NULL;
  uint32_t i = 0;

  (*tokens)++;
  while  (elements--)
  {
    iot_data_t * key = iot_data_string_from_json (tokens, json, cache);
    if (ordered) iot_data_vector_add (ordering, i++, iot_data_add_ref (key));
    iot_data_map_add (map, key, iot_data_value_from_json (tokens, json, ordered, cache));
  }
  if (ordered) iot_data_set_metadata (map, ordering,IOT_DATA_STATIC (&iot_data_order));
  return map;
}

static iot_data_t * iot_data_vector_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered, iot_data_t * cache)
{
  uint32_t elements = (*tokens)->size;
  uint32_t index = 0;
  iot_data_t * vector = iot_data_alloc_vector (elements);

  (*tokens)++;
  while (elements--)
  {
    iot_data_vector_add (vector, index++, iot_data_value_from_json (tokens, json, ordered, cache));
  }
  return vector;
}

static iot_data_t * iot_data_value_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered, iot_data_t * cache)
{
  iot_data_t * data = NULL;
  switch ((*tokens)->type)
  {
    case IOT_JSON_PRIMITIVE: data = iot_data_primitive_from_json (tokens, json); break;
    case IOT_JSON_OBJECT: data = iot_data_map_from_json (tokens, json, ordered, cache); break;
    case IOT_JSON_ARRAY: data = iot_data_vector_from_json (tokens, json, ordered, cache); break;
    default: data = iot_data_string_from_json (tokens, json, cache); break;
  }
  return data;
}

extern iot_data_t * iot_data_from_json (const char * json)
{
  return iot_data_from_json_with_cache (json, false, NULL);
}

extern iot_data_t * iot_data_from_json_with_ordering (const char * json, bool ordered)
{
  return iot_data_from_json_with_cache (json, ordered, NULL);
}

extern iot_data_t * iot_data_from_json_with_cache (const char * json, bool ordered, iot_data_t * cache)
{
  iot_data_t * data = NULL;
  const char * ptr = json;

  assert ((cache == NULL) || iot_data_map_key_is_of_type (cache, IOT_DATA_STRING));

  if (ptr && *ptr)
  {
    iot_json_parser parser;
    int32_t used;
    uint32_t count = 1;

    // Approximate token count
    while (*ptr != '\0')
    {
      switch (*ptr)
      {
        case ',': case '{': count++; break;
        case ':': case '[': count += 2; break;
        default: break;
      }
      ptr++;
    }
    iot_json_tok_t *tokens = calloc (1, sizeof (*tokens) * count);
    iot_json_tok_t *tptr = tokens;

    iot_json_init (&parser);
    used = iot_json_parse (&parser, json, strlen (json), tptr, count);
    if (used && (used <= count))
    {
      iot_data_t * km = cache ? cache : iot_data_alloc_map (IOT_DATA_STRING);
      data = iot_data_value_from_json (&tptr, json, ordered, km);
      if (cache == NULL) iot_data_free (km);
    }
    free (tokens);
  }
  return data ? data : iot_data_alloc_null ();
}
