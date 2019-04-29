#include <iot/bus.h>
#include <MQTTClient.h>

#ifndef CUTILS_MQTT_H
#define CUTILS_EXPORT_MQTT_H
#define MAX_RETRIES 5
#define CONNECT_TIMEOUT 500

typedef struct xrt_mqtt_exporter_t xrt_mqtt_exporter_t;

typedef struct xrt_mqtt_exporter_t
{
  MQTTClient client;
  char * topic;
  iot_threadpool_t * thpool;
  int message_schematics;
  long time_out;
  iot_bus_sub_t *sub;
}xrt_mqtt_exporter_t;

struct mqtt_info {
  const char * address;
  const char * client_id;
  const char * username;
  const char * password;
  const char * topic;
  const char * match;
  int keep_alive_interval;
  long time_out;
  int message_schematics;
  int persistance_type;
  char * const * server_uri;
};

struct mqtt_ssl_info {
  const char * trust_store;
  const char * key_store;
  const char * private_key;
  const char * private_key_password;
  const char * enabled_cipher_suites;
  int enable_server_cert_auth;
  int ssl_version;
  int verify;
  const char * ca_path;
  void * ssl_error_context;
};

xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (struct mqtt_info mqtt, iot_bus_t * iot_bus);
/* alloc with ssl options */
extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (iot_bus_t * sub, struct mqtt_info mqtt, struct mqtt_ssl_info mqtt_ssl, iot_data_sub_fn_t callback);

/* Push to a external messaging service for data export */
void push_topic (iot_data_t * data, void * self, const char * match);

extern void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t * state);

#endif //CUTILS_MQTT_H
