//
// Copyright (c) 2023-2026 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "data-impl.h"
#include <math.h>
#include "yxml.h"

#define YXML_PARSER_BUFF_SIZE 4096
#define YXML_BUFF_SIZE 512

static inline void iot_data_holder_reset (iot_string_holder_t * holder)
{
  holder->str[0] = '\0';
  holder->free = holder->size - 1;
}

// Converts an element, called after its YXML_ELEMSTART. Returns NULL on error or if the input ends before the element is closed.

static iot_data_t * iot_data_map_from_xml (yxml_t * x, iot_string_holder_t * holder, const char ** str)
{
  iot_data_t * elem = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * children = NULL;
  iot_data_t * attrs = iot_data_alloc_map (IOT_DATA_STRING);
  uint32_t size = 0u;
  iot_data_holder_reset (holder);
  iot_data_string_map_add (elem, "name", iot_data_alloc_string (x->elem, IOT_DATA_COPY));
  iot_data_string_map_add (elem, "attributes", attrs);
  while (**str)
  {
    yxml_ret_t ret = yxml_parse (x, *(*str)++);
    switch (ret)
    {
      case YXML_ELEMSTART:
      {
        // Keep content preceding the child element, as the holder is shared with the child
        char * content = (holder->str[0] != '\0') ? strdup (holder->str) : NULL;
        iot_data_t * child = iot_data_map_from_xml (x, holder, str);
        iot_data_holder_reset (holder);
        if (content)
        {
          iot_data_strcat_escape (holder, content, false);
          free (content);
        }
        if (child == NULL) goto error;
        if (children == NULL)
        {
          children = iot_data_alloc_vector (4u);
        }
        else if (size == iot_data_vector_size (children))
        {
          iot_data_vector_resize (children, size * 2u);
        }
        iot_data_vector_add (children, size++, child);
        break;
      }
      case YXML_ELEMEND:
      {
        if (holder->str[0] != '\0')
        {
          iot_data_string_map_add (elem, "content", iot_data_alloc_string (holder->str, IOT_DATA_COPY));
          iot_data_holder_reset (holder);
        }
        if (children)
        {
          iot_data_vector_resize (children, size);
          iot_data_string_map_add (elem, "children", children);
        }
        return elem;
      }
      case YXML_ATTRVAL:
      case YXML_CONTENT:
      {
        iot_data_strcat_escape (holder, x->data, false);
        break;
      }
      case YXML_ATTREND:
      {
        iot_data_map_add (attrs, iot_data_alloc_string (x->attr, IOT_DATA_COPY), iot_data_alloc_string (holder->str, IOT_DATA_COPY));
        iot_data_holder_reset (holder);
        break;
      }
      default:
      {
        if (ret < 0) goto error;
        break;
      }
    }
  }
error: // Parse error or input ended before element closed
  iot_data_free (children);
  iot_data_free (elem);
  return NULL;
}

iot_data_t * iot_data_from_xml (const char * xml)
{
  assert (xml);
  iot_data_t * result = NULL;
  bool ok = true;
  yxml_t * x = malloc (sizeof (yxml_t) + YXML_PARSER_BUFF_SIZE);
  iot_string_holder_t holder;
  holder.str = calloc (1, YXML_BUFF_SIZE);
  holder.size = YXML_BUFF_SIZE;
  holder.free = YXML_BUFF_SIZE - 1; // Allowing for string terminator
  yxml_init (x, x+1, YXML_PARSER_BUFF_SIZE);
  while (ok && *xml)
  {
    yxml_ret_t ret = yxml_parse (x, *xml++);
    if (ret == YXML_ELEMSTART) // Root element, the parser rejects any further elements
    {
      result = iot_data_map_from_xml (x, &holder, &xml);
      ok = (result != NULL);
    }
    else if (ret < 0)
    {
      ok = false;
    }
  }
  if (ok && yxml_eof (x) != YXML_OK) ok = false; // No root element, or input truncated
  if (! ok)
  {
    iot_data_free (result);
    result = NULL;
  }
  free (x);
  free (holder.str);
  return result;
}
