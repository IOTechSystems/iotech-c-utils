//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_COMPONENT_H_
#define _IOT_COMPONENT_H_

#include "iot/data.h"

typedef struct iot_component_t iot_component_t;
typedef struct iot_container_t iot_container_t;
typedef struct iot_component_factory_t iot_component_factory_t;

typedef enum
{
  IOT_COMPONENT_INITIAL = 0,
  IOT_COMPONENT_STOPPED,
  IOT_COMPONENT_RUNNING,
  IOT_COMPONENT_DELETED
}
iot_component_state_t;

typedef iot_component_t * (*iot_component_config_fn_t) (iot_container_t * cont, const iot_data_t * map);
typedef bool (*iot_component_start_fn_t) (iot_component_t * this);
typedef void (*iot_component_stop_fn_t) (iot_component_t * this);
typedef void (*iot_component_free_fn_t) (iot_component_t * this);

struct iot_component_t
{
  iot_component_state_t state;
  iot_component_start_fn_t start_fn;
  iot_component_stop_fn_t stop_fn;
};

struct iot_component_factory_t
{
  const char * const type;
  iot_component_config_fn_t config_fn;
  iot_component_free_fn_t free_fn;
};

#endif
