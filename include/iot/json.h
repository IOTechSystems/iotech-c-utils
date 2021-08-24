//
// Copyright (c) 2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_JSON_H_
#define _IOT_JSON_H_

/**
 * @file
 * @brief IOTech JSON API
 */

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Alias for JSON type enumeration
 */
typedef enum
{
  IOT_JSON_UNDEFINED = 0,   /**< Undefined */
  IOT_JSON_OBJECT = 1,      /**< Object */
  IOT_JSON_ARRAY = 2,       /**< Array */
  IOT_JSON_STRING = 3,      /**< String */
  IOT_JSON_STRING_ESC = 4,  /**< String with escape sequences */
  IOT_JSON_PRIMITIVE = 5    /**< Other primitive: number, boolean, or null */
} iot_json_type_t;

/** Not enough tokens were provided */
#define IOT_JSON_ERROR_NOMEM -1
/** Invalid character inside JSON string */
#define IOT_JSON_ERROR_INVAL -2
/** The string is not a full JSON packet, more bytes expected */
#define IOT_JSON_ERROR_PART -3

/**
 * Alias for JSON token structure
 */
typedef struct 
{
  iot_json_type_t type;   /**< type (object, array string etc) */
  int32_t start;          /**< start position in JSON data string */
  int32_t end;            /**< End position in JSON data string */
  uint32_t size;          /**< Size of array / object */
  int32_t parent;         /**< Parent token */
} iot_json_tok_t;

/**
 * Alias for JSON parser structure
 *
 * Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct
{
  uint32_t pos;     /**< Offset in the JSON string */
  int32_t toknext;  /**< Next token to allocate */
  int32_t toksuper; /**< Superior token node, e.g parent object or array */
} iot_json_parser;

/**
 * @brief Create and initialise a JSON parser
 *
 * @param parser Pointer to JSON parser
 */
void iot_json_init (iot_json_parser * parser);

 /**
  * @brief Run JSON parser
  *
  * A JSON data string is parsed into an array of tokens each describing a single JSON object.
  *
  * @param parser     Pointer to JSON parser
  * @param json       Input JSON string
  * @param len        Length of JSON string
  * @param tokens     Pointer to array of tokens
  * @param num_tokens Number of tokens
  * @return           Number of strings parsed
  */
int iot_json_parse (iot_json_parser * parser, const char * json, size_t len, iot_json_tok_t * tokens, uint32_t num_tokens);

#ifdef __cplusplus
}
#endif
#endif
