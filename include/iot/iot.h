//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_IOT_H_
#define _IOT_IOT_H_

#include "iot/data.h"
#include "iot/time.h"
#include "iot/hash.h"
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

/**
 * @brief Load JSON configuration from file
 *
 * The function to load JSON configuration from a file
 *
 * @param name  Name of the configuration file
 * @param uri   Directory URI for configuration files
 * @return      JSON string loaded from the file
 */
extern char * iot_file_config_loader (const char * name, const char * uri);

#ifdef __cplusplus
}
#endif
#endif
