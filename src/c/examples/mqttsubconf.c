#include <iot/bus.h>
#include <svc/export-mqtt.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>

#define ARRAY_SIZE 3
#define PUB_ITERS 10
#define broker_address "ssl://localhost:8883"


static iot_data_t * publisher_callback (void * self);
static void publish (iot_bus_pub_t * pub, uint32_t iters);
void init_info_mqtt (struct mqtt_info * mqtt);
void init_info_mqtt_ssl (struct mqtt_ssl_info * ssl_info);

int main ()
{
  iot_data_init ();
  iot_threadpool_t * thread_pool = iot_threadpool_alloc (4, 0, NULL);
  iot_scheduler_t * scheduler = iot_scheduler_alloc (thread_pool);

  iot_bus_t * bus = iot_bus_alloc (scheduler, thread_pool, 200000);

  //publication for mqtt config, to export to topics.
  iot_bus_pub_t * pub_configuration = iot_bus_pub_alloc (bus, NULL, publisher_callback, "configuration");
  iot_bus_pub_t * pub = iot_bus_pub_alloc (bus, NULL, publisher_callback, "test/tube");




}

static void publish_configuration (iot_bus_pub_t * pub)
{
  iot_data_t * topic1 = iot_data_alloc_string ("topic1", true);
  iot_data_t * topic2 = iot_data_alloc_string ("topic2", true);
  iot_data_t * topic3 = iot_data_alloc_string ("topic3", true);
  iot_data_t * topic4 = iot_data_alloc_string ("topic4", true);

  /* Push Topics for mqtt service too export too. These names then should be available when data comes in from the sub.*/
  iot_bus_pub_push (pub, topic1, false);
  iot_bus_pub_push (pub, topic2, false);
  iot_bus_pub_push (pub, topic3, false);
  iot_bus_pub_push (pub, topic4, false);


}


void init_info_mqtt (struct mqtt_info * mqtt)
{
  mqtt->address = broker_address;
  mqtt->topic_export = "test";
  mqtt->username = "";
  mqtt->password = "";
  mqtt->client_id = "JD";
  mqtt->keep_alive_interval = 20;
  mqtt->time_out = 10000L;
  mqtt->message_schematics = 1;
  mqtt->persistance_type = MQTTCLIENT_PERSISTENCE_NONE;
  mqtt->match = "#";
}

void init_info_mqtt_ssl (struct mqtt_ssl_info * ssl_info)
{
  ssl_info->trust_store = "/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/cacert.pem";
  //key and cert in one file.
  ssl_info->key_store = "/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/localhost.pem";
  ssl_info->private_key_password = "";
  ssl_info->enable_server_cert_auth = 1;
  ssl_info->ssl_version = MQTT_SSL_VERSION_TLS_1_2;
  ssl_info->enabled_cipher_suites = "ALL";
}