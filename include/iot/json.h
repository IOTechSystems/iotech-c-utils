#ifndef _IOT_JSON_H_
#define _IOT_JSON_H_

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum
{
  IOT_JSON_UNDEFINED = 0,
  IOT_JSON_OBJECT = 1,
  IOT_JSON_ARRAY = 2,
  IOT_JSON_STRING = 3,
  IOT_JSON_PRIMITIVE = 4
} iot_json_type_t;

/* Not enough tokens were provided */
#define IOT_JSON_ERROR_NOMEM -1
/* Invalid character inside JSON string */
#define IOT_JSON_ERROR_INVAL -2
/* The string is not a full JSON packet, more bytes expected */
#define IOT_JSON_ERROR_PART -3

/**
 * JSONtoken description.
 * type    type (object, array, string etc.)
 * start   start position in JSON data string
 * end     end position in JSON data string
 * size    size of array/object
 */
typedef struct 
{
  iot_json_type_t type;
  int32_t start;
  int32_t end;
  uint32_t size;
} iot_json_tok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct
{
  uint32_t pos;     /* offset in the JSON string */
  int32_t toknext;    /* next token to allocate */
  int32_t toksuper;   /* superior token node, e.g parent object or array */
} iot_json_parser;

/**
 * Create JSON parser over an array of tokens
 */
void iot_json_init (iot_json_parser * parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens,
 * each describing a single JSON object.
 */
int iot_json_parse (iot_json_parser * parser, const char * json, size_t len, iot_json_tok_t * tokens, uint32_t num_tokens);

#ifdef __cplusplus
}
#endif

#endif /* _IOT_JSON_H_ */
