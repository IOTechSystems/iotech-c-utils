#include <iot/bus.h>
#include <MQTTClient.h>

#ifndef CUTILS_MQTT_H
#define CUTILS_EXPORT_MQTT_H
#define TIMEOUT 10000L
/*default address */
#define BROKER_ADDRESS "tcp://54.171.53.113:13939"
#define IOT_MQTT_TYPE "IOT::Mqtt"


typedef struct mqtt_exporter_t mqtt_exporter_t;

/*
 * mqtt_info struct, simplifies allocation method,
 * client should use topic_export_single if they want to
 * export to a single topic.
 */
struct mqtt_info
{
  int keep_alive_interval;
  int message_schematics;
  int persistance_type;
  bool retain_message_server;
  long time_out;
  const char *address;
  const char *client_id;
  const char *username;
  const char *password;
  const char *topic_export_single;
  const char *match;
};

/*
 * mqtt_ssl_info struct, simplifies allocation method,
 * client should use mqtt_info as well as mqtt_ssl_info
 * to enable to use of tls.
 */
struct mqtt_ssl_info
{
  int enable_server_cert_auth;
  int ssl_version;
  const char *trust_store;
  const char *key_store;
  const char *private_key_password;
  const char *enabled_cipher_suites;
};

/* Lifecycle operations */
extern bool mqtt_exporter_start (mqtt_exporter_t *state);
extern void mqtt_exporter_stop (mqtt_exporter_t *state);
extern void mqtt_exporter_free (mqtt_exporter_t *state);

/* Generic push function, called internally, but client can call if they wish to */
extern bool mqtt_exporter_push_generic (mqtt_exporter_t *exporter, const iot_data_t *data, const char *topic);

/* helper function to check if client is still connected to the end point. */
extern bool connection_alive (mqtt_exporter_t * exporter);

/* alloc functions, each alloc will sub to a bus topic, set bool here to sub to a topic enabled for communication. Once a topic has been picked up it is then allocated for use */
extern mqtt_exporter_t * mqtt_exporter_alloc (struct mqtt_info mqtt, iot_bus_t *bus, bool export_single_topic);
extern mqtt_exporter_t * mqtt_exporter_ssl_alloc (struct mqtt_info mqtt, struct mqtt_ssl_info mqtt_ssl, iot_bus_t *bus, bool export_single_topic);

const iot_component_factory_t * iot_mqtt_factory (void);

#endif //CUTILS_MQTT_H
