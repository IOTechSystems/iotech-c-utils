//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"

#define ARRAY_SIZE 6

int main (void)
{
  iot_data_init ();

  iot_data_t * map = iot_data_alloc_map (IOT_DATA_INT16);
  iot_data_t * map2 = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * array = iot_data_alloc_array (ARRAY_SIZE);
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
    value = iot_data_alloc_string ("Hello", IOT_DATA_REF);
    iot_data_array_add (array, i, value);
  }
  key = iot_data_alloc_i16 (22);
  iot_data_map_add (map, key, array);

  key = iot_data_alloc_string ("I8", IOT_DATA_REF);
  value = iot_data_alloc_i8 (-1);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI8", IOT_DATA_REF);
  value = iot_data_alloc_ui8 (1);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("I16", IOT_DATA_REF);
  value = iot_data_alloc_i16 (-22);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI16", IOT_DATA_REF);
  value = iot_data_alloc_ui16 (22);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("I32", IOT_DATA_REF);
  value = iot_data_alloc_i32 (-333);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI32", IOT_DATA_REF);
  value = iot_data_alloc_ui32 (333);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("I64", IOT_DATA_REF);
  value = iot_data_alloc_i64 (-4444);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("UI64", IOT_DATA_REF);
  value = iot_data_alloc_ui64 (4444);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("F32", IOT_DATA_REF);
  value = iot_data_alloc_f32 (55555.5);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("F64", IOT_DATA_REF);
  value = iot_data_alloc_f64 (666666.6);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("BL", IOT_DATA_REF);
  value = iot_data_alloc_bool (true);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("STR", IOT_DATA_REF);
  value = iot_data_alloc_string ("Hi", IOT_DATA_REF);
  iot_data_map_add (map2, key, value);
  key = iot_data_alloc_string ("Mr", IOT_DATA_REF);
  value = iot_data_alloc_blob (blob, sizeof (blob), IOT_DATA_COPY);
  iot_data_map_add (map2, key, value);

  key = iot_data_alloc_i16 (23);
  iot_data_map_add (map, key, map2);

  char * json = iot_data_to_json (map, false);
  printf ("%s\n", json);
  free (json);

  iot_data_free (map);
  iot_data_fini ();
}
