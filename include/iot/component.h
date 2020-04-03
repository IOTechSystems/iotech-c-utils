//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_COMPONENT_H_
#define _IOT_COMPONENT_H_

/**
 * @file
 * @brief IOTech Component API
 */

#include "iot/data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iot_component_t iot_component_t;
typedef struct iot_container_t iot_container_t;
typedef struct iot_component_factory_t iot_component_factory_t;

typedef enum
{
  IOT_COMPONENT_INITIAL = 0U,
  IOT_COMPONENT_STOPPED = 1U,
  IOT_COMPONENT_RUNNING = 2U,
  IOT_COMPONENT_DELETED = 4U
}
iot_component_state_t;

typedef iot_component_t * (*iot_component_config_fn_t) (iot_container_t * cont, const iot_data_t * map);
typedef bool (*iot_component_reconfig_fn_t) (iot_component_t * comp, iot_container_t * cont, const iot_data_t * map);
typedef bool (*iot_component_start_fn_t) (iot_component_t * comp);
typedef void (*iot_component_stop_fn_t) (iot_component_t * comp);
typedef void (*iot_component_free_fn_t) (iot_component_t * comp);

struct iot_component_factory_t
{
  const char * const type;
  iot_component_config_fn_t config_fn;
  iot_component_free_fn_t free_fn;
  iot_component_reconfig_fn_t reconfig_fn;
  const iot_component_factory_t * next;
};

struct iot_component_t
{
  volatile iot_component_state_t state;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  iot_component_start_fn_t start_fn;
  iot_component_stop_fn_t stop_fn;
  atomic_uint_fast32_t refs;
  const iot_component_factory_t * factory;
};

/**
 * @brief Initialise component
 *
 * The function to initialise the component
 *
 * @param component  Pointer to the component
 * @param factory    Function pointer to the component factory
 * @param start      Function pointer to the component start function
 * @param stop       Function pointer to the component stop function
 */
extern void iot_component_init (iot_component_t * component, const iot_component_factory_t * factory, iot_component_start_fn_t start, iot_component_stop_fn_t stop);

/**
 * @brief Reconfigure component
 *
 * The function to reconfigure a component, if supported. Reconfiguration of parameters depends on the support available by the component
 * @param component  Pointer to the component
 * @param cont       Pointer to the container that holds the component
 * @param map        Reconfigurable parameters in a map
 * @return           'true' if the component support reconfiguration and able to update the configuration, 'false' otherwise
 */
extern bool iot_component_reconfig (iot_component_t * component, iot_container_t * cont, const iot_data_t * map);

/**
 * @brief Increment the component reference count
 *
 * The function to increment the component reference count
 *
 * @param component  Pointer to the component
 */
extern void iot_component_add_ref (iot_component_t * component);

/**
 * @brief Decrement the component reference count
 *
 * The function to decrement the component reference count
 *
 * @param component Pointer to the component
 * @return          'true', if the reference count of the component is <=1, 'false' otherwise
 */
extern bool iot_component_dec_ref (iot_component_t * component);

/**
 * @brief Free the resources used by the component
 *
 * The function to release the resources used by the component
 *
 * @param component  Pointer to the component
 */
extern void iot_component_fini (iot_component_t * component);

/**
 * @brief Set the component state to IOT_COMPONENT_RUNNING
 *
 * @param component  Pointer to the component
 * @return           'true', if the state transition occurred, 'false' otherwise
 */
extern bool iot_component_set_running (iot_component_t * component);

/**
 * @brief Set the component state to IOT_COMPONENT_STOPPED
 *
 * @param component  Pointer to the component
 * @return           'true', if the state transition occurred, 'false' otherwise
 */
extern bool iot_component_set_stopped (iot_component_t * component);

/**
 * @brief Set the component state to IOT_COMPONENT_DELETED
 *
 * @param component  Pointer to the component
 * @return           'true', if the state transition occurred, 'false' otherwise
 */
extern bool iot_component_set_deleted (iot_component_t * component);

/**
 * @brief Block until the component state is changed to states
 *
 * The function that acquires a lock and blocks the calling thread for the component until the state does not change to state(s) provided.
 * This function releases the lock before returning
 *
 * @code
 *
 *   iot_component_state_t state = iot_component_wait (myComponent, IOT_COMPONENT_DELETED | IOT_COMPONENT_RUNNING);
 *
 * @endcode
 *
 * @param component  Pointer to the component
 * @param states     Component state(s) to check for unblocking
 * @return           State of the component that resulted in unblocking
 */
extern iot_component_state_t iot_component_wait  (iot_component_t * component, uint32_t states);

/**
 * @brief Block until the component state is changed to states
 *
 * The function that acquires a lock and blocks the calling thread for the component until the state does not change to state(s) provided.
 * The caller must release the mutex
 *
 * @code
 *
 *   iot_component_state_t state = iot_component_wait_and_lock (myComponent, IOT_COMPONENT_DELETED | IOT_COMPONENT_RUNNING);
 *
 * @endcode
 *
 * @param component  Pointer to the component
 * @param states     Component state(s) to check for unblocking
 * @return           State of the component that resulted in unblocking
 */
extern iot_component_state_t iot_component_wait_and_lock  (iot_component_t * component, uint32_t states);

/**
 * @brief Acquire lock on the component
 *
 * @param component  Pointer to the component
 * @return           Current state of the component
 */
extern iot_component_state_t iot_component_lock (iot_component_t * component);

/**
 * @brief Release lock held by the component
 *
 * @param component  Pointer to the component
 * @return           Current state of the component
 */
extern iot_component_state_t iot_component_unlock (iot_component_t * component);

/**
 * @brief Load JSON configuration from file
 *
 * The function to load JSON configuration from a file
 *
 * @param name  Name of the root configuration file
 * @param from  Location at which the configuration file is available
 * @return      JSON string loaded from the file
 */
extern char * iot_component_file_config_loader (const char * name, void * from);

/**
 * @brief Add a component factory
 *
 * The function adds a component factory allowing containers to manage instances of the associated type.
 * Factory type names must be unique. Attempts to add a factory with an existing type name are ignored.
 *
 * @param factory  Pointer to the component factory to add
 */
extern void iot_component_factory_add (const iot_component_factory_t * factory);

/**
 * @brief Find a component factory for a type
 *
 * The function finds a component factory by type name. NULL is returned if the factory
 * cannot be found.
 *
 * @param name  Type name of the component factory to find
 * @return      Pointer to the component factory if found, NULL otherwise
 */
extern const iot_component_factory_t * iot_component_factory_find (const char * type);

#ifdef __cplusplus
}
#endif
#endif
