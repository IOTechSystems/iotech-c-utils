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

#include <cbor.h>

static iot_data_t *cbor_to_iot_data (const cbor_item_t *item);

static iot_data_t *cbor_uint_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_is_int (item));
  switch (cbor_int_get_width (item))
  {
    case CBOR_INT_8:
      return iot_data_alloc_ui8 (cbor_get_uint8 (item));
    case CBOR_INT_16:
      return iot_data_alloc_ui16 (cbor_get_uint16 (item));
    case CBOR_INT_32:
      return iot_data_alloc_ui32 (cbor_get_uint32 (item));
    case CBOR_INT_64:
      return iot_data_alloc_ui64 (cbor_get_uint64 (item));
  }
  assert (false);
  return NULL;
}

static iot_data_t *cbor_negint_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_is_int (item));
  switch (cbor_int_get_width (item))
  {
    case CBOR_INT_8:
      return iot_data_alloc_i16 (1 - cbor_get_uint8 (item));
    case CBOR_INT_16:
      return iot_data_alloc_i32 (1 - cbor_get_uint16 (item));
    case CBOR_INT_32:
      return iot_data_alloc_i64 (1 - cbor_get_uint32 (item));
    case CBOR_INT_64:
      return iot_data_alloc_i64 (1 - cbor_get_uint64 (item));
  }
  assert (false);
  return NULL;
}

static iot_data_t *cbor_definite_bytestring_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_bytestring_is_definite (item));
  size_t length = cbor_bytestring_length (item);
  cbor_mutable_data data = cbor_bytestring_handle (item);
  if (!data)
  {
    assert (false);
    return NULL;
  }
  return iot_data_alloc_binary (data, (uint32_t) length, IOT_DATA_COPY);
}

static iot_data_t *cbor_indefinite_bytestring_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_bytestring_is_indefinite (item));
  size_t chunk_count = cbor_bytestring_chunk_count (item);
  char * data = NULL;
  size_t data_size = 0;
  size_t write_offset = 0;
  cbor_item_t **chunks = cbor_bytestring_chunks_handle (item);
  for (size_t i=0; i<chunk_count;i++)
  {
    const cbor_item_t *chunk = chunks[i];
    if (!cbor_isa_bytestring (chunk))
    {
      assert (false);
      goto error;
    }
    size_t chunk_size = cbor_bytestring_length (chunk);
    cbor_mutable_data chunk_data = cbor_bytestring_handle (chunk);
    data_size += chunk_size;
    char *new_data = realloc (data, data_size);
    if (!new_data)
    {
      assert (false);
      goto error;
    }
    data = new_data;
    memcpy (data + write_offset, chunk_data, chunk_size);
    write_offset += chunk_size;
  }
  return iot_data_alloc_binary (data, (uint32_t) data_size, IOT_DATA_TAKE);
error:
  free (data);
  return NULL;
}

static iot_data_t *cbor_bytestring_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_isa_bytestring (item));
  return cbor_bytestring_is_definite (item) ? cbor_definite_bytestring_to_iot_data (item) : cbor_indefinite_bytestring_to_iot_data (item);
}

static iot_data_t *cbor_definite_string_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_string_is_definite (item));
  size_t length = cbor_string_length (item);
  cbor_mutable_data data = cbor_string_handle (item);
  if (!data)
  {
    assert (false);
    return NULL;
  }
  char *str = malloc (length + 1);
  memcpy (str, data, length);
  str[length] = '\0';
  return iot_data_alloc_string (str, IOT_DATA_TAKE);
}

static iot_data_t *cbor_indefinite_string_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_string_is_indefinite (item));
  size_t chunk_count = cbor_string_chunk_count (item);
  if (chunk_count == 0) return NULL;
  char * data = NULL;
  size_t data_size = 1; // For null terminator
  size_t write_offset = 0;
  cbor_item_t **chunks = cbor_string_chunks_handle (item);
  for (size_t i=0; i<chunk_count;i++)
  {
    const cbor_item_t *chunk = chunks[i];
    if (!cbor_isa_string (chunk))
    {
      assert (false);
      goto error;
    }
    size_t chunk_size = cbor_bytestring_length (chunk);
    cbor_mutable_data chunk_data = cbor_bytestring_handle (chunk);
    data_size += chunk_size;
    char *new_data = realloc (data, data_size);
    if (!new_data)
    {
      assert (false);
      goto error;
    }
    data = new_data;
    memcpy (data + write_offset, chunk_data, chunk_size);
    write_offset += chunk_size;
  }
  data[data_size-1] = '\0';
  return iot_data_alloc_string (data, IOT_DATA_TAKE);
error:
  free (data);
  return NULL;
}

static iot_data_t *cbor_string_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_isa_string (item));
  return cbor_string_is_definite (item) ? cbor_definite_string_to_iot_data (item) : cbor_indefinite_string_to_iot_data (item);
}

static iot_data_t *cbor_array_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_isa_array (item));
  size_t size = cbor_array_size (item);
  iot_data_t *iot_array = iot_data_alloc_vector ((uint32_t) size);
  for (size_t i=0; i<size; i++)
  {
    cbor_item_t *element_item = cbor_array_get (item, i);
    assert (element_item);
    iot_data_t *element_data = cbor_to_iot_data (element_item);
    cbor_decref (&element_item);
    if (!element_data)
    {
      assert (false);
      element_data = iot_data_alloc_null();
    }
    iot_data_vector_add (iot_array, i , element_data);
  }
  return iot_array;
}

static iot_data_t *cbor_map_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_isa_map(item));
  size_t size = cbor_map_size (item);
  iot_data_t *iot_map = iot_data_alloc_map (IOT_DATA_STRING);
  for (size_t i=0; i<size; i++)
  {
    struct cbor_pair pair = cbor_map_handle (item)[i];
    iot_data_t *key = cbor_to_iot_data (pair.key);
    if (!key)
    {
      assert (false);
      continue;
    }
    iot_data_t *value = cbor_to_iot_data (pair.value);
    if (!value)
    {
      assert (false);
      continue;
    }
    iot_data_map_add (iot_map, key, value);
  }
  return iot_map;
}

static iot_data_t *cbor_tag_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_isa_tag);
  return cbor_to_iot_data (cbor_tag_item(item));
}

static iot_data_t *cbor_float_ctrl_to_iot_data (const cbor_item_t *item)
{
  assert (cbor_isa_float_ctrl (item));
  if (cbor_float_ctrl_is_ctrl (item))
  {
    if (cbor_is_bool (item)) return iot_data_alloc_bool (cbor_get_bool (item));
    if (cbor_is_null (item)) return iot_data_alloc_null ();
    assert (false);
    return NULL;
  }

  switch (cbor_float_get_width (item))
  {
    case CBOR_FLOAT_16:
      return iot_data_alloc_f32 (cbor_float_get_float2 (item));
    case CBOR_FLOAT_32:
      return iot_data_alloc_f32 (cbor_float_get_float4 (item));
    case CBOR_FLOAT_64:
      return iot_data_alloc_f64 (cbor_float_get_float8 (item));
  }
  assert (false);
  return NULL;
}

static iot_data_t *cbor_to_iot_data (const cbor_item_t *item)
{
  switch (cbor_typeof (item))
  {
    case CBOR_TYPE_UINT:
      return cbor_uint_to_iot_data (item);
    case CBOR_TYPE_NEGINT:
      return cbor_negint_to_iot_data (item);
    case CBOR_TYPE_BYTESTRING:
      return cbor_bytestring_to_iot_data (item);
    case CBOR_TYPE_STRING:
      return cbor_string_to_iot_data (item);
    case CBOR_TYPE_ARRAY:
      return cbor_array_to_iot_data (item);
    case CBOR_TYPE_MAP:
      return cbor_map_to_iot_data (item);
    case CBOR_TYPE_TAG:
      return cbor_tag_to_iot_data (item);
    case CBOR_TYPE_FLOAT_CTRL:
      return cbor_float_ctrl_to_iot_data (item);
  }
  assert (false);
  return NULL;
}

iot_data_t * iot_data_from_cbor (const uint8_t *data, uint32_t size)
{
  iot_data_t *out = NULL;
  struct cbor_load_result result;
  cbor_item_t *item = cbor_load (data, size, &result);
  if (result.error.code != CBOR_ERR_NONE)
  {
    assert (false);
    goto done;
  }
  out = cbor_to_iot_data (item);
done:
  if (item) cbor_decref (&item);
  return out;
}

iot_data_t * iot_data_from_iot_cbor (const iot_data_t *data)
{
  if (!iot_data_is_of_type (data, IOT_DATA_BINARY))
  {
    assert (false);
    return NULL;
  }
  return iot_data_from_cbor (iot_data_address (data),iot_data_array_size (data));
}

