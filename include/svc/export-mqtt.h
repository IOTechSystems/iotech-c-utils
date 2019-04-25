//
// Created by jordan on 25/04/19.
//

#include <iot/bus.h>

#ifndef CUTILS_MQTT_H
#define CUTILS_EXPORT_MQTT_H
#define MAX_RETRIES 5
#define CONNECT_TIMEOUT 500

typedef struct xrt_mqtt_exporter_t xrt_mqtt_exporter_t;

xrt_mqtt_exporter_t * xrt_mqtt_exporter_alloc (  iot_bus_t *pubsub,  const char *match, const char *address, const char *client_id, const char * username, const char * password, const char *topic, const int keept_alive_interval, const long time_out,
                                                 const int message_schematics, const int persistance_type, char* const* server_uri);

#endif //CUTILS_MQTT_H
