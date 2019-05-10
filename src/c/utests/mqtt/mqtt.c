#include "mqtt.h"
#include "CUnit.h"

static void init_info_mqtt (struct mqtt_info * mqtt)
{
  mqtt->address = BROKER_ADDRESS;
  mqtt->topic_export_single = "test";
  mqtt->username = "hsfunheq";
  mqtt->password = "6n5OVDPYRnCr";
  mqtt->client_id = "JD";
  mqtt->keep_alive_interval = 1;
  mqtt->time_out = 10000L;
  mqtt->message_schematics = 1;
  mqtt->persistance_type = MQTTCLIENT_PERSISTENCE_NONE;
  mqtt->match = "#";
}

static void init_info_mqtt_ssl (struct mqtt_ssl_info * ssl_info)
{
  ssl_info->trust_store = "/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/cacert.pem";
  ssl_info->key_store = "/home/jordan/CLionProjects/iotech-c-utils-2/src/c/examples/localhost.pem";
  ssl_info->private_key_password = "";
  ssl_info->enable_server_cert_auth = 1;
  ssl_info->ssl_version = MQTT_SSL_VERSION_TLS_1_2;
  ssl_info->enabled_cipher_suites = "ALL";
}

static int suite_init (void)
{
  iot_data_init ();
  return 0;
}

static int suite_clean (void)
{
  iot_data_fini ();
  return 0;
}

static void test_mqtt_exporter_fail_alloc (void)
{
  struct mqtt_info mqtt;
  struct mqtt_ssl_info ssl_info;
  init_info_mqtt (&mqtt);
  init_info_mqtt_ssl (&ssl_info);

  mqtt.address = "rubbish_address";

  mqtt_exporter_t * mqtt_exporter = mqtt_exporter_ssl_alloc (mqtt, ssl_info, NULL, true);
  CU_ASSERT (mqtt_exporter != NULL);
  mqtt_exporter_free (mqtt_exporter);

  mqtt_exporter = mqtt_exporter_alloc (mqtt, NULL, true);
  CU_ASSERT (mqtt_exporter != NULL);
  mqtt_exporter_free (mqtt_exporter);

}

static void test_mqtt_exporter_alloc (void)
{
  struct mqtt_info mqtt;
  init_info_mqtt (&mqtt);

  mqtt_exporter_t * mqtt_exporter = mqtt_exporter_alloc (mqtt, NULL, true);
  CU_ASSERT (mqtt_exporter != NULL);
  mqtt_exporter_free (mqtt_exporter);

}

static void test_mqtt_exporter_test_export_random_topic (void)
{
  struct mqtt_info mqtt;
  init_info_mqtt (&mqtt);

  mqtt_exporter_t * mqtt_exporter = mqtt_exporter_alloc (mqtt, NULL, true);
  CU_ASSERT (mqtt_exporter != NULL);

  mqtt_exporter_start (mqtt_exporter);
  const char * topic = "rand";
  iot_data_t * data = iot_data_alloc_string ("Item", true);

  CU_ASSERT (mqtt_exporter_push_generic (mqtt_exporter, data, topic) == true);
  mqtt_exporter_stop (mqtt_exporter);
  mqtt_exporter_free (mqtt_exporter);

  iot_data_free (data);
}

static void test_mqtt_exporter_lifecycle (void)
{
  struct mqtt_info mqtt;
  init_info_mqtt (&mqtt);

  mqtt_exporter_t * mqtt_exporter = mqtt_exporter_alloc (mqtt, NULL, true);
  CU_ASSERT (mqtt_exporter != NULL);

  CU_ASSERT (mqtt_exporter_start (mqtt_exporter) == true);
  CU_ASSERT (connection_alive (mqtt_exporter) == true)
  mqtt_exporter_stop (mqtt_exporter);
  CU_ASSERT (connection_alive (mqtt_exporter) == false)
  mqtt_exporter_free (mqtt_exporter);
}

void cunit_mqtt_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("mqtt", suite_init, suite_clean);
  CU_add_test (suite, "mqtt_exporter_fail_alloc", test_mqtt_exporter_fail_alloc);
  CU_add_test (suite, "test_mqtt_exporter_alloc", test_mqtt_exporter_alloc);
  CU_add_test (suite, "test_mqtt_exporter_test_export_random_topic", test_mqtt_exporter_test_export_random_topic);
  CU_add_test (suite, "test_mqtt_exporter_lifecycle", test_mqtt_exporter_lifecycle);
}

