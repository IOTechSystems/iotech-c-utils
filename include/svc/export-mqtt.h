#include <iot/bus.h>

#ifndef CUTILS_MQTT_H
#define CUTILS_EXPORT_MQTT_H
#define MAX_RETRIES 5
#define CONNECT_TIMEOUT 500

typedef struct xrt_mqtt_exporter_t xrt_mqtt_exporter_t;
struct mqtt_info mqtt_info;
struct mqtt_ssl_info mqtt_ssl_info;

extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (  iot_bus_t *pubsub,  const char *match, const char *address, const char *client_id, const char * username, const char * password, const char *topic, const int keept_alive_interval, const long time_out,
  int message_schematics, int persistance_type, char* const* server_uri);

extern xrt_mqtt_exporter_t * xrt_mqtt_exporter_ssl_alloc (iot_bus_t *pubsub, struct mqtt_info, struct mqtt_ssl_info);

extern void xrt_mqtt_exporter_free (xrt_mqtt_exporter_t * state);

#endif //CUTILS_MQTT_H
