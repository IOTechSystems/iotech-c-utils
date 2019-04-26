#include <iot/threadpool.h>
#include <svc/export-mqtt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

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

xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (iot_bus_t *pubsub, const char *match, const char *address, const char *client_id,
                         const char *username, const char *password, const char *topic, int keept_alive_interval, long time_out,
                         const int message_schematics, const int persistance_type, char *const *server_uri)
{
  xrt_mqtt_exporter_t *state = calloc (1, sizeof (xrt_mqtt_exporter_t));

  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  if (MQTTClient_create (&state->client, address, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS){
    exit (0);
  }
  conn_opts.username = username;
  conn_opts.password = password;
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  conn_opts.connectTimeout = time_out;
  conn_opts.serverURIs = server_uri;

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