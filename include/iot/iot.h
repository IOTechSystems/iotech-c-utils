//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_IOT_H_
#define _IOT_IOT_H_

#include "iot/data.h"
#include "iot/component.h"
#include "iot/container.h"
#include "iot/logger.h"
#include "iot/thread.h"
#include "iot/json.h"
#include "iot/scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void iot_init (void);
extern void iot_fini (void);

#ifdef __cplusplus
}
#endif
#endif
