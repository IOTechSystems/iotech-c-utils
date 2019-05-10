#include <iot/threadpool.h>
#include <svc/export-mqtt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>
#include <iot/container.h>

#define TIME_OUT 10000L

typedef struct xrt_mqtt_exporter_t
{
  iot_component_t component;      /* Component base type */
  MQTTClient client;
  iot_bus_sub_t * sub;
  int message_schematics;
  pthread_rwlock_t lock;
  const char * topic;
  MQTTClient_connectOptions conn_opts;
  MQTTClient_SSLOptions ssl_options;
  bool use_ssl;
}xrt_mqtt_exporter_t;


void xrt_mqtt_exporter_push_generic (xrt_mqtt_exporter_t * exporter, const iot_data_t * data, const char * topic)
{
    pthread_rwlock_wrlock (&exporter->lock);
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    char *json = iot_data_to_json (data, true);
    pubmsg.payload = json;
    pubmsg.payloadlen = strlen (json);
    pubmsg.qos = exporter->message_schematics;
    pubmsg.retained = false;
    MQTTClient_publishMessage (exporter->client, topic, &pubmsg, &token);
    int ret = MQTTClient_waitForCompletion (exporter->client, token, TIMEOUT);
    ret != MQTTCLIENT_SUCCESS ? printf ("Error: %d\n", ret) : printf ("json %s\n", json);
    free (json);
    pthread_rwlock_unlock (&exporter->lock);
}

/**
 * \brief Method pushes to a topic, if the topic is given via the clients methods it will
 * deconstruct the array, otherwise it will use the topic supplied during alloc,
 * within the mqtt struct. uses the push generic function to actually export the data
 * to the correct topic.
 *
 * \param data data to be exported
 * \param self will be set to the exporter
 * \param actual data that has been matched
 */
static void push_topic (iot_data_t *data, void *self, const char *match)
{
  xrt_mqtt_exporter_t *exporter = (xrt_mqtt_exporter_t *) self;

  if (iot_data_type (data) == IOT_DATA_ARRAY && iot_data_array_size (data) == 3 &&
      iot_data_type (iot_data_array_get (data, 2)) == IOT_DATA_BOOL &&
      iot_data_bool (iot_data_array_get (data, 2)) == true)
  {
    xrt_mqtt_exporter_push_generic (exporter, iot_data_array_get (data, 1), iot_data_string (iot_data_array_get (data, 0)));
  }
  else
  {
    xrt_mqtt_exporter_push_generic (exporter, data, exporter->topic);
  }
}

static void free_mqtt (xrt_mqtt_exporter_t * mqtt)
{
  pthread_rwlock_wrlock (&mqtt->lock);
  MQTTClient_disconnect (mqtt->client, 10);
  MQTTClient_destroy (&mqtt->client);
  pthread_rwlock_unlock (&mqtt->lock);
  free (mqtt);
}

static xrt_mqtt_exporter_t * xrt_mqtt_exporter_common_alloc (struct mqtt_info mqtt, MQTTClient_connectOptions *conn_opts,  bool export_single_topic)
{
  xrt_mqtt_exporter_t * mqtt_exporter = calloc (1, sizeof (xrt_mqtt_exporter_t));

  if (MQTTClient_create (&mqtt_exporter->client, mqtt.address, mqtt.client_id, mqtt.persistance_type, NULL) != MQTTCLIENT_SUCCESS)
  {
    goto exit;
  }
  mqtt_exporter->component.start_fn = (iot_component_start_fn_t) xrt_mqtt_exporter_start;
  mqtt_exporter->component.stop_fn =  (iot_component_stop_fn_t) xrt_mqtt_exporter_stop;
  mqtt_exporter->message_schematics = mqtt.message_schematics;
  mqtt_exporter->topic = export_single_topic ? mqtt.topic_export_single : NULL;
  conn_opts->username = mqtt.username;
  conn_opts->password = mqtt.password;
  conn_opts->keepAliveInterval = mqtt.keep_alive_interval;
  conn_opts->cleansession = 1;
  conn_opts->connectTimeout = mqtt.time_out;
  conn_opts->MQTTVersion = 3;

  return mqtt_exporter;

  exit:
  free_mqtt (mqtt_exporter);
  return NULL;
}

/**
 * \brief Alloc method, allocs the data needed to use a mqtt_exporter
 * \param mqtt struct which contains all the data required to set up mqtt without ssl.
 * \param bus databus which the mqtt exporter needs to sub to for data export
 * \param sub_to_topic_config boolean which needs to be set if client wants to listen to conf settings on which topics to publish too
 * \param export_single_topic boolean to let the method know that the client only wants to export to a single topic throughout the lifecycle of the program.
 * \return mqtt_exporter or NULL
 */
xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (struct mqtt_info mqtt, iot_bus_t * bus, bool export_single_topic)
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  xrt_mqtt_exporter_t * mqtt_exporter = xrt_mqtt_exporter_common_alloc (mqtt, &conn_opts, export_single_topic);
  mqtt_exporter->conn_opts = conn_opts;

  if (bus)
  {
    mqtt_exporter->sub = iot_bus_sub_alloc (bus, mqtt_exporter, push_topic, mqtt.match);
  }
  mqtt_exporter->use_ssl = false;
  return mqtt_exporter;
}

/**
 * \brief Alloc method, allocs the data needed to use a mqtt_exporter with ssl
 * \param mqtt struct which contains all the data required to set up mqtt without ssl.
 * \param mqtt_ssl struct which contains all the data required to set up mqtt with ssl.
 * \param bus databus which the mqtt exporter needs to sub to for data export
 * \param sub_to_topic_config boolean which needs to be set if client wants to listen to conf settings on which topics to publish too
 * \param export_single_topic boolean to let the method know that the client only wants to export to a single topic throughout the lifecycle of the program.
 * \return mqtt_exporter or NULL
 */
extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (struct mqtt_info mqtt, struct mqtt_ssl_info mqtt_ssl, iot_bus_t * bus, bool export_single_topic)
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  xrt_mqtt_exporter_t * mqtt_exporter = xrt_mqtt_exporter_common_alloc (mqtt, &conn_opts, export_single_topic);

  MQTTClient_SSLOptions ssl_options = MQTTClient_SSLOptions_initializer;
  conn_opts.ssl = &ssl_options;
  conn_opts.ssl->trustStore = mqtt_ssl.trust_store;
  conn_opts.ssl->keyStore = mqtt_ssl.key_store;
  conn_opts.ssl->privateKeyPassword = mqtt_ssl.private_key_password;
  conn_opts.ssl->enableServerCertAuth = mqtt_ssl.enable_server_cert_auth;
  conn_opts.ssl->sslVersion = mqtt_ssl.ssl_version;
  conn_opts.ssl->enabledCipherSuites = mqtt_ssl.enabled_cipher_suites;
  mqtt_exporter->conn_opts = conn_opts;

  if (bus)
  {
    mqtt_exporter->sub = iot_bus_sub_alloc (bus, mqtt_exporter, push_topic, mqtt.match);
  }
  mqtt_exporter->ssl_options = ssl_options;
  mqtt_exporter->use_ssl = true;

  return mqtt_exporter;
}


extern bool xrt_mqtt_exporter_start (xrt_mqtt_exporter_t * state)
{
  pthread_rwlock_wrlock (&state->lock);
  if (state->use_ssl)
  {
    state->conn_opts.ssl = &state->ssl_options;
  }
  if ( MQTTClient_connect (state->client, &state->conn_opts) != MQTTCLIENT_SUCCESS)
  {
    pthread_rwlock_unlock (&state->lock);
    return false;
  }

  if (state->component.state != IOT_COMPONENT_RUNNING)
  {
    state->component.state = IOT_COMPONENT_RUNNING;
  }
  pthread_rwlock_unlock (&state->lock);
  return true;
}

extern void xrt_mqtt_exporter_stop (xrt_mqtt_exporter_t * state)
{
  pthread_rwlock_wrlock (&state->lock);
  if (state->component.state != IOT_COMPONENT_STOPPED)
  {
    MQTTClient_disconnect (state->client, TIME_OUT);
    state->component.state = IOT_COMPONENT_STOPPED;
  }
  pthread_rwlock_unlock (&state->lock);
}


void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t * state)
{
  if (state)
  {
    state->component.state = IOT_COMPONENT_DELETED;
    free_mqtt (state);
  }
}

/* Container support */

static iot_component_t * iot_mqtt_config (iot_container_t * cont, const iot_data_t * map)
{
  const char * name = iot_data_string_map_get_string (map, "Bus");
  //bus passed to the configuration
  iot_bus_t * bus = (iot_bus_t*) iot_container_find (cont, name);
  assert (bus);
  struct mqtt_info mqtt_info;
  mqtt_info.topic_export_single = iot_data_string_map_get_string (map, "topic_export_single");
  mqtt_info.address = iot_data_string_map_get_string (map, "address");
  mqtt_info.client_id = iot_data_string_map_get_string (map, "client_id");
  mqtt_info.username = iot_data_string_map_get_string (map, "username");
  mqtt_info.password = iot_data_string_map_get_string (map, "password");
  mqtt_info.match = iot_data_string_map_get_string (map, "match");
  mqtt_info.keep_alive_interval = iot_data_i64 (iot_data_string_map_get (map, "keep_alive_interval"));
  mqtt_info.message_schematics = iot_data_i64 (iot_data_string_map_get (map, "message_schematics"));
  mqtt_info.time_out = iot_data_i64 (iot_data_string_map_get (map, "time_out"));
  mqtt_info.persistance_type = iot_data_i64 (iot_data_string_map_get (map, "persistance_type"));

  struct mqtt_ssl_info mqtt_ssl_info;
  xrt_mqtt_exporter_t * mqtt_exporter;
  if (strcmp (iot_data_string_map_get_string (map, "ssl"), "true") == 0)
  {
    mqtt_ssl_info.trust_store = iot_data_string_map_get_string (map, "trust_store");
    mqtt_ssl_info.key_store = iot_data_string_map_get_string (map, "key_store");
    mqtt_ssl_info.private_key_password = iot_data_string_map_get_string (map, "private_key_password");
    mqtt_ssl_info.enabled_cipher_suites = iot_data_string_map_get_string (map, "enabled_cipher_suites");
    mqtt_ssl_info.ssl_version = iot_data_i64 (iot_data_string_map_get (map, "ssl_version"));
    mqtt_ssl_info.enable_server_cert_auth =iot_data_i64 (iot_data_string_map_get (map, "enable_server_cert_auth"));
    mqtt_exporter = xrt_mqtt_exporter_ssl_alloc (mqtt_info, mqtt_ssl_info, bus, true);
  }
  else
  {
    mqtt_exporter = xrt_mqtt_exporter_alloc (mqtt_info, bus, true);
  }
  return &mqtt_exporter->component;
}

const iot_component_factory_t * iot_mqtt_factory (void)
{
  static iot_component_factory_t factory = { IOT_MQTT_TYPE, iot_mqtt_config, (iot_component_free_fn_t) xrt_mqtt_exporter_free };
  return &factory;
}