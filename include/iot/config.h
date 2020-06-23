//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_CONFIG_H_
#define _IOT_CONFIG_H_

/**
 * @file
 * @brief IOTech Config API
 */

#include "iot/data.h"
#include "iot/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get mandatory int64_t configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param val Pointer to int64_t configuration value to set if found
 * @param logger Logger used to log if configuration not found. If not set default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_i64 (const iot_data_t * map, const char * key, int64_t * val, iot_logger_t * logger);

/**
 * @brief Get mandatory string configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param val Pointer to string configuration value to set if found
 * @param alloc If true then allocate the returned string
 * @param logger Logger used to log if configuration not found. If not set default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_string (const iot_data_t * map, const char * key, const char ** val, bool alloc, iot_logger_t * logger);

/**
 * @brief Get mandatory double configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param val Pointer to double configuration value to set if found
 * @param logger Logger used to log if configuration not found. If not set default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_f64 (const iot_data_t * map, const char * key, double * val, iot_logger_t * logger);

/**
 * @brief Get mandatory boolean configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param val Pointer to bool configuration value to set if found
 * @param logger Logger used to log if configuration not found. If not set default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_bool (const iot_data_t * map, const char * key, bool * val, iot_logger_t * logger);

#ifdef __cplusplus
}
#endif
#endif
