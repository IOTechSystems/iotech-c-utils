//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
// This file incorporates work covered by the following copyright and
// permission notice:
//
//   Copyright (c) 2010 Serge A. Zaitsev SPDX-License-Identifier: MIT
//
#include "iot/json.h"
/**
 * Allocates a fresh unused token from the token pool.
 */
static iot_json_tok_t * iot_json_alloc_token (iot_json_parser *parser, iot_json_tok_t *tokens, uint32_t num_tokens)
{
  iot_json_tok_t * tok = NULL;
  if (parser->toknext < num_tokens)
  {
    tok = &tokens[parser->toknext++];
    tok->start = tok->end = -1;
    tok->size = 0;
  }
  return tok;
}

/**
 * Fills token type and boundaries.
 */
static inline void iot_json_fill_token (iot_json_tok_t * token, iot_json_type_t type, int32_t start, int32_t end)
{
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int iot_json_parse_primitive (iot_json_parser * parser, const char *js, size_t len, iot_json_tok_t * tokens, uint32_t num_tokens)
{
  iot_json_tok_t * token;
  uint32_t start = parser->pos;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    switch (js[parser->pos])
    {
#ifndef JSON_STRICT
      /* In strict mode primitive must be followed by "," or "}" or "]" */
      case ':':
#endif
      case '\t':
      case '\r':
      case '\n':
      case ' ':
      case ',':
      case ']':
      case '}': goto found;
      default: break;
    }
    if (js[parser->pos] < 32 || js[parser->pos] >= 127)
    {
      parser->pos = start;
      return IOT_JSON_ERROR_INVAL;
    }
  }
#ifdef JSON_STRICT
  /* In strict mode primitive must be followed by a comma/object/array */
  parser->pos = start;
  return JSON_ERROR_PART;
#endif

found:
  if (tokens == NULL)
  {
    parser->pos--;
    return 0;
  }
  token = iot_json_alloc_token (parser, tokens, num_tokens);
  if (token == NULL)
  {
    parser->pos = start;
    return IOT_JSON_ERROR_NOMEM;
  }
  iot_json_fill_token (token, IOT_JSON_PRIMITIVE, start, parser->pos);
  parser->pos--;
  return 0;
}

/**
 * Fills next token with JSON string.
 */
static int iot_json_parse_string (iot_json_parser *parser, const char *js, size_t len, iot_json_tok_t *tokens, uint32_t num_tokens)
{
  iot_json_tok_t *token;
  uint32_t start = parser->pos;
  parser->pos++;

  /* Skip starting quote */
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    char c = js[parser->pos];

    /* Quote: end of string */
    if (c == '\"')
    {
      if (tokens == NULL)
      {
        return 0;
      }
      token = iot_json_alloc_token (parser, tokens, num_tokens);
      if (token == NULL)
      {
        parser->pos = start;
        return IOT_JSON_ERROR_NOMEM;
      }
      iot_json_fill_token (token, IOT_JSON_STRING, start + 1, parser->pos);
      return 0;
    }

    /* Backslash: Quoted symbol expected */
    if (c == '\\' && parser->pos + 1 < len)
    {
      int i;
      parser->pos++;
      switch (js[parser->pos])
      {
        /* Allowed escaped symbols */
        case '\"':
        case '/':
        case '\\':
        case 'b':
        case 'f':
        case 'r':
        case 'n' :
        case 't':
          break;
          /* Allows escaped symbol \uXXXX */
        case 'u':
          parser->pos++;
          for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++)
          {
            /* If it isn't a hex character we have an error */
            if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
                  (js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
                  (js[parser->pos] >= 97 && js[parser->pos] <= 102)))
            { /* a-f */
              parser->pos = start;
              return IOT_JSON_ERROR_INVAL;
            }
            parser->pos++;
          }
          parser->pos--;
          break;
          /* Unexpected symbol */
        default:
          parser->pos = start;
          return IOT_JSON_ERROR_INVAL;
      }
    }
  }
  parser->pos = start;
  return IOT_JSON_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int iot_json_parse (iot_json_parser *parser, const char * json, size_t len, iot_json_tok_t *tokens, uint32_t num_tokens)
{
  int r;
  int i;
  iot_json_tok_t *token;
  int32_t count = parser->toknext;

  for (; parser->pos < len && json[parser->pos] != '\0'; parser->pos++)
  {
    char c;
    iot_json_type_t type;

    c = json[parser->pos];
    switch (c)
    {
      case '{':
      case '[':
        count++;
        if (tokens == NULL)
        {
          break;
        }
        token = iot_json_alloc_token (parser, tokens, num_tokens);
        if (token == NULL)
        {
          return IOT_JSON_ERROR_NOMEM;
        }
        if (parser->toksuper != -1)
        {
          tokens[parser->toksuper].size++;
        }
        token->type = (c == '{' ? IOT_JSON_OBJECT : IOT_JSON_ARRAY);
        token->start = parser->pos;
        parser->toksuper = parser->toknext - 1;
        break;
      case '}': case ']':
        if (tokens == NULL)
        {
          break;
        }
        type = (c == '}' ? IOT_JSON_OBJECT : IOT_JSON_ARRAY);

        for (i = parser->toknext - 1; i >= 0; i--)
        {
          token = &tokens[i];
          if (token->start != -1 && token->end == -1)
          {
            if (token->type != type)
            {
              return IOT_JSON_ERROR_INVAL;
            }
            parser->toksuper = -1;
            token->end = parser->pos + 1;
            break;
          }
        }
        /* Error if unmatched closing bracket */
        if (i == -1)
        { return IOT_JSON_ERROR_INVAL; }
        for (; i >= 0; i--)
        {
          token = &tokens[i];
          if (token->start != -1 && token->end == -1)
          {
            parser->toksuper = i;
            break;
          }
        }
        break;
      case '\"':
        r = iot_json_parse_string (parser, json, len, tokens, num_tokens);
        if (r < 0) return r;
        count++;
        if (parser->toksuper != -1 && tokens != NULL)
        {
          tokens[parser->toksuper].size++;
        }
        break;
      case '\t':
      case '\r':
      case '\n':
      case ' ':
        break;
      case ':':
        parser->toksuper = parser->toknext - 1;
        break;
      case ',':
        if (tokens != NULL && parser->toksuper != -1 &&
            tokens[parser->toksuper].type != IOT_JSON_ARRAY &&
            tokens[parser->toksuper].type != IOT_JSON_OBJECT)
        {
          for (i = parser->toknext - 1; i >= 0; i--)
          {
            if (tokens[i].type == IOT_JSON_ARRAY || tokens[i].type == IOT_JSON_OBJECT)
            {
              if (tokens[i].start != -1 && tokens[i].end == -1)
              {
                parser->toksuper = i;
                break;
              }
            }
          }
        }
        break;
#ifdef JSON_STRICT
      /* In strict mode primitives are: numbers and booleans */
      case '-': case '0': case '1' : case '2': case '3' : case '4':
      case '5': case '6': case '7' : case '8': case '9':
      case 't': case 'f': case 'n' :
        /* And they must not be keys of the object */
        if (tokens != NULL && parser->toksuper != -1) {
          jsontok_t *t = &tokens[parser->toksuper];
          if (t->type == JSON_OBJECT ||
              (t->type == JSON_STRING && t->size != 0)) {
            return JSON_ERROR_INVAL;
          }
        }
#else
        /* In non-strict mode every unquoted value is a primitive */
      default:
#endif
        r = iot_json_parse_primitive (parser, json, len, tokens, num_tokens);
        if (r < 0) return r;
        count++;
        if (parser->toksuper != -1 && tokens != NULL)
        {
          tokens[parser->toksuper].size++;
        }
        break;

#ifdef JSON_STRICT
      /* Unexpected char in strict mode */
      default:
        return JSON_ERROR_INVAL;
#endif
    }
  }

  if (tokens != NULL)
  {
    for (i = parser->toknext - 1; i >= 0; i--)
    {
      /* Unmatched opened object or array */
      if (tokens[i].start != -1 && tokens[i].end == -1)
      {
        return IOT_JSON_ERROR_PART;
      }
    }
  }

  return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void iot_json_init (iot_json_parser *parser)
{
  parser->pos = 0;
  parser->toknext = 0;
  parser->toksuper = -1;
}

