//
// Copyright (c) 2019-2022 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_CONTAINER_H_
#define _IOT_CONTAINER_H_

/**
 * @file
 * @brief IOTech Container API
 */

#include "iot/component.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Type for container configuration load function pointer */
typedef char * (*iot_container_config_load_fn_t) (const char * name, const char * uri);
/** TYpe for container configuration save function pointer */
typedef bool (*iot_container_config_save_fn_t) (const char * name, const char * uri, const char * config);

/**
 * Alias for container configuration structure
 */
typedef struct iot_container_config_t
{
  iot_container_config_load_fn_t load;  /**< Pointer to function that handles container configuration load functionality */
  const char * uri;                     /**< Configuration URI string (optional) */
  iot_container_config_save_fn_t save;  /**< Pointer to function that handles container configuration save functionality (optional) */
} iot_container_config_t;

/**
 * @brief Initialise the configuration resolver for containers and components
 *
 * The function sets the global configuration resolver data for containers and components
 *
 * @param conf  Pointer to the config structure used to load JSON configurations from a specified location
 */
extern void iot_container_config (const iot_container_config_t * conf);

/**
 * @brief Get the configuration resolver for containers and components
 *
 * The function returns the global configuration resolver data for containers and components
 *
 * @return Pointer to the config structure used to load JSON configurations from a specified location (NULL if not set)
 */
extern const iot_container_config_t * iot_container_get_config (void);

/**
 * @brief Allocate a named container
 *
 * The function to allocate memory for a container and set it's name
 *
 * @param name  Name of the container, must be unique
 * @return  Pointer to the created container
 */
extern iot_container_t * iot_container_alloc (const char * name);

/**
 * @brief Initialise the container for a given configuration
 *
 * The function to initialise the container with a configuration provided
 *
 * @param cont  Pointer to the container to initialise
 * @return      'true' if the container initialisation is successful
 */
extern bool iot_container_init (iot_container_t * cont);

/**
 *  @brief Start the components within the container
 *
 * The function that invokes the start function of the components defined within the container
 *
 * @param cont  Pointer to the container
 */
extern void iot_container_start (iot_container_t * cont);

/**
 * @brief Stop the components within the container
 *
 * The function that invokes the stop function of the components defined within the container
 *
 * @param cont  Pointer to the container
 */
extern void iot_container_stop (iot_container_t * cont);

/**
 * @brief Destroy the container and the associated components
 *
 * The function that invokes the free function of the components defined within the container to release the resources
 *
 * @param cont  Pointer to the container
 */
extern void iot_container_free (iot_container_t * cont);

/**
 * @brief Add component to the container
 *
 * The function to add a component to the container. If a component factory is not available, config should contain
 * the component library and factory to load them dynamically.
 *
 * @param cont      Pointer to the container
 * @param ctype     Component type
 * @param name      Component name
 * @param config    Configuration of the component to add
 */
extern void iot_container_add_component (iot_container_t * cont, const char * ctype, const char * name, const char * config);

/**
 * @brief Find a named component in a container
 *
 * The function finds a named component within the container
 *
 * @param cont  Pointer to the container
 * @param name  Name of the component to find
 * @return      Pointer to the component if found within the container, NULL otherwise
 */
extern iot_component_t * iot_container_find_component (iot_container_t * cont, const char * name);

/**
 * @brief Free a component from the container
 *
 * Release the resources used by a component and delete from the container.
 * Attempts to remove a component not in a container is ignored.
 *
 * @param cont  Pointer to the container
 * @param name  Component name
 */
extern void iot_container_delete_component (iot_container_t * cont, const char * name);

/**
 * @brief List state of all components
 *
 * @param cont     Pointer to a container
 * @param category Category name of component to list. If NULL all components are listed
 * @return         List of maps of component data, with keys "name", "type", "state" and "config"
 */
extern iot_data_t * iot_container_list_components (iot_container_t * cont, const char * category);

/**
 * @brief Get state of named component
 *
 * @param cont  Pointer to a container
 * @param name  Component name
 * @return      Data map, with keys "name", "type", "state" and "config"
 */
extern iot_data_t * iot_container_component_read (iot_container_t * cont, const char * name);

#ifdef __cplusplus
}
#endif
#endif
