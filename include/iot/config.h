//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_CONFIG_H_
#define _IOT_CONFIG_H_

/**
 * @file
 * @brief IOTech Config support API
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
 * @param logger Logger used to log if configuration not found. If not set, default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_i64 (const iot_data_t * map, const char * key, int64_t * val, iot_logger_t * logger);

/**
 * @brief Get mandatory string configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param alloc If true then allocate the returned string
 * @param logger Logger used to log if configuration not found. If not set, default logger is used.
 * @return Returned string value or NULL if configuration was not resolved
 */
extern const char * iot_config_string (const iot_data_t * map, const char * key, bool alloc, iot_logger_t * logger);

/**
 * @brief Get mandatory configuration or default value if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param def Default string returned if key could not be resolved
 * @param alloc If true then allocate the returned string
 * @return Returned string value or the default if the key was not resolved
 */
extern const char * iot_config_string_default (const iot_data_t * map, const char * key, const char * def, bool alloc);

/**
 * @brief Get mandatory double configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param val Pointer to double configuration value to set if found
 * @param logger Logger used to log if configuration not found. If not set, default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_f64 (const iot_data_t * map, const char * key, double * val, iot_logger_t * logger);

/**
 * @brief Get mandatory boolean configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param val Pointer to bool configuration value to set if found
 * @param logger Logger used to log if configuration not found. If not set, default logger is used.
 * @return Whether configuration value was resolved
 */
extern bool iot_config_bool (const iot_data_t * map, const char * key, bool * val, iot_logger_t * logger);

/**
 * @brief Get mandatory map configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param logger Logger used to log if configuration not found. If not set, default logger is used.
 * @return Returned map or NULL if name could not be found in map or named map entry not a map.
 */
extern const iot_data_t * iot_config_map (const iot_data_t * map, const char * key, iot_logger_t * logger);

/**
 * @brief Get mandatory vector configuration value and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param logger Logger used to log if configuration not found. If not set, default logger is used.
 * @return Returned vector or NULL if name could not be found in map or named map entry not a vector.
 */
extern const iot_data_t * iot_config_vector (const iot_data_t * map, const char * key, iot_logger_t * logger);

/**
 * @brief Get a mandatory component configuration and log error if cannot be resolved.
 *
 * @param map Configuration map
 * @param key Configuration key
 * @param container Pointer to the container from which to find the component
 * @param logger Logger used to log if component not found. If not set, default logger is used.
 * @return Returned component or NULL if component name or component could not be resolved
 */
extern iot_component_t * iot_config_component (const iot_data_t * map, const char * key, iot_container_t * container, iot_logger_t * logger);

#ifdef __cplusplus
}
#endif
#endif
