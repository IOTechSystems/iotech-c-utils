#include <iot/bus.h>
#include <svc/export-mqtt.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>

#define ARRAY_SIZE 5
#define ARRAY_MOD 1000
#define broker_add "54.171.53.113:13939"


static void testFunc1 (void *in)
{
  printf ("FN-1 ");
}

static iot_data_t * publisher_callback (void * self);

static void subscriber_callback (iot_data_t * data, void * self, const char * match)
{
#ifndef NDEBUG
  printf ("Subscription (%s): ", match);
  char * json = iot_data_to_json (data, true);
  printf ("%s\n", json);
  free (json);
#endif
}

int main (void)
{
 // iot_data_t * array = iot_data_array ();
  iot_threadpool_t * thread_pool = iot_threadpool_alloc (4, NULL);

  iot_scheduler_t * scheduler = iot_scheduler_alloc (thread_pool);

  iot_bus_t * bus = iot_bus_alloc (scheduler, thread_pool, 500);
  iot_bus_sub_alloc (bus, NULL, subscriber_callback, "test/#);
  iot_bus_pub_t * pub = iot_bus_pub_alloc (bus, NULL, publisher_callback, "test/tube");

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, testFunc1, NULL, IOT_MS_TO_NS (500), 0, 0, NULL);
  iot_schedule_add (scheduler, sched1);

  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  iot_bus_start (bus);
  publish (pub, PUB_ITERS);

  char * const * server = NULL;
  xrt_mqtt_exporter_t * mqtt_export = xrt_mqtt_exporter_alloc (bus, "#", broker_add, "clientid", "hsfunheq", "6n5OVDPYRnCr",
          "test", 500, 20, 20, MQTTCLIENT_PERSISTENCE_DEFAULT, server);

  return 0;
}
static void publish (iot_bus_pub_t * pub, uint32_t iters)
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
        iot_bus_pub_push (pub, map, true);
    }
    iot_data_free (map);
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

static void export_data (xrt_mqtt_exporter_t * mqtt_export, iot_bus_sub_t * sub)
{
    iot_data_t * map = iot_bus_sub_pull (iot_bus_sub_t * sub);
    push_topic (iot_bus_sub_pull (iot_bus_sub_t * sub), mqtt_export, "#", "test");
}