//
// Copyright (c) 2023 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "data-impl.h"
#include <math.h>
#include "yxml.h"

#define YXML_PARSER_BUFF_SIZE 4096
#define YXML_BUFF_SIZE 512

static iot_data_t * iot_data_map_from_xml (bool root, yxml_t * x, iot_string_holder_t * holder, const char ** str)
{
  iot_data_t * elem = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * children = NULL;
  iot_data_t * attrs = iot_data_alloc_map (IOT_DATA_STRING);
  const char * elem_name = x->elem;
  bool more = true;
  holder->str[0] = '\0';
  holder->free = holder->size - 1;
  iot_data_string_map_add (elem, "name", iot_data_alloc_string (elem_name, IOT_DATA_COPY));
  iot_data_string_map_add (elem, "attributes", attrs);
  while (more && **str)
  {
    switch (yxml_parse (x, *(*str)++))
    {
      case YXML_ELEMSTART:
      {
        iot_data_t * child = iot_data_map_from_xml (false, x, holder, str);
        if (child)
        {
          if (root)
          {
            iot_data_free (elem);
            elem = child;
            more = false;
          }
          else
          {
            uint32_t size = 0u;
            if (!children)
            {
              children = iot_data_alloc_vector (1u);
              iot_data_string_map_add (elem, "children", children);
            }
            else
            {
              size = iot_data_vector_size (children);
              iot_data_vector_resize (children, size + 1u);
            }
            iot_data_vector_add (children, size, child);
          }
        }
        else
        {
          iot_data_free (elem);
          elem = NULL;
          more = false;
        }
        break;
      }
      case YXML_ELEMEND:
      {
        if (holder->str[0] != '\0')
        {
          iot_data_string_map_add (elem, "content", iot_data_alloc_string (holder->str, IOT_DATA_COPY));
          holder->str[0] = '\0';
          holder->free = holder->size - 1;
        }
        more = false;
        break;
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
        holder->str[0] = '\0';
        holder->free = holder->size - 1;
        break;
      }
      case YXML_EEOF:
      case YXML_EREF:
      case YXML_ECLOSE:
      case YXML_ESTACK:
      case YXML_ESYN:
      {
        // Error
        iot_data_free (elem);
        elem = NULL;
        more = false;
        break;
      }
      default: break;
    }
  }
  return elem;
}

iot_data_t * iot_data_from_xml (const char * xml)
{
  iot_data_t * result;
  yxml_t * x = malloc (sizeof (yxml_t) + YXML_PARSER_BUFF_SIZE);
  iot_string_holder_t holder;
  holder.str = calloc (1, YXML_BUFF_SIZE);
  holder.size = YXML_BUFF_SIZE;
  holder.free = YXML_BUFF_SIZE - 1; // Allowing for string terminator
  yxml_init (x, x+1, YXML_PARSER_BUFF_SIZE);
  result = iot_data_map_from_xml (true, x, &holder, &xml);
  free (x);
  free (holder.str);
  return result;
}
