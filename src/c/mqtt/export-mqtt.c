#include <iot/threadpool.h>
#include <svc/export-mqtt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>

typedef struct xrt_mqtt_exporter_t
{
  iot_component_t component;      /* Component base type */
  MQTTClient client;
  iot_bus_sub_t * sub;
  iot_bus_sub_t * sub_config;
  iot_bus_t * bus;
  int message_schematics;
  const char * topic;
  pthread_rwlock_t lock;
}xrt_mqtt_exporter_t;

static void push_topic (iot_data_t * data, void * self, const char * match)
{
  if (data == NULL)
  {
    return;
  }
  int i = 5;
  xrt_mqtt_exporter_t *exporter = (xrt_mqtt_exporter_t *) self;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;

  char *json = iot_data_to_json (data, true);
  pubmsg.payload = json;
  pubmsg.payloadlen = strlen (json);
  pubmsg.qos = exporter->message_schematics;
  pubmsg.retained = true;

  int ret;
  while (i--)
  {
    MQTTClient_publishMessage (exporter->client, exporter->topic, &pubmsg, &token);
    if ((ret = MQTTClient_waitForCompletion (exporter->client, token, TIMEOUT)) != MQTTCLIENT_SUCCESS)
    {
      printf ("Error: %d\n", ret);
    }
    else
    {
      printf ("json %s\n", json);
      break;
    }
  }
  free (json);
}

static void sub_config (iot_data_t * data, void * self, const char * match)
{
  if (data == NULL)
  {
    return;
  }
  xrt_mqtt_exporter_t * exporter = (xrt_mqtt_exporter_t *) self;
  exporter->sub =  iot_bus_sub_alloc (exporter->bus, exporter, push_topic,  iot_data_string (data));
}


static xrt_mqtt_exporter_t * xrt_mqtt_exporter__common_alloc (struct mqtt_info mqtt, MQTTClient_connectOptions *conn_opts)
{
  xrt_mqtt_exporter_t *state = calloc (1, sizeof (xrt_mqtt_exporter_t));

  if (MQTTClient_create (&state->client, mqtt.address, mqtt.client_id, mqtt.persistance_type, NULL) !=
      MQTTCLIENT_SUCCESS)
  {
    exit (0);
  }

  state->component.start_fn = (iot_component_start_fn_t) xrt_mqtt_exporter_start;
  state->component.stop_fn =  (iot_component_stop_fn_t) xrt_mqtt_exporter_stop;

  conn_opts->username = mqtt.username;
  conn_opts->password = mqtt.password;
  conn_opts->keepAliveInterval = mqtt.keep_alive_interval;
  conn_opts->cleansession = 1;
  conn_opts->connectTimeout = mqtt.time_out;
  state->message_schematics = mqtt.message_schematics;
  state->topic = mqtt.topic;

  return state;
}

xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (struct mqtt_info mqtt, iot_bus_t * iot_bus, bool sub_to_topic_config)
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  xrt_mqtt_exporter_t * state = xrt_mqtt_exporter__common_alloc (mqtt, &conn_opts);

  int rc;
  for (unsigned i = 0; i < MAX_RETRIES; ++i)
  {
    if ((rc = MQTTClient_connect (state->client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
      sleep (CONNECT_TIMEOUT);
      printf ("Failed to connect, return code %d\n", rc);
      continue;
    }
    break;
  }

  if (MQTTClient_isConnected (state->client))
  {
    state->message_schematics = mqtt.message_schematics;
    state->topic = mqtt.topic;
    if (sub_to_topic_config)
    {
      state->sub_config = iot_bus_sub_alloc (iot_bus, state, sub_config, mqtt.match);
      state->bus = iot_bus;
    }
    else
    {
      state->sub = iot_bus_sub_alloc (iot_bus, state, push_topic, mqtt.match);
    }
  }
  else
  {
    free (state);
    state = NULL;
  }
  return state;
}

extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (struct mqtt_info mqtt, struct mqtt_ssl_info mqtt_ssl, iot_bus_t * bus, bool sub_to_topic_config)
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  xrt_mqtt_exporter_t * state = xrt_mqtt_exporter__common_alloc (mqtt, &conn_opts);

  state->component.start_fn = (iot_component_start_fn_t) xrt_mqtt_exporter_start;
  state->component.stop_fn =  (iot_component_stop_fn_t) xrt_mqtt_exporter_stop;


  MQTTClient_SSLOptions ssl_options = MQTTClient_SSLOptions_initializer;
  conn_opts.ssl = &ssl_options;
  conn_opts.ssl->trustStore = mqtt_ssl.trust_store;
  conn_opts.ssl->keyStore = mqtt_ssl.key_store;
  conn_opts.ssl->privateKeyPassword = mqtt_ssl.private_key_password;
  conn_opts.ssl->enableServerCertAuth = mqtt_ssl.enable_server_cert_auth;
  conn_opts.ssl->sslVersion = mqtt_ssl.ssl_version;
  conn_opts.ssl->enabledCipherSuites = mqtt_ssl.enabled_cipher_suites;

  state->message_schematics = mqtt.message_schematics;
  state->topic = mqtt.topic;
  for (unsigned i = 0; i < MAX_RETRIES; ++i)
  {
    int ret;
    if ( (ret = MQTTClient_connect (state->client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
      printf ("Failed to connect, return code %d\n", ret);
    }
    else
    {
      printf ("is conn");
      break;
    }
  }

  if (MQTTClient_isConnected (state->client))
  {
    state->message_schematics = mqtt.message_schematics;
    state->topic = mqtt.topic;
    if (sub_to_topic_config)
    {
      state->sub_config = iot_bus_sub_alloc (bus, state, sub_config, mqtt.match);
      state->bus = bus;
    }
    else
    {
      state->sub = iot_bus_sub_alloc (bus, state, push_topic, mqtt.match);
    }
  }
  else
  {
    free (state);
    state = NULL;
  }
  return state;
}


extern bool xrt_mqtt_exporter_start (xrt_mqtt_exporter_t * state)
{
  pthread_rwlock_wrlock (&state->lock);
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
    state->component.state = IOT_COMPONENT_STOPPED;
  }
  pthread_rwlock_unlock (&state->lock);
}


void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t *state)
{
  if (state)
  {
    MQTTClient_disconnect (state->client, 10);
    MQTTClient_destroy (&state->client);
    iot_bus_sub_free (state->sub);
    free (state);
  }
}