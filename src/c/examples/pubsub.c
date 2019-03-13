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
#else
#define PUB_ITERS 10000000
#endif

static void publish (iot_coredata_pub_t * pub, uint32_t iters);
static void subscriber_callback (iot_data_t * data, void * self, const char * match);
static iot_data_t * publisher_callback (void * self);

static iot_data_t * get_config (void)
{
  static const char * json =
  "{"
    "\"Interval\": 200000000,"
    "\"Threads\": 4,"
    "\"Topics\": [{ \"Topic\": \"test/tube\", \"Priority\": 10 }, { \"Topic\": \"test/data\", \"Priority\": 20 }]"
  "}";
  iot_data_init ();
  return iot_data_from_json (json);
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
  sleep (3);
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
    iot_coredata_publish (pub, map, true);
  }

  // Finally delete sampleutests/threadpool/threadpool.h

  iot_data_free (map);
}

static void subscriber_callback (iot_data_t * data, void * self, const char * match)
{
#ifndef NDEBUG
  printf ("Subscription (%s): ", match);
  char * json = iot_data_to_json (data, true);
  printf ("%s\n", json);
  free (json);
#endif
}

static iot_data_t * publisher_callback (void * self)
{
  static float f32 = 20.00;

  f32 = (float) (f32 * 1.02);
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-7", false));
  iot_data_string_map_add (map, "Temp", iot_data_alloc_f32 (f32));
  return map;
}