//
// Copyright (c) 2023 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_QUEUE_H_
#define _IOT_QUEUE_H_

/**
 * @file
 * @brief IOTech Queue API
 */

#include "iot/data.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Alias for queue structure */
typedef struct iot_queue_t iot_queue_t;

/**
 * @brief Allocate memory and initialise queue
 *
 * @param  maxsize      Maximum number of elements to be held in the queue. 0 for no limit
 * @return iot_queue_t  Pointer to the created queue
 */
extern iot_queue_t *iot_queue_alloc (uint32_t maxsize);

/**
 * @brief Unblock all threads waiting to enqueue or dequeue elements
 *
 * @code
 *
 *   myApp->running = false;
 *   iot_queue_stop (myApp->queue);
 *   pthread_join (myApp->processThread, NULL);
 *   iot_queue_free (myApp->queue);
 *
 * @endcode
 *
 * @param q  Pointer to a queue
 */
extern void iot_queue_stop (iot_queue_t *q);

/**
 * @brief Free resources used by the queue
 * @param q  Pointer to a queue
 */
void iot_queue_free (iot_queue_t *q);

/**
 * @brief Add an element to the queue. If the queue is full, block until space is available
 * @param q       Pointer to a queue
 * @param element The element to add to the queue
 */
void iot_queue_enqueue (iot_queue_t *q, iot_data_t *element);

/**
 * @brief Add an element to the queue. If the queue is full, return false
 * @param q       Pointer to a queue
 * @param element The element to add to the queue
 * @return bool   Whether the operation was successful
 */
bool iot_queue_try_enqueue (iot_queue_t *q, iot_data_t *element);

/**
 * @brief Take an element from the queue. If the queue is empty, wait until an element is enqueued
 * @param  q          Pointer to a queue
 * @return iot_data_t Pointer to the dequeued element, or NULL if iot_queue_stop was called while we were waiting
 */
iot_data_t *iot_queue_dequeue (iot_queue_t *q);

/**
 * @brief Take an element from the queue, if there is one
 * @param  q          Pointer to a queue
 * @return iot_data_t Pointer to the dequeued element, or NULL if the queue was empty
 */
iot_data_t *iot_queue_try_dequeue (iot_queue_t *q);

/**
 * @brief Find the queue size
 * @param  q         Pointer to a queue
 * @return uint32_t  The number of elements in the queue
 */
uint32_t iot_queue_size (const iot_queue_t *q);

/**
 * @brief Find the queue size limit
 * @param q          Pointer to a queue
 * @return uint32_t  The maximum number of elements
 */
uint32_t iot_queue_maxsize (const iot_queue_t *q);

/**
 * @brief Set the queue size limit.
 * If there are more elements currently in the queue, subsequent enqueue operations will be blocked
 * until the queue size has been reduced to less than the new maximum
 * @param q        Pointer to a queue
 * @param maxsize  The maximum number of elements to be allowed before blocking new additions, or 0 for unlimited
 */
void iot_queue_setmaxsize (iot_queue_t *q, uint32_t maxsize);

#ifdef __cplusplus
}
#endif
#endif
