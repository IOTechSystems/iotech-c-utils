//
// Copyright (c) 2019-2020 IOTech Ltd
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

/** Alias for component structure */
typedef struct iot_component_t iot_component_t;
/** Alias for container structure */
typedef struct iot_container_t iot_container_t;
/** Alias for component factory structure */
typedef struct iot_component_factory_t iot_component_factory_t;

/**
 * Component state enumeration
 */
typedef enum
{
  IOT_COMPONENT_INITIAL = 0U,  /**< Initial component state */
  IOT_COMPONENT_STOPPED = 1U,  /**< Stopped component state */
  IOT_COMPONENT_RUNNING = 2U,  /**< Running component state */
  IOT_COMPONENT_DELETED = 4U,  /**< Deleted component state */
  IOT_COMPONENT_STARTING = 8U  /**< Starting transient component state */
} iot_component_state_t;

/**
 * Component data struct
 */
typedef struct iot_component_data_t
{
  struct iot_component_data_t * next; /**< Pointer to next component data struct in list */
  char * name;                        /**< The component name */
  char * type;                        /**< The component type name */
  iot_component_state_t state;        /**< The component state */
} iot_component_data_t;

/**
 * Component info struct
 */
typedef struct iot_component_info_t
{
  uint32_t count;                 /**< The number of component data elements */
  iot_component_data_t * data;    /**< List of component data */
} iot_component_info_t;

/** Alias for component configuration function pointer */
typedef iot_component_t * (*iot_component_config_fn_t) (iot_container_t * cont, const iot_data_t * map);
/** Alias for component reconfiguration function pointer */
typedef bool (*iot_component_reconfig_fn_t) (iot_component_t * comp, iot_container_t * cont, const iot_data_t * map);
/** Alias for component start function pointer */
typedef void (*iot_component_start_fn_t) (iot_component_t * comp);
/** Alias for component stop function pointer */
typedef void (*iot_component_stop_fn_t) (iot_component_t * comp);
/** Alias for component free function pointer */
typedef void (*iot_component_free_fn_t) (iot_component_t * comp);

/**
 * Component factory structure
 */
struct iot_component_factory_t
{
  const char * const type;                 /**< Indicates the type of a component */
  iot_component_config_fn_t config_fn;     /**< Pointer to function that handles component configuration */
  iot_component_free_fn_t free_fn;         /**< Pointer to function that handles the freeing of a component */
  iot_component_reconfig_fn_t reconfig_fn; /**< Pointer to function that handles component reconfiguration */
  const iot_component_factory_t * next;    /**< Pointer to next component factory structure */
};

/**
 * Component structure
 */
struct iot_component_t
{
  volatile iot_component_state_t state;     /**< Current state of component */
  pthread_mutex_t mutex;                    /**< Thread mutex settings of component */
  pthread_cond_t cond;                      /**< Thread condition settings of component */
  iot_component_start_fn_t start_fn;        /**< Pointer to function that handles starting a component */
  iot_component_stop_fn_t stop_fn;          /**< Pointer to function that handles stopping a component */
  atomic_uint_fast32_t refs;                /**< Current reference count of a given component */
  const iot_component_factory_t * factory;  /**< Pointer to component factory structure */
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
 * @brief Set the component state to IOT_COMPONENT_STARTING
 *
 * @param component  Pointer to the component
 * @return           'true', if the state transition occurred, 'false' otherwise
 */
extern bool iot_component_set_starting (iot_component_t * component);

/**
 * @brief Block until the component is in a given state
 *
 * The function blocks until the component is in one of a given set of states.
 *
 * @code
 *
 *   iot_component_state_t state = iot_component_wait (myComponent, IOT_COMPONENT_DELETED | IOT_COMPONENT_RUNNING);
 *
 * @endcode
 *
 * @param component  Pointer to the component
 * @param states     Logical OR of states on which to wait
 * @return           State of the component that resulted in unblocking (one of states)
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
 * @param type  Type name of the component factory to find
 * @return      Pointer to the component factory if found, NULL otherwise
 */
extern const iot_component_factory_t * iot_component_factory_find (const char * type);

/**
 * @brief Returns component state name
 *
 * The function returns a constant string for the component state enum
 *
 * @param state The component state
 * @return      Pointer to a constant string representing the state name
 */
extern const char * iot_component_state_name (iot_component_state_t state);

/**
 * @brief Frees a component info struct
 *
 * The function frees a component info struct
 *
 * @param info Pointer to the struct to be freed
 */
extern void iot_component_info_free (iot_component_info_t * info);

#ifdef __cplusplus
}
#endif
#endif
