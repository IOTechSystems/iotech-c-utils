//
// Copyright (c) 2019-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_THREAD_H_
#define _IOT_THREAD_H_

/**
 * @file
 * @brief IOTech Thread API
 */

#include "iot/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Thread with no affinity has affinity level of -1 */
#define IOT_THREAD_NO_AFFINITY -1
/** Thread with no priority has priority level of -1 */
#define IOT_THREAD_NO_PRIORITY -1

/** Alias for thread function pointer */
typedef void * (*iot_thread_fn_t) (void * arg);

/**
 * @brief Create new thread
 *
 * @param tid      Thread ID
 * @param func     Start the routine to which 'func' points
 * @param arg      The routine started is passed 'arg'
 * @param priority Priority of thread
 * @param affinity Affinity of thread
 * @param logger   Logger, can be NULL
 * @return         'true' if thread created successfully, 'false' if not
 */
extern bool iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity, iot_logger_t * logger);

/**
 * @brief Get current threads priority
 *
 * @return Current threads priority level
 */
extern int iot_thread_current_get_priority (void);

/**
 * @brief Set priority of current thread
 *
 * @param priority Priority level we wish to assign to current thread
 * @return         'true' if priority set, 'false' if not
 */
extern bool iot_thread_current_set_priority (int priority);

/**
 * @brief Get priority of specified thread
 *
 * @param thread Specified thread
 * @return       Priority level of specified thread
 */
extern int iot_thread_get_priority (pthread_t thread);

/**
 * @brief Set priority of specified thread
 *
 * @param thread   Specified thread
 * @param priority Priority level we wish to assign to specified thread
 * @return         'true' if priority set, 'false' if not
 */
extern bool iot_thread_set_priority (pthread_t thread, int priority);

/**
 * @brief Check if a threads priority level is valid
 *
 * @param priority Priority level to check
 * @return         'true' if valid, 'false' if not
 */
extern bool iot_thread_priority_valid (int priority);

/**
 * @brief Initialise specified mutex
 *
 * @param mutex Mutex to initialise
 */
extern void iot_mutex_init (pthread_mutex_t * mutex);


#ifdef __cplusplus
}
#endif
#endif
