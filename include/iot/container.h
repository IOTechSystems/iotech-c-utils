//
// Copyright (c) 2019-2020 IOTech Ltd
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

/** Alias for container configuration load function pointer */
typedef char * (*iot_container_config_load_fn_t) (const char * name, const char * uri);

/**
 * Alias for container configuration structure
 */
typedef struct iot_container_config_t
{
  iot_container_config_load_fn_t load;  /**< Pointer to function that handles container configuration load functionality */
  const char * uri;                     /**< Pointer to a string which identifies the source of configuration */
} iot_container_config_t;

/**
 * @brief Initialise the configuration resolver for containers and components
 *
 * The function sets the global configuration resolver data for containers and components
 *
 * @param conf  Pointer to the config structure to load JSON configurations from a specified location
 */
extern void iot_container_config (iot_container_config_t * conf);

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
 * @brief Find a named container
 *
 * The function to find a named container
 *
 * @param name  Name of the container
 * @return  Pointer container if found, NULL otherwise
 */
extern iot_container_t * iot_container_find (const char * name);

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
 * The function that invokes start function of the components defined within the container
 *
 * @param cont  Pointer to the container
 * @return      'true' if all the components within the container is started successfully, 'false' otherwise
 */
extern bool iot_container_start (iot_container_t * cont);

/**
 * @brief Stop the components within the container
 *
 * The function that invokes stop function of the components defined within the container
 *
 * @param cont  Pointer to the container
 */
extern void iot_container_stop (iot_container_t * cont);

/**
 * @brief Destroy the container and the associated components
 *
 * The function that invokes free function of the components defined within the container to release the resources
 *
 * @param cont  Pointer to the container
 */
extern void iot_container_free (iot_container_t * cont);

/**
 * @brief Add component factories to the container
 *
 * The function to add component factories to the container
 *
 * @param cont     Pointer to the container to hold component factories
 * @param factory  Pointer to the component factory to add
 */
extern void iot_container_add_factory (iot_container_t * cont, const iot_component_factory_t * factory);

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
 * The function to find a component factory within the container
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
extern void iot_container_rm_component (iot_container_t * cont, const char * name);

/**
 * @brief List the components within a container
 *
 * @param cont  Pointer to a container
 * @return      Pointer to an array of structure that contain component name, type and state
 */
extern iot_component_info_t * iot_container_ls_component (iot_container_t * cont);

/**
 * @brief List the names of the containers
 * @return  Map containing the names of the containers
 */
extern iot_data_t * iot_container_ls_containers (void);

#ifdef __cplusplus
}
#endif
#endif
