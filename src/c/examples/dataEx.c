//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/data.h"

#define ARRAY_SIZE 6

static void data_dump (const iot_data_t * data);

int main (void)
{
  iot_data_t * map = iot_data_map_alloc (IOT_DATA_INT16);
  iot_data_t * map2 = iot_data_map_alloc (IOT_DATA_STRING);
  iot_data_t * array = iot_data_array_alloc (ARRAY_SIZE);
  iot_data_t * key;
  iot_data_t * value;
  uint8_t blob[4] = { 6,7,8,9 };

  for (uint16_t k = 0; k < 10; k++)
  {
    key = iot_data_alloc_i16 (k);
    value = iot_data_alloc_i32 (k * 2);
    iot_data_map_add (map, key, value);
  }
  for (uint32_t i = 0; i < ARRAY_SIZE; i++)
  {
    value = iot_data_alloc_string ("Hello");
    iot_data_array_add (array, i, value);
  }
  key = iot_data_alloc_i16 (22);
  iot_data_map_add (map, key, array);

  key = iot_data_alloc_string ("I8");
  value = iot_data_alloc_i8 (-1);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI8");
  value = iot_data_alloc_ui8 (1);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("I16");
  value = iot_data_alloc_i16 (-22);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI16");
  value = iot_data_alloc_ui16 (22);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("I32");
  value = iot_data_alloc_i32 (-333);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI32");
  value = iot_data_alloc_ui32 (333);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("I64");
  value = iot_data_alloc_i64 (-4444);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI64");
  value = iot_data_alloc_ui64 (4444);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("F32");
  value = iot_data_alloc_f32 (55555.5);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("F64");
  value = iot_data_alloc_f64 (666666.6);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("BL");
  value = iot_data_alloc_bool (true);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("STR");
  value = iot_data_alloc_string ("Hi");
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("BLOB");
  value = iot_data_alloc_blob (blob, sizeof (blob), true);
  iot_data_map_add (map2, key, value);

  key = iot_data_alloc_i16 (23);
  iot_data_map_add (map, key, map2);

  data_dump (map);
  printf ("\n");
  iot_data_free (map);
}

static void data_dump (const iot_data_t * data)
{
  switch (iot_data_type (data))
  {
    case IOT_DATA_INT8: printf ("%d", iot_data_get_i8 (data)); break;
    case IOT_DATA_UINT8: printf ("%u", iot_data_get_ui8 (data)); break;
    case IOT_DATA_INT16: printf ("%d", iot_data_get_i16 (data)); break;
    case IOT_DATA_UINT16: printf ("%u", iot_data_get_ui16 (data)); break;
    case IOT_DATA_INT32: printf ("%d", iot_data_get_i32 (data)); break;
    case IOT_DATA_UINT32: printf ("%u", iot_data_get_ui32 (data)); break;
    case IOT_DATA_INT64: printf ("%"PRId64, iot_data_get_i64 (data)); break;
    case IOT_DATA_UINT64: printf ("%"PRIu64, iot_data_get_ui64 (data)); break;
    case IOT_DATA_FLOAT32: printf ("%f", iot_data_get_f32 (data)); break;
    case IOT_DATA_FLOAT64: printf ("%lf", iot_data_get_f64 (data)); break;
    case IOT_DATA_BOOL: printf ("%s", iot_data_get_bool (data) ? "true" : "false"); break;
    case IOT_DATA_STRING: printf ("%s", iot_data_get_string (data)); break;
    case IOT_DATA_BLOB:
    {
      uint32_t size;
      iot_data_get_blob (data, &size);
      printf ("BLOB(%u)", size); break;
    }
    case IOT_DATA_MAP:
    {
      iot_data_map_iter_t iter;
      iot_data_map_iter (data, &iter);
      const iot_data_t * key;
      const iot_data_t * value;
      printf ("{ ");
      while (iot_data_map_iter_next (&iter))
      {
        key = iot_data_map_iter_get_key (&iter);
        value = iot_data_map_iter_get_value (&iter);
        data_dump (key);
        printf (":");
        data_dump (value);
        printf (" ");
      }
      printf ("}");
      break;
    }
    case IOT_DATA_ARRAY:
    {
      iot_data_array_iter_t iter;
      iot_data_array_iter (data, &iter);
      uint32_t index;
      const iot_data_t * value;
      printf ("[");
      while (iot_data_array_iter_next (&iter))
      {
        index = iot_data_array_iter_get_index (&iter);
        value = iot_data_array_iter_get_value (&iter);
        printf ("%u", index);
        printf ("-");
        data_dump (value);
        printf (" ");
      }
      printf ("]");
      break;
    }
  }
}