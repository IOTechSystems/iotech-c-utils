//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_COMPONENT_H_
#define _IOT_COMPONENT_H_

#include "iot/data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iot_component_t iot_component_t;
typedef struct iot_container_t iot_container_t;
typedef struct iot_component_factory_t iot_component_factory_t;

typedef enum
{
  IOT_COMPONENT_INITIAL = 0u,
  IOT_COMPONENT_STOPPED = 1u,
  IOT_COMPONENT_RUNNING = 2u,
  IOT_COMPONENT_DELETED = 3u
}
iot_component_state_t;

typedef iot_component_t * (*iot_component_config_fn_t) (iot_container_t * cont, const iot_data_t * map);
typedef bool (*iot_component_start_fn_t) (iot_component_t * this);
typedef void (*iot_component_stop_fn_t) (iot_component_t * this);
typedef void (*iot_component_free_fn_t) (iot_component_t * this);

struct iot_component_t
{
  volatile iot_component_state_t state;
  iot_component_start_fn_t start_fn;
  iot_component_stop_fn_t stop_fn;
  atomic_uint_fast32_t refs;
};

struct iot_component_factory_t
{
  const char * const type;
  iot_component_config_fn_t config_fn;
  iot_component_free_fn_t free_fn;
};

extern void iot_component_init (iot_component_t * component, iot_component_start_fn_t start, iot_component_stop_fn_t stop);
extern void iot_component_add_ref (iot_component_t * component);
extern bool iot_component_free (iot_component_t * component);

#ifdef __cplusplus
}
#endif
#endif
