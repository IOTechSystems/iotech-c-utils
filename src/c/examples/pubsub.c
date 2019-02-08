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

static iot_data_t * get_config (void)
{
  iot_data_init ();
  iot_data_t * config = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (config, "Interval", iot_data_alloc_ui64 (200000000u));
  iot_data_string_map_add (config, "Threads", iot_data_alloc_ui32 (2u) );
  return config;
}

int main (void)
{
  time_t stamp;
  iot_data_t * config = get_config ();
  iot_coredata_t * cd = iot_coredata_alloc ();
  iot_coredata_init (cd, config);
  iot_data_free (config);
  iot_coredata_sub_alloc (cd, NULL, subscriber_callback, "test/tube");
  iot_coredata_pub_t * pub = iot_coredata_pub_alloc (cd, NULL, publisher_callback, "test/tube");
  iot_coredata_start (cd);
  stamp = time (NULL);
  printf ("Samples: %d\nStart: %s", PUB_ITERS, ctime (&stamp));
  publish (pub, PUB_ITERS);
  stamp = time (NULL);
  printf ("Stop: %s", ctime (&stamp));
  sleep (5);
  iot_coredata_stop (cd);
  iot_coredata_free (cd);
  iot_data_fini ();
}

static void publish (iot_coredata_pub_t * pub, uint32_t iters)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * array = iot_data_alloc_array (ARRAY_SIZE);
  uint32_t index = 0;

  // Create fixed part of sample

  iot_data_array_add (array, index++, iot_data_alloc_i32 (11));
  iot_data_array_add (array, index++, iot_data_alloc_i32 (22));
  iot_data_array_add (array, index, iot_data_alloc_i32 (33));
  iot_data_string_map_add (map, "Coords", array);
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-54", false));

  while (iters--)
  {
    // Update first field for each iteration

    iot_data_string_map_add (map, "#", iot_data_alloc_i32 (PUB_ITERS - iters));

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
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-7", false));
  iot_data_string_map_add (map, "Temp", iot_data_alloc_f32 (27.34));
  return map;
}

#ifndef NDEBUG

static void data_dump (const iot_data_t * data)
{
  switch (iot_data_type (data))
  {
    case IOT_DATA_INT8: printf ("%d", iot_data_i8 (data)); break;
    case IOT_DATA_UINT8: printf ("%u", iot_data_ui8 (data)); break;
    case IOT_DATA_INT16: printf ("%d", iot_data_i16 (data)); break;
    case IOT_DATA_UINT16: printf ("%u", iot_data_ui16 (data)); break;
    case IOT_DATA_INT32: printf ("%d", iot_data_i32 (data)); break;
    case IOT_DATA_UINT32: printf ("%u", iot_data_ui32 (data)); break;
    case IOT_DATA_INT64: printf ("%"PRId64, iot_data_i64 (data)); break;
    case IOT_DATA_UINT64: printf ("%"PRIu64, iot_data_ui64 (data)); break;
    case IOT_DATA_FLOAT32: printf ("%f", iot_data_f32 (data)); break;
    case IOT_DATA_FLOAT64: printf ("%lf", iot_data_f64 (data)); break;
    case IOT_DATA_BOOL: printf ("%s", iot_data_bool (data) ? "true" : "false"); break;
    case IOT_DATA_STRING: printf ("%s", iot_data_string (data)); break;
    case IOT_DATA_BLOB:
    {
      uint32_t size;
      iot_data_blob (data, &size);
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
        key = iot_data_map_iter_key (&iter);
        value = iot_data_map_iter_value (&iter);
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
        index = iot_data_array_iter_index (&iter);
        value = iot_data_array_iter_value (&iter);
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
