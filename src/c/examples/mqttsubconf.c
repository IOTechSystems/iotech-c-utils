#include <iot/bus.h>
#include <svc/export-mqtt.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>

#define ARRAY_SIZE 3
#define PUB_ITERS 10

/*
 * Subscribes to Bus topic for configuration (topic on which to publish)
 *
 * does this by the user telling the user what topic to publish too with each packet of data.
 * check publisher_callback function.
 *
 * topic is first element
 * data is the second element.
 * */

static iot_data_t * publisher_callback (void * self);
void publish (iot_bus_pub_t * pub, uint32_t iters, char * topic);
void init_info_mqtt (struct mqtt_info * mqtt);
void init_info_mqtt_ssl (struct mqtt_ssl_info * ssl_info);

int main (void)
{
  iot_data_init ();
  iot_threadpool_t * thread_pool = iot_threadpool_alloc (8, 0, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (thread_pool);
  iot_bus_t * bus = iot_bus_alloc (scheduler, thread_pool,200000);

  //mqtt, set up configuration.
  struct mqtt_info mqtt;
  init_info_mqtt (&mqtt);
  mqtt_exporter_t * mqtt_exporter = mqtt_exporter_alloc (mqtt, bus, true);

  iot_threadpool_start (thread_pool);
  iot_scheduler_start (scheduler);
  iot_bus_start (bus);
  if (mqtt_exporter != NULL)
  {
    mqtt_exporter_start (mqtt_exporter);
  }
  char * topic = "test";

  iot_bus_pub_t * pub = iot_bus_pub_alloc (bus, "test_another", publisher_callback, "test/tube");
  publish (pub, 5, topic);
  sleep (5);

  iot_bus_stop (bus);
  iot_scheduler_stop (scheduler);
  iot_threadpool_stop (thread_pool);
  if (mqtt_exporter != NULL)
  {
    mqtt_exporter_stop (mqtt_exporter);
  }

  if (mqtt_exporter != NULL)
  {
    mqtt_exporter_free (mqtt_exporter);
  }
  iot_bus_free (bus);
  iot_scheduler_free (scheduler);
  iot_threadpool_free (thread_pool);

  iot_data_fini ();
  return 0;
}

/* init a mqtt struct */
void init_info_mqtt (struct mqtt_info * mqtt)
{
  mqtt->address = BROKER_ADDRESS;
  mqtt->topic_export_single = NULL;
  mqtt->username = "hsfunheq";
  mqtt->password = "6n5OVDPYRnCr";
  mqtt->client_id = "JD";
  mqtt->keep_alive_interval = 100;
  mqtt->time_out = 10000L;
  mqtt->message_schematics = 1;
  mqtt->persistance_type = MQTTCLIENT_PERSISTENCE_NONE;
  mqtt->match = "#";
}

/* init a mqtt struct for ssl*/
void init_info_mqtt_ssl (struct mqtt_ssl_info * ssl_info)
{
  ssl_info->trust_store = "/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/cacert.pem";
  ssl_info->key_store = "/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/localhost.pem";
  ssl_info->private_key_password = "";
  ssl_info->enable_server_cert_auth = 1;
  ssl_info->ssl_version = MQTT_SSL_VERSION_TLS_1_2;
  ssl_info->enabled_cipher_suites = "ALL";
}

/* Encode topic here,  */
static iot_data_t *publisher_callback (void *self)
{
  iot_data_t *msg = iot_data_alloc_array (3);
  iot_data_array_add (msg, 0, iot_data_alloc_string ((char *) self, true));
  iot_data_t *type = iot_data_alloc_bool (true);
  iot_data_array_add (msg, 2, type);

  static float f32 = 20.00;
  f32 = (float) (f32 * 1.02);
  iot_data_t *map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-7", false));
  iot_data_string_map_add (map, "Temp", iot_data_alloc_f32 (f32));

  iot_data_array_add (msg, 1, map);
  return msg;
}


void publish (iot_bus_pub_t * pub, uint32_t iters, char * topic)
{
  iot_data_t * msg = iot_data_alloc_array (3);
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * array = iot_data_alloc_array (ARRAY_SIZE);
  uint32_t index = 0;

  // Create fixed part of sample
  iot_data_array_add (array, index++, iot_data_alloc_i32 (11));
  iot_data_array_add (array, index++, iot_data_alloc_i32 (22));
  iot_data_array_add (array, index, iot_data_alloc_i32 (33));
  iot_data_string_map_add (map, "Coords", array);
  iot_data_string_map_add (map, "Origin", iot_data_alloc_string ("Sensor-54", false));
  iot_data_array_add (msg, 0, iot_data_alloc_string (topic, true));
  iot_data_t * type = iot_data_alloc_bool (true);
  iot_data_array_add (msg, 2, type);
  iot_data_array_add (msg, 1, map);

  while (iters--)
  {
    iot_data_string_map_add (map, "#", iot_data_alloc_i32 (PUB_ITERS - iters));
    iot_data_addref (msg);
    iot_bus_pub_push (pub, msg, false);
  }
  iot_data_free (msg);
}