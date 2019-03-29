//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_CONTAINER_H_
#define _IOT_CONTAINER_H_

#include "iot/component.h"

typedef const char * (*iot_container_config_load_fn_t) (const char * name);

extern iot_container_t * iot_container_alloc (iot_container_config_load_fn_t loader);
extern bool iot_container_init (iot_container_t * cont, const char * name);
extern bool iot_container_start (iot_container_t * cont);
extern void iot_container_stop (iot_container_t * cont);
extern void iot_container_free (iot_container_t * cont);

extern void iot_container_add_factory (iot_container_t * cont, const iot_component_factory_t * factory);
extern iot_component_t * iot_container_find (iot_container_t * cont, const char * name);

#endif
