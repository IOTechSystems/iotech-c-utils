//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include "iot/coredata.h"

#define ARRAY_SIZE 3

#ifndef NDEBUG
#define PUB_ITERS 10
static void data_dump (const iot_data_t * data);
#else
#define PUB_ITERS 10000000
#endif

static void publish (iot_coredata_pub_t * pub, uint32_t iters);
static void subscriber_callback (iot_data_t * data, void * self, const char * match);
static iot_data_t * publisher_callback (void * self);

int main (void)
{
  time_t stamp;
  iot_coredata_t * cd = iot_coredata_alloc ();
  iot_coredata_sub_t * sub = iot_coredata_sub_alloc (cd, NULL, subscriber_callback, "test/tube");
  iot_coredata_pub_t * pub = iot_coredata_pub_alloc (cd, NULL, publisher_callback, "test/tube");

  iot_data_init ();
  iot_coredata_start (cd);
  stamp = time (NULL);
  printf ("Samples: %d\nStart: %s", PUB_ITERS, ctime (&stamp));
  publish (pub, PUB_ITERS);
  stamp = time (NULL);
  printf ("Stop: %s", ctime (&stamp));
  (void) sub;
  sleep (10);
  iot_coredata_stop (cd);
  iot_coredata_free (cd);
  iot_data_fini ();
}

static void publish (iot_coredata_pub_t * pub, uint32_t iters)
{
  iot_data_t * map = iot_data_map_alloc (IOT_DATA_STRING);
  iot_data_t * array = iot_data_array_alloc (ARRAY_SIZE);
  iot_data_t * key;
  iot_data_t * value;
  uint32_t index = 0;

  // Create fixed part of sample

  value = iot_data_alloc_i32 (11);
  iot_data_array_add (array, index++, value);
  value = iot_data_alloc_i32 (22);
  iot_data_array_add (array, index++, value);
  value = iot_data_alloc_i32 (33);
  iot_data_array_add (array, index++, value);
  key = iot_data_alloc_string ("Coords", false);
  iot_data_map_add (map, key, array);
  key = iot_data_alloc_string ("Origin", false);
  value = iot_data_alloc_string ("Sensor-54", false);
  iot_data_map_add (map, key, value);

  while (iters--)
  {
    // Update first field for each iteration

    key = iot_data_alloc_string ("#", false);
    value = iot_data_alloc_i32 (PUB_ITERS - iters);
    iot_data_map_add (map, key, value);

    // Increment map ref count or publish will delete

    iot_data_addref (map);
    iot_coredata_publish (pub, map);
  }

  // Finally delete sample

  iot_data_free (map);
}

static void subscriber_callback (iot_data_t * data, void * self, const char * match)
{
#ifndef NDEBUG
  printf ("Sub (%s): ", match);
  data_dump (data);
  printf ("\n");
#endif
}

static iot_data_t * publisher_callback (void * self)
{
  iot_data_t * map = iot_data_map_alloc (IOT_DATA_STRING);
  iot_data_t * key = iot_data_alloc_string ("Origin", false);
  iot_data_t * value = iot_data_alloc_string ("Sensor-7", false);
  iot_data_map_add (map, key, value);
  key = iot_data_alloc_string ("Temp", false);
  value = iot_data_alloc_f32 (27.34);
  iot_data_map_add (map, key, value);
  return map;
}

#ifndef NDEBUG

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
#endif
