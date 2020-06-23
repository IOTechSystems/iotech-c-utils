//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/iot.h"

#define MY_COMPONENT_TYPE "IOT::MyComponent"

typedef struct my_component_t my_component_t;

/* Basic exported component functions */

extern my_component_t * my_component_alloc (iot_logger_t * logger);
extern void my_component_add_ref (my_component_t * comp);
extern void my_component_free (my_component_t * comp);
extern void my_component_start (my_component_t * comp);
extern void my_component_stop (my_component_t * comp);
extern const iot_component_factory_t * my_component_factory (void);
