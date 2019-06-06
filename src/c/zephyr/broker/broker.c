//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/bus.h"

#define DATA_ARRAY_SIZE 3

#ifndef NDEBUG
#define PUB_ITERS 10
#else
#define PUB_ITERS 100000
#endif

static void publish (iot_bus_pub_t * pub, uint32_t iters);
static void subscriber_callback (iot_data_t * data, void * self, const char * match);
static iot_data_t * publisher_callback (void * self);

int main (void)
{
  struct timespec start, stop;
  iot_data_init ();
  int prio_max = sched_get_priority_max (SCHED_FIFO);
  int prio_min = sched_get_priority_min (SCHED_FIFO);
  printf ("\nFIFO priority max: %d min: %d\n", prio_max, prio_min);
  iot_threadpool_t * pool = iot_threadpool_alloc (4, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (pool);
  iot_bus_t * bus = iot_bus_alloc (scheduler, pool, 200000);
  iot_bus_sub_alloc (bus, NULL, subscriber_callback, "test/tube");
  iot_bus_pub_t * pub = iot_bus_pub_alloc (bus, NULL, publisher_callback, "test/tube");
  iot_bus_start (bus);
  clock_gettime (CLOCK_MONOTONIC, &start);
  publish (pub, PUB_ITERS);
  clock_gettime (CLOCK_MONOTONIC, &stop);
  printf ("Published %d samples in %d seconds %d nanoseconds\n", PUB_ITERS, stop.tv_sec - start.tv_sec, stop.tv_nsec - start.tv_nsec);
  iot_bus_stop (bus);
  iot_bus_free (bus);
  iot_data_fini ();
  printf ("Done\n");
  fflush (stdout);
  return 0;
}

static void publish (iot_bus_pub_t * pub, uint32_t iters)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * array = iot_data_alloc_array (DATA_ARRAY_SIZE);
  uint32_t index = 0;

  // Create fixed part of sample

  iot_data_array_add (array, index++, iot_data_alloc_i32 (11));
  iot_data_array_add (array, index++, iot_data_alloc_i32 (22));
  iot_data_array_add (array, index, iot_data_alloc_i32 (33));
  iot_data_string_map_add (map, "Coords", array);
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-54", IOT_DATA_REF));

  while (iters--)
  {
    // Update first field for each iteration

    iot_data_string_map_add (map, "#", iot_data_alloc_i32 (PUB_ITERS - iters));

    // Increment map ref count or publish will delete

    iot_data_addref (map);
    iot_bus_pub_push (pub, map, true);
  }

  // Finally delete sample

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
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-7", IOT_DATA_REF));
  iot_data_string_map_add (map, "Temp", iot_data_alloc_f32 (f32));
  return map;
}
