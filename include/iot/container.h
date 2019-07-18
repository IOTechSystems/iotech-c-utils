//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_CONTAINER_H_
#define _IOT_CONTAINER_H_

#include "iot/component.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef const char * (*iot_container_config_load_fn_t) (const char * name, void * from);

extern iot_container_t * iot_container_alloc (void);
extern bool iot_container_init (iot_container_t * cont, const char * name, iot_container_config_load_fn_t loader, void * from);
extern bool iot_container_start (iot_container_t * cont);
extern void iot_container_stop (iot_container_t * cont);
extern void iot_container_free (iot_container_t * cont);

extern void iot_container_add_factory (iot_container_t * cont, const iot_component_factory_t * factory);
extern iot_component_t * iot_container_find (iot_container_t * cont, const char * name);

#ifdef __cplusplus
}
#endif
#endif
