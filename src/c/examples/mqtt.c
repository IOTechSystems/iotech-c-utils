#include <iot/bus.h>
#include <svc/export-mqtt.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>

#define ARRAY_SIZE 3
#define broker_add "54.171.53.113:13939"
#define PUB_ITERS 10


static iot_data_t * publisher_callback (void * self);
static void publish (iot_bus_pub_t * pub, uint32_t iters);
void init_info_mqtt (struct mqtt_info * mqtt);

int main (void)
{
  iot_data_init ();
  iot_threadpool_t * thread_pool = iot_threadpool_alloc (4, 0, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (thread_pool);

  iot_bus_t * bus = iot_bus_alloc (scheduler, thread_pool, 200000);

  iot_bus_pub_t * pub = iot_bus_pub_alloc (bus, NULL, publisher_callback, "test/tube");
  struct mqtt_info mqtt;
  init_info_mqtt (&mqtt);
  xrt_mqtt_exporter_t *  mqttExporter = xrt_mqtt_exporter_alloc (mqtt, bus);

  iot_threadpool_start (thread_pool);
  iot_scheduler_start (scheduler);
  iot_bus_start (bus);


  publish (pub, PUB_ITERS);
  sleep (5);

  iot_bus_stop (bus);
  iot_scheduler_stop (scheduler);
  iot_threadpool_stop (thread_pool);
  xrt_mqtt_exporter_free (mqttExporter);
  iot_bus_free (bus);
  iot_scheduler_free (scheduler);
  iot_threadpool_free (thread_pool);
  iot_data_fini ();

  return 0;
}

void init_info_mqtt (struct mqtt_info * mqtt)
{
  mqtt->address = broker_add;
  mqtt->topic = "test/";
  mqtt->username = "hsfunheq";
  mqtt->password = "6n5OVDPYRnCr";
  mqtt->client_id = "clientid";
  mqtt->keep_alive_interval = 500;
  mqtt->time_out = 5000;
  mqtt->message_schematics = 1;
  mqtt->persistance_type = MQTTCLIENT_PERSISTENCE_DEFAULT;
  mqtt->server_uri = NULL;
  mqtt->match = "#";
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
    iot_data_string_map_add (map, "#", iot_data_alloc_i32 (PUB_ITERS - iters));

    iot_data_addref (map);
    iot_bus_pub_push (pub, map, false);
  }

  // Finally delete sampleutests/threadpool/threadpool.h
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
