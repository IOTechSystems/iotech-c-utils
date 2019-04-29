#include <iot/threadpool.h>
#include <svc/export-mqtt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>


void push_topic (iot_data_t * data, void * self, const char * match)
{
  if (data == NULL)
  {
    printf ("Hi");
    return;
  }
  xrt_mqtt_exporter_t *exporter = (xrt_mqtt_exporter_t *) self;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;

  char *json = iot_data_to_json (data, true);
  pubmsg.payload = json;
  pubmsg.qos = exporter->message_schematics;
  pubmsg.retained = 0;

  printf ("Subscription (%s): \n", json);

  MQTTClient_publishMessage (exporter->client, exporter->topic, &pubmsg, &token);
  int ret;
  if ((ret = MQTTClient_waitForCompletion (exporter->client, token, 5000)) != MQTTCLIENT_SUCCESS)
  {
    printf ("mqtt: error publishing: %d\n", token);
    printf ("mqtt: error ret: %d\n", ret);
  }
  free (json);
}

static xrt_mqtt_exporter_t * xrt_mqtt_exporter__common_alloc (struct mqtt_info mqtt, MQTTClient_connectOptions *conn_opts)
{
  xrt_mqtt_exporter_t *state = calloc (1, sizeof (xrt_mqtt_exporter_t));

  if (MQTTClient_create (&state->client, mqtt.address, mqtt.client_id, mqtt.persistance_type, NULL) !=
      MQTTCLIENT_SUCCESS)
  {
    exit (0);
  }
  conn_opts->username = mqtt.username;
  conn_opts->password = mqtt.password;
  conn_opts->keepAliveInterval = mqtt.keep_alive_interval;
  conn_opts->cleansession = 1;
  conn_opts->connectTimeout = mqtt.time_out;
  conn_opts->serverURIs = mqtt.server_uri;
  return state;
}

xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (struct mqtt_info mqtt, iot_bus_t * iot_bus)
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
    }
    else
    {
      printf ("Conn\n");
      break;
    }
  }

  if (MQTTClient_isConnected (state->client))
  {
    state->message_schematics = mqtt.message_schematics;
    state->topic = iot_strdup (mqtt.topic);
    state->sub = iot_bus_sub_alloc (iot_bus, state, push_topic, mqtt.match);
  }
  else
  {
    free (state);
    state = NULL;
  }

  return state;
}

extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (iot_bus_t *pubsub, struct mqtt_info mqtt, struct mqtt_ssl_info mqtt_ssl, iot_data_sub_fn_t callback)
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  xrt_mqtt_exporter_t *state = xrt_mqtt_exporter__common_alloc (mqtt, &conn_opts);

  MQTTClient_SSLOptions ssl_options = MQTTClient_SSLOptions_initializer;
  ssl_options.trustStore = mqtt_ssl.trust_store;
  ssl_options.keyStore = mqtt_ssl.key_store;
  ssl_options.privateKey = mqtt_ssl.private_key;
  ssl_options.privateKeyPassword = mqtt_ssl.private_key_password;
  ssl_options.enabledCipherSuites = mqtt_ssl.enabled_cipher_suites;
  ssl_options.enableServerCertAuth = mqtt_ssl.enable_server_cert_auth;
  ssl_options.sslVersion = mqtt_ssl.ssl_version;
  ssl_options.verify = mqtt_ssl.verify;
  ssl_options.CApath = mqtt_ssl.ca_path;

  conn_opts.ssl = &ssl_options;
  int rc;
  for (unsigned i = 0; i < MAX_RETRIES; ++i)
  {
    if ((rc = MQTTClient_connect (state->client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
      sleep (CONNECT_TIMEOUT);
      printf ("Failed to connect, return code %d\n", rc);
    }
    else
    {
      printf ("Conn");

      break;
    }
  }

  if (MQTTClient_isConnected (state->client))
  {
    state->message_schematics = mqtt.message_schematics;
    state->topic = iot_strdup (mqtt.topic);
  }
  else
  {
    free (state);
    state = NULL;
  }

  return state;
}

void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t *state)
{
  if (state)
  {
    MQTTClient_disconnect (state->client, 10);
    MQTTClient_destroy (&state->client);
    iot_bus_sub_free (state->sub);
    free (state->topic);
    free (state);
  }
}