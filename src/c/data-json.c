//
// Copyright (c) 2023-2026 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/json.h"
#include "iot/base64.h"
#include "data-impl.h"
#include <math.h>
#include <errno.h>

#define IOT_JSON_BUFF_SIZE 512u
#define IOT_VAL_BUFF_SIZE 31u

// State for conversion of parsed JSON tokens to data
typedef struct iot_json_ctx_t
{
  const char * json;             // JSON string
  const iot_json_tok_t * next;   // Next token to convert
  const iot_json_tok_t * end;    // End of parsed tokens
  iot_data_t * cache;            // Cache of key strings
  uint32_t depth;                // Current nesting depth
  bool ordered;                  // Whether to record map key ordering
} iot_json_ctx_t;

static iot_data_t * iot_data_value_from_json (iot_json_ctx_t * ctx);

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
    assert (holder->str[holder->size - holder->free - 1] == '\0'); // Terminator at tracked length (strlen would make appends quadratic)
    size_t len = iot_b64_encodesize (inLen) - 1; /* Allow for string terminator */
    char *out;

    if (holder->free < len)
    {
      iot_data_holder_realloc (holder, len);
    }

    out = holder->str + holder->size - holder->free - 1;
    iot_b64_encode (data, inLen, out, holder->free + 1);
    holder->free -= len;
    assert (holder->str[holder->size - holder->free - 1] == '\0'); // Terminator at tracked length (strlen would make appends quadratic)
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
      // Infinity (not representable in JSON) written as a number which overflows to infinity of the same sign when read.
      // NaN (not representable in JSON) written as null
      if (isnan (*(const float*) ptr)) { strncpy (buff, "null", IOT_VAL_BUFF_SIZE); break; }
      (fpclassify (*(const float*) ptr) == FP_INFINITE) ? snprintf (buff, IOT_VAL_BUFF_SIZE, "%s", signbit (*(const float*) ptr) ? "-1e400" : "1e400") : snprintf (buff, IOT_VAL_BUFF_SIZE, "%.8e", *(const float*) ptr); break;
    case IOT_DATA_FLOAT64:
      if (isnan (*(const double*) ptr)) { strncpy (buff, "null", IOT_VAL_BUFF_SIZE); break; }
      (fpclassify (*(const double*) ptr) == FP_INFINITE) ? snprintf (buff, IOT_VAL_BUFF_SIZE, "%s", signbit (*(const double*) ptr) ? "-1e800" : "1e800") : snprintf (buff, IOT_VAL_BUFF_SIZE, "%.16e", *(const double*) ptr); break;
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
        const iot_data_t * value = iot_data_vector_iter_value (&iter);
        if (value) iot_data_dump_json (holder, value); else iot_data_strcat (holder, "null"); // Empty slot
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

// Read 4 hex digits of a \uXXXX escape

static bool iot_json_hex4 (const char ** src, const char * end, uint32_t * value)
{
  if (end - *src < 4) return false;
  *value = 0u;
  for (int i = 0; i < 4; i++)
  {
    char c = *(*src)++;
    uint32_t digit;
    if (c >= '0' && c <= '9') digit = (uint32_t) (c - '0');
    else if (c >= 'a' && c <= 'f') digit = (uint32_t) (c - 'a' + 10);
    else if (c >= 'A' && c <= 'F') digit = (uint32_t) (c - 'A' + 10);
    else return false;
    *value = (*value << 4) | digit;
  }
  return true;
}

// Encode a unicode code point as UTF-8, returning number of bytes

static size_t iot_json_utf8_encode (uint32_t cp, char * dst)
{
  if (cp < 0x80u)
  {
    dst[0] = (char) cp;
    return 1u;
  }
  if (cp < 0x800u)
  {
    dst[0] = (char) (0xc0u | (cp >> 6));
    dst[1] = (char) (0x80u | (cp & 0x3fu));
    return 2u;
  }
  if (cp < 0x10000u)
  {
    dst[0] = (char) (0xe0u | (cp >> 12));
    dst[1] = (char) (0x80u | ((cp >> 6) & 0x3fu));
    dst[2] = (char) (0x80u | (cp & 0x3fu));
    return 3u;
  }
  dst[0] = (char) (0xf0u | (cp >> 18));
  dst[1] = (char) (0x80u | ((cp >> 12) & 0x3fu));
  dst[2] = (char) (0x80u | ((cp >> 6) & 0x3fu));
  dst[3] = (char) (0x80u | (cp & 0x3fu));
  return 4u;
}

// Returns allocated string for token, with escape sequences converted, or NULL if invalid

static char * iot_data_string_from_json_token (const char * json, const iot_json_tok_t * token)
{
  size_t len = (size_t) (token->end - token->start);
  char * str = malloc (len + 1u); // Escape sequences never expand (\uXXXX is at most 4 UTF-8 bytes)
  if (token->type == IOT_JSON_STRING_ESC)
  {
    const char * src = json + token->start;
    const char * end = src + len;
    char * dst = str;
    while (src < end)
    {
      if (*src != '\\')
      {
        *dst++ = *src++;
        continue;
      }
      if (++src == end) goto FAIL;
      char c = *src++;
      switch (c)
      {
        case 'b': *dst++ = '\b'; break;
        case 'f': *dst++ = '\f'; break;
        case 'r': *dst++ = '\r'; break;
        case 'n': *dst++ = '\n'; break;
        case 't': *dst++ = '\t'; break;
        case 'u':
        {
          uint32_t cp;
          if (! iot_json_hex4 (&src, end, &cp)) goto FAIL;
          if (cp >= 0xd800u && cp <= 0xdbffu) // High surrogate, combine with following low surrogate
          {
            uint32_t low;
            const char * next = src + 2;
            if ((end - src >= 6) && (src[0] == '\\') && (src[1] == 'u') && iot_json_hex4 (&next, end, &low) && (low >= 0xdc00u) && (low <= 0xdfffu))
            {
              cp = 0x10000u + ((cp - 0xd800u) << 10) + (low - 0xdc00u);
              src = next;
            }
            else
            {
              cp = 0xfffdu; // Unpaired surrogate, so unicode replacement character
            }
          }
          else if (cp >= 0xdc00u && cp <= 0xdfffu)
          {
            cp = 0xfffdu;
          }
          if (cp == 0u) goto FAIL; // NULL cannot be held in a string
          dst += iot_json_utf8_encode (cp, dst);
          break;
        }
        default: *dst++ = c; break; // Quote, solidus or reverse solidus
      }
    }
    *dst = '\0';
  }
  else
  {
    memcpy (str, json + token->start, len);
    str[len] = '\0';
  }
  return str;

FAIL:
  free (str);
  return NULL;
}

static inline const iot_json_tok_t * iot_json_next_token (iot_json_ctx_t * ctx)
{
  return (ctx->next < ctx->end) ? ctx->next++ : NULL;
}

static iot_data_t * iot_data_string_from_json (iot_json_ctx_t * ctx, const iot_json_tok_t * token)
{
  char * chars = iot_data_string_from_json_token (ctx->json, token);
  if (chars == NULL) return NULL;
  iot_data_t * str = iot_data_alloc_string (chars, IOT_DATA_TAKE);
  const iot_data_t * cached = iot_data_map_get (ctx->cache, str);
  if (cached)
  {
    iot_data_free (str);
    str = (iot_data_t*) cached;
  }
  else
  {
    iot_data_map_add (ctx->cache, str, iot_data_add_ref (str));
  }
  return iot_data_add_ref (str);
}

// Check number matches JSON grammar: -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?

static bool iot_json_number_valid (const char * str)
{
#define IOT_JSON_DIGIT(c) ((c) >= '0' && (c) <= '9')
  if (*str == '-') str++;
  if (*str == '0') str++;
  else if (IOT_JSON_DIGIT (*str)) while (IOT_JSON_DIGIT (*str)) str++;
  else return false;
  if (*str == '.')
  {
    str++;
    if (! IOT_JSON_DIGIT (*str)) return false;
    while (IOT_JSON_DIGIT (*str)) str++;
  }
  if (*str == 'e' || *str == 'E')
  {
    str++;
    if (*str == '+' || *str == '-') str++;
    if (! IOT_JSON_DIGIT (*str)) return false;
    while (IOT_JSON_DIGIT (*str)) str++;
  }
  return (*str == '\0');
#undef IOT_JSON_DIGIT
}

static iot_data_t * iot_data_primitive_from_json (iot_json_ctx_t * ctx, const iot_json_tok_t * token)
{
  iot_data_t * ret = NULL;
  char * str = iot_data_string_from_json_token (ctx->json, token);
  if (strcmp (str, "true") == 0) ret = iot_data_true;
  else if (strcmp (str, "false") == 0) ret = iot_data_false;
  else if (strcmp (str, "null") == 0) ret = iot_data_null;
  else if (strcmp (str, "nan") == 0 || strcmp (str, "-nan") == 0) ret = iot_data_alloc_f64 (NAN); // As written for NaN by iot_data_to_json in earlier versions
  else if (iot_json_number_valid (str))
  {
    // Integers held as int64_t, or uint64_t if too big. Floating point numbers, and integers too big for 64 bits, as double
    if (! strpbrk (str, ".eE"))
    {
      errno = 0;
      if (str[0] == '-')
      {
        long long val = strtoll (str, NULL, 10);
        if (errno == 0) ret = iot_data_alloc_i64 ((int64_t) val);
      }
      else
      {
        unsigned long long val = strtoull (str, NULL, 10);
        if (errno == 0) ret = (val <= INT64_MAX) ? iot_data_alloc_i64 ((int64_t) val) : iot_data_alloc_ui64 ((uint64_t) val);
      }
    }
    if (ret == NULL) ret = iot_data_alloc_f64 (strtod (str, NULL));
  }
  free (str);
  return ret;
}

static iot_data_t * iot_data_map_from_json (iot_json_ctx_t * ctx, const iot_json_tok_t * token)
{
  uint32_t elements = token->size;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * ordering = ctx->ordered ? iot_data_alloc_vector (elements) : NULL;
  uint32_t i = 0;

  while (elements--)
  {
    // Each key must be a string with exactly one value
    const iot_json_tok_t * key_token = iot_json_next_token (ctx);
    if (! key_token || (key_token->type != IOT_JSON_STRING && key_token->type != IOT_JSON_STRING_ESC) || key_token->size != 1u) goto FAIL;
    iot_data_t * key = iot_data_string_from_json (ctx, key_token);
    if (! key) goto FAIL;
    iot_data_t * val = iot_data_value_from_json (ctx);
    if (! val)
    {
      iot_data_free (key);
      goto FAIL;
    }
    if (ordering) iot_data_vector_add (ordering, i++, iot_data_add_ref (key));
    iot_data_map_add (map, key, val);
  }
  if (ordering) iot_data_set_metadata (map, ordering, IOT_DATA_STATIC (&iot_data_order));
  return map;

FAIL:
  iot_data_free (ordering);
  iot_data_free (map);
  return NULL;
}

static iot_data_t * iot_data_vector_from_json (iot_json_ctx_t * ctx, const iot_json_tok_t * token)
{
  uint32_t elements = token->size;
  iot_data_t * vector = iot_data_alloc_vector (elements);

  for (uint32_t index = 0; index < elements; index++)
  {
    iot_data_t * val = iot_data_value_from_json (ctx);
    if (! val)
    {
      iot_data_free (vector);
      return NULL;
    }
    iot_data_vector_add (vector, index, val);
  }
  return vector;
}

// Convert next value, returning NULL if JSON structure invalid

static iot_data_t * iot_data_value_from_json (iot_json_ctx_t * ctx)
{
  iot_data_t * data = NULL;
  const iot_json_tok_t * token = iot_json_next_token (ctx);
  if (! token) return NULL;
  if (token->type == IOT_JSON_OBJECT || token->type == IOT_JSON_ARRAY)
  {
    if (++ctx->depth > IOT_DATA_MAX_DEPTH) return NULL; // Limit nesting so that recursion depth is bounded
    data = (token->type == IOT_JSON_OBJECT) ? iot_data_map_from_json (ctx, token) : iot_data_vector_from_json (ctx, token);
    ctx->depth--;
  }
  else if (token->size == 0u) // Only object keys have child tokens
  {
    data = (token->type == IOT_JSON_PRIMITIVE) ? iot_data_primitive_from_json (ctx, token) : iot_data_string_from_json (ctx, token);
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

    iot_json_init (&parser);
    used = iot_json_parse (&parser, json, (size_t) (ptr - json), tokens, count);
    if (used > 0 && (uint32_t) used <= count)
    {
      iot_json_ctx_t ctx = { .json = json, .next = tokens, .end = tokens + parser.toknext, .depth = 0u, .ordered = ordered };
      ctx.cache = cache ? cache : iot_data_alloc_map (IOT_DATA_STRING);
      data = iot_data_value_from_json (&ctx);
      if (data && ctx.next != ctx.end) // Must be a single JSON value
      {
        iot_data_free (data);
        data = NULL;
      }
      if (cache == NULL) iot_data_free (ctx.cache);
    }
    free (tokens);
  }
  return data ? data : iot_data_null;
}
