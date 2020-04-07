//
// Copyright (c) 2019 IOTech
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

typedef char * (*iot_container_config_load_fn_t) (const char * name, void * from);

typedef struct iot_container_config_t
{
  iot_container_config_load_fn_t load;
  void * from;
} iot_container_config_t;


/**
 * @brief Allocate memory for a container
 *
 * The function to allocate memory for a container
 *
 * @return  Pointer to the created container
 */
extern iot_container_t * iot_container_alloc (void);

/**
 * @brief Initialise the container for a given configuration
 *
 * The function to initialise the container with a configuration provided
 *
 * @param cont  Pointer to the container to initialise
 * @param name  Name of the root configuration, that defines configuration of all the components
 * @param conf  Pointer to the config structure to load the JSON file from a specified location
 * @return      'true' if the container initialisation is successful
 */
extern bool iot_container_init (iot_container_t * cont, const char * name, iot_container_config_t * conf);

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
 * @param cont       Pointer to the container
 * @param ctype      Component type
 * @param cname      Component name
 * @param config     Configuration of the component to add
 */
extern void iot_container_add_comp (iot_container_t * cont, const char * ctype, const char *cname, const char * config);

/**
 * @brief Find a component factory within the container
 *
 * The function to find a component factory within the container
 *
 * @param cont  Pointer to the container
 * @param name  Name of the component factory to find
 * @return      Pointer to the component factory if found within the container, NULL otherwise
 */
extern iot_component_t * iot_container_find (iot_container_t * cont, const char * name);

/**
 * @brief Free a component from the container
 *
 * Release the resources used by a component and delete from the container.
 * Attempts to remove a component not in a container is ignored.
 *
 * @param cont   Pointer to the container
 * @param cname  Component name
 */
extern void iot_container_rm_comp (iot_container_t * cont, const char * cname);

/**
 * @brief Start a component
 *
 * @param cont  Pointer to the container
 * @param name  Name of the component to start
 */

extern void iot_container_start_comp (iot_container_t * cont, const char * cname);

/**
 * @brief Stop a component
 *
 * @param cont  Pointer to the container
 * @param name  Name of the component to stop
 */

extern void iot_container_stop_comp (iot_container_t * cont, const char * name);

/**
 * @brief Reconfigure a component
 *
 * Reconfigure the existing component.
 *
 * @param cont    Pointer to a container
 * @param name    Name of the component to reconfigure
 * @param config
 */
extern void iot_container_configure_comp (iot_container_t * cont, const char * name, const char * config);

/**
 * @brief List the components within a container
 *
 * @param cont  Pointer to a container
 * @return      Map containing the component names with their component type and state
 */
extern iot_data_t * iot_container_ls_comp (iot_container_t * cont);

#ifdef __cplusplus
}
#endif
#endif
