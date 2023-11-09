//
// Copyright (c) 2023 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "data-impl.h"
#include <endian.h>

#define IOT_CBOR_BUFF_SIZE 512u
#define IOT_CBOR_BUFF_DOUBLING_LIMIT 4096u
#define IOT_CBOR_BUFF_INCREMENT 1024u

typedef struct iot_cbor_holder_t
{
  uint8_t * data;
  size_t size;
  size_t index;
} iot_cbor_holder_t;

static void iot_cbor_holder_check_size (iot_cbor_holder_t * holder, size_t required)
{
  size_t total = holder->index + required;
  if (holder->size < total)
  {
    size_t inc = holder->size > IOT_CBOR_BUFF_DOUBLING_LIMIT ? IOT_CBOR_BUFF_INCREMENT : holder->size;
    if (holder->size + inc < total) inc = required;
    holder->size += inc;
    holder->data = realloc (holder->data, holder->size);
  }
}

static void iot_data_cbor_write_bytes (iot_cbor_holder_t * holder, const void *data, size_t length)
{
  iot_cbor_holder_check_size (holder, length);
  memcpy (holder->data + holder->index, data, length);
  holder->index += length;
}

static void iot_data_cbor_write_uint (iot_cbor_holder_t * holder, uint64_t value, uint8_t tag)
{
  if (value < 0x18)
  {
    iot_cbor_holder_check_size (holder, 1);
    holder->data[holder->index++] = (uint8_t) (value + tag);
  }
  else if (value <= UINT8_MAX)
  {
    iot_cbor_holder_check_size (holder, 2);
    holder->data[holder->index++] = 0x18 + tag;
    holder->data[holder->index++] = (uint8_t) value;
  }
  else if (value <= UINT16_MAX)
  {
    iot_cbor_holder_check_size (holder, 3);
    holder->data[holder->index++] = 0x19 + tag;
    *(uint16_t *)(holder->data + holder->index) = htobe16 (value);
    holder->index += 2;
  }
  else if (value <= UINT32_MAX)
  {
    iot_cbor_holder_check_size (holder, 5);
    holder->data[holder->index++] = 0x1a + tag;
    *(uint32_t *)(holder->data + holder->index) = htobe32 (value);
    holder->index += 4;
  }
  else
  {
    iot_cbor_holder_check_size (holder, 9);
    holder->data[holder->index++] = 0x1b + tag;
    *(uint64_t *)(holder->data + holder->index) = htobe64 (value);
    holder->index += 8;
  }
}

static void iot_data_cbor_write_int (iot_cbor_holder_t * holder, int64_t value)
{
  iot_data_cbor_write_uint (holder, value < 0 ? -value : value, value < 0 ? 0x20 : 0);
}

static void iot_data_dump_cbor_ptr (iot_cbor_holder_t * holder, const void * ptr, const iot_data_type_t type)
{
  switch (type)
  {
    case IOT_DATA_INT8: iot_data_cbor_write_int (holder, *(const int8_t *) ptr); break;
    case IOT_DATA_UINT8: iot_data_cbor_write_uint (holder, *(const uint8_t *) ptr, 0); break;
    case IOT_DATA_INT16: iot_data_cbor_write_int (holder, *(const int16_t *) ptr); break;
    case IOT_DATA_UINT16: iot_data_cbor_write_uint (holder, *(const uint16_t *) ptr, 0); break;
    case IOT_DATA_INT32: iot_data_cbor_write_int (holder, *(const int32_t *) ptr); break;
    case IOT_DATA_UINT32: iot_data_cbor_write_uint (holder, *(const uint32_t *) ptr, 0); break;
    case IOT_DATA_INT64: iot_data_cbor_write_int (holder, *(const int64_t *) ptr); break;
    case IOT_DATA_UINT64: iot_data_cbor_write_uint (holder, *(const uint64_t *) ptr, 0); break;
    case IOT_DATA_FLOAT32: iot_data_cbor_write_uint (holder, *(const uint32_t *) ptr, 0xe0); break;
    case IOT_DATA_FLOAT64: iot_data_cbor_write_uint (holder, *(const uint64_t *) ptr, 0xe0); break;
    case IOT_DATA_NULL:
      iot_cbor_holder_check_size (holder, 1); holder->data[holder->index++] = 0xf6; break;
    default:
      iot_cbor_holder_check_size (holder, 1); holder->data[holder->index++] = *(const bool *)ptr ? 0xf5 : 0xf4; break;
  }
}

static void iot_data_dump_cbor (iot_cbor_holder_t * holder, const iot_data_t * data)
{
  switch (data->type)
  {
    case IOT_DATA_UINT8:
      iot_data_cbor_write_uint (holder, iot_data_ui8 (data), 0);
      break;
    case IOT_DATA_UINT16:
      iot_data_cbor_write_uint (holder, iot_data_ui16 (data), 0);
      break;
    case IOT_DATA_UINT32:
      iot_data_cbor_write_uint (holder, iot_data_ui32 (data), 0);
      break;
    case IOT_DATA_UINT64:
      iot_data_cbor_write_uint (holder, iot_data_ui64 (data), 0);
      break;
    case IOT_DATA_INT8:
      iot_data_cbor_write_int (holder, iot_data_i8 (data));
      break;
    case IOT_DATA_INT16:
      iot_data_cbor_write_int (holder, iot_data_i16 (data));
      break;
    case IOT_DATA_INT32:
      iot_data_cbor_write_int (holder, iot_data_i32 (data));
      break;
    case IOT_DATA_INT64:
      iot_data_cbor_write_int (holder, iot_data_i64 (data));
      break;
    case IOT_DATA_FLOAT32:
    {
      uint32_t v;
      float f = iot_data_f32 (data);
      memcpy (&v, &f, sizeof (v));
      iot_data_cbor_write_uint (holder, v, 0xe0);
      break;
    }
    case IOT_DATA_FLOAT64:
    {
      uint64_t v;
      double d = iot_data_f64 (data);
      memcpy (&v, &d, sizeof (v));
      iot_data_cbor_write_uint (holder, v, 0xe0);
      break;
    }
    case IOT_DATA_BOOL:
      iot_cbor_holder_check_size (holder, 1);
      holder->data[holder->index++] = iot_data_bool (data) ? 0xf5 : 0xf4;
      break;
    case IOT_DATA_POINTER:
      break;
    case IOT_DATA_STRING:
    {
      const char *str = iot_data_string (data);
      size_t len = strlen (str);
      iot_data_cbor_write_uint (holder, len, 0x60);
      iot_data_cbor_write_bytes (holder, str, len);
      break;
    }
    case IOT_DATA_NULL:
      iot_cbor_holder_check_size (holder, 1);
      holder->data[holder->index++] = 0xf6;
      break;
    case IOT_DATA_BINARY:
      iot_data_cbor_write_uint (holder, iot_data_array_size (data), 0x40);
      iot_data_cbor_write_bytes (holder, iot_data_address (data), iot_data_array_size (data));
      break;
    case IOT_DATA_ARRAY:
    {
      iot_data_array_iter_t iter;
      iot_data_type_t type = iot_data_array_type (data);
      iot_data_cbor_write_uint (holder, iot_data_array_length (data), 0x80);
      iot_data_array_iter (data, &iter);
      while (iot_data_array_iter_next (&iter))
      {
        iot_data_dump_cbor_ptr (holder, iot_data_array_iter_value (&iter), type);
      }
      break;
    }
    case IOT_DATA_VECTOR:
    {
      iot_data_vector_iter_t iter;
      iot_data_cbor_write_uint (holder, iot_data_vector_size (data), 0x80);
      iot_data_vector_iter (data, &iter);
      while (iot_data_vector_iter_next (&iter))
      {
        iot_data_dump_cbor (holder, iot_data_vector_iter_value (&iter));
      }
      break;
    }
    case IOT_DATA_LIST:
    {
      iot_data_list_iter_t iter;
      iot_data_cbor_write_uint (holder, iot_data_list_length (data), 0x80);
      iot_data_list_iter (data, &iter);
      while (iot_data_list_iter_next (&iter))
      {
        iot_data_dump_cbor (holder, iot_data_list_iter_value (&iter));
      }
      break;
    }
    case IOT_DATA_MAP:
    {
      iot_data_map_iter_t iter;
      iot_data_map_iter (data, &iter);
      iot_data_cbor_write_uint (holder, iot_data_map_size (data), 0xA0);
      while (iot_data_map_iter_next (&iter))
      {
        const iot_data_t * key = iot_data_map_iter_key (&iter);
        const iot_data_t * value = iot_data_map_iter_value (&iter);
        iot_data_dump_cbor (holder, key);
        if (value)
        {
          iot_data_dump_cbor (holder, value);
        }
        else
        {
          iot_cbor_holder_check_size (holder, 1);
          holder->data[holder->index++] = 0xf6;  // null
        }
      }
      break;
    }
    case IOT_DATA_MULTI:
    case IOT_DATA_INVALID:
      assert (data->type != IOT_DATA_MULTI);
      assert (data->type != IOT_DATA_INVALID);
      break;
  }
}

iot_data_t * iot_data_to_cbor (const iot_data_t * data)
{
  return iot_data_to_cbor_with_size (data, IOT_CBOR_BUFF_SIZE);
}

iot_data_t * iot_data_to_cbor_with_size (const iot_data_t * data, uint32_t size)
{
  iot_cbor_holder_t holder;
  assert (data && size > 0);
  holder.data = malloc (size);
  holder.size = size;
  holder.index = 0;
  iot_data_dump_cbor (&holder, data);
  if (holder.index <= UINT32_MAX)
  {
    return iot_data_alloc_binary (holder.data, (uint32_t) holder.index, IOT_DATA_TAKE);
  }
  else
  {
    free (holder.data);
    return NULL;
  }
}
