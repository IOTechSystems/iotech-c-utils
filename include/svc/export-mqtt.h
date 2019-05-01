#include <iot/bus.h>
#include <MQTTClient.h>

#ifndef CUTILS_MQTT_H
#define CUTILS_EXPORT_MQTT_H
#define MAX_RETRIES 5
#define CONNECT_TIMEOUT 5
#define TIMEOUT 10000L

typedef struct xrt_mqtt_exporter_t xrt_mqtt_exporter_t;

/* structs to simplify allocations for tls use or without tls*/
struct mqtt_info {
  int keep_alive_interval;
  int message_schematics;
  int persistance_type;
  long time_out;
  const char * address;
  const char * client_id;
  const char * username;
  const char * password;
  const char * topic;
  const char * match;
};

struct mqtt_ssl_info {
  int enable_server_cert_auth;
  int ssl_version;
  const char * trust_store;
  const char * key_store;
  const char * private_key_password;
  const char * enabled_cipher_suites;
};

/* Lifecycle operations */
extern bool xrt_mqtt_exporter_start (xrt_mqtt_exporter_t * state);
extern void xrt_mqtt_exporter_stop (xrt_mqtt_exporter_t * state);
extern void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t * state);

/* alloc functions, each alloc will sub to a bus topic, set bool here to sub to a topic enabled for communication. Once a topic has been picked up it is then allocated for use */
xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (struct mqtt_info mqtt, iot_bus_t * iot_bus, bool sub_to_topic_config);
extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (struct mqtt_info mqtt, struct mqtt_ssl_info mqtt_ssl, iot_bus_t * bus,  bool sub_to_topic_config);


#endif //CUTILS_MQTT_H
