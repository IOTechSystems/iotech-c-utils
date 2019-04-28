#include <iot/threadpool.h>
#include <svc/export-mqtt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

struct mqtt_info {
    iot_bus_t * pubsub,
    const char * match,
    const char * address,
    const char * client_id,
    const char * username,
    const char * password,
    const char * topic,
    int keep_alive_interval,
    long time_out,
    int message_schematics,
    int persistance_type,
    char * const * server_uri;
};

struct mqtt_ssl_info {
    const char * trust_store,
    const char * key_store,
    const char * private_key,
    const char * private_key_password,
    const char * enabled_cipher_suites,
    int enable_server_cert_auth,
    int ssl_version,
    int verify,
    const char * ca_path,
    void * ssl_error_context;
};

static MQTTClient_message common_push (iot_data_t * data, void * self, const char * match, char *json)
{
  xrt_mqtt_exporter_t *exporter = (xrt_mqtt_exporter_t *)self;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;

  json = iot_data_to_json (data, true);
  pubmsg.payload = json;
  pubmsg.qos = exporter->message_schematics;
  pubmsg.retained = 0;

  return pubmsg;
}

static void push_bus (iot_data_t * data, void * self, const char * match)
{
  xrt_mqtt_exporter_t *exporter = (xrt_mqtt_exporter_t *)self;
  char * json = NULL;
  MQTTClient_message pubmsg = common_push (data, self, match, json);
  MQTTClient_deliveryToken token;

  MQTTClient_publishMessage (exporter->client, exporter->topic, &pubmsg, &token);
  if (MQTTClient_waitForCompletion (exporter->client, token, exporter->time_out) != MQTTCLIENT_SUCCESS)
  {
    printf ("mqtt: error publishing: %d\n", token);
  }
  free (json);
}

void push_topic (iot_data_t * data, void * self, const char * match, char * topic)
{
  xrt_mqtt_exporter_t *exporter = (xrt_mqtt_exporter_t *)self;

  char * json = NULL;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;

  json = iot_data_to_json (data, true);
  pubmsg.payload = json;
  pubmsg.qos = exporter->message_schematics;
  pubmsg.retained = 0;

  MQTTClient_publishMessage (exporter->client, topic, &pubmsg, &token);
  if (MQTTClient_waitForCompletion (exporter->client, token, exporter->time_out) != MQTTCLIENT_SUCCESS)
  {
    printf ("mqtt: error publishing: %d\n", token);
  }
  free (json);
}


static void process_mqtt (void *p)
{
  xrt_mqtt_exporter_t *me = (xrt_mqtt_exporter_t *)p;

  int topicLength = strlen (me->topic);
  while (MQTTClient_isConnected (me->client))
  {
    MQTTClient_message * message;
    MQTTClient_receive (me->client, &me->topic, &topicLength, &message, me->time_out);
  }
}

static xrt_mqtt_exporter_t * xrt_mqtt_exporter__common_alloc (iot_bus_t *pubsub, struct mqtt_info mqtt)
{
    xrt_mqtt_exporter_t *state = calloc (1, sizeof (xrt_mqtt_exporter_t));

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    if (MQTTClient_create (&state->client, mqtt.address, mqtt.client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS){
        exit (0);
    }
    conn_opts.username = mqtt.username;
    conn_opts.password = mqtt.password;
    conn_opts.keepAliveInterval = mqtt.keep_alive_interval;
    conn_opts.cleansession = 1;
    conn_opts.connectTimeout = mqtt.time_out;
    conn_opts.serverURIs = mqtt.time_out;

    return state;
}

xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (iot_bus_t *pubsub, struct mqtt_info mqtt)
{
  xrt_mqtt_exporter_t *state = xrt_mqtt_exporter__common_alloc (pubsub, mqtt);

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
    printf ("Main");
    state->message_schematics = message_schematics;
    state->topic = iot_strdup (topic);
    state->sub = iot_bus_sub_alloc (pubsub, state, push_bus, match);
    state->thpool = iot_threadpool_alloc (1, NULL);
    iot_threadpool_add_work (state->thpool, process_mqtt, state, NULL);
  }
  else
  {
    free (state);
    state = NULL;
  }

  return state;
}

extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (iot_bus_t *pubsub, struct mqtt_info, struct mqtt_ssl_info)
{
    xrt_mqtt_exporter_t *state = xrt_mqtt_exporter__common_alloc (pubsub, mqtt);

    MQTTClient_SSLOptions ssl_options = MQTTClient_SSLOptions_initializer;
    ssl_options.trustStore = mqtt_ssl_info.trust_store;
    ssl_options.keyStore =  mqtt_ssl_info.key_store;
    ssl_options.privateKey =  mqtt_ssl_info.privateKey;
    ssl_options.privateKeyPassword = mqtt_ssl_info.private_key_password;
    ssl_options.enabledCipherSuites = mqtt_ssl_info.enabled_cipher_suites;
    ssl_options.enableServerCertAuth = mqtt_ssl_info.enabled_server_cert_auth;
    ssl_options.sslVersion = mqtt_ssl_info.ssl_version;
    ssl_options.verify = mqtt_ssl_info.verify;
    ssl_options.CApath = mqtt_ssl_info.ca_path;

    int rc;
    for (unsigned i = 0; i < MAX_RETRIES; ++i)
    {
        if ((rc = MQTTClient_connect (state->client, &ssl_options)) != MQTTCLIENT_SUCCESS)
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
        printf ("Main");
        state->message_schematics = message_schematics;
        state->topic = iot_strdup (topic);
        state->sub = iot_bus_sub_alloc (pubsub, state, push_bus, match);
        state->thpool = iot_threadpool_alloc (1, NULL);
        iot_threadpool_add_work (state->thpool, process_mqtt, state, NULL);
    }
    else
    {
        free (state);
        state = NULL;
    }

    return state;
}

void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t * state)
{
  if (state)
  {
    MQTTClient_disconnect (state->client, 10000);
    MQTTClient_destroy (&state->client);
    iot_threadpool_wait (state->thpool);
    iot_threadpool_stop (state->thpool);
    iot_bus_sub_free (state->sub);
    free (state);
  }
}