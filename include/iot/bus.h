//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_BUS_H_
#define _IOT_BUS_H_

#include "iot/data.h"
#include "iot/scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_BUS_TYPE "IOT::Bus"

/* Types */

typedef struct iot_bus_t iot_bus_t;
typedef struct iot_bus_sub_t iot_bus_sub_t;
typedef struct iot_bus_pub_t iot_bus_pub_t;
typedef iot_data_t * (*iot_data_pub_cb_fn_t) (void * self);
typedef void (*iot_data_sub_fn_t) (iot_data_t * data, void * self, const char * match);

/* Lifecycle operations */

extern iot_bus_t * iot_bus_alloc (iot_scheduler_t * scheduler, uint64_t default_poll_interval);
extern bool iot_bus_start (iot_bus_t * bus);
extern void iot_bus_stop (iot_bus_t * bus);
extern void iot_bus_free (iot_bus_t * bus);

/* Topics */

extern void iot_bus_topic_create (iot_bus_t * bus, const char * name, const int * prio);

/* Data subscription */

extern iot_bus_sub_t * iot_bus_sub_alloc (iot_bus_t * bus, void * self, iot_data_sub_fn_t callback, const char * pattern);
extern void iot_bus_sub_free (iot_bus_sub_t * sub);

/* Data publication */

extern iot_bus_pub_t * iot_bus_pub_alloc (iot_bus_t * bus, void * self, iot_data_pub_cb_fn_t callback, const char * topic);
extern void iot_bus_publish (iot_bus_pub_t * pub, iot_data_t * data, bool sync);
extern void iot_bus_pub_free  (iot_bus_pub_t * pub);

/* Component factory */

extern const iot_component_factory_t * iot_bus_factory (void);

#ifdef __cplusplus
}
#endif
#endif