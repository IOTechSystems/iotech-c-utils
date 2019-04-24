#ifndef _IOT_THREADPOOL_H_
#define _IOT_THREADPOOL_H_

#include "iot/component.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_THREADPOOL_TYPE "IOT::ThreadPool"

typedef struct iot_threadpool_t iot_threadpool_t;

/**
 * @brief Allocate thread pool
 *
 * Initializes a thread pool. This function does not return until all
 * threads have initialized successfully.
 *
 * @param num_threads        number of threads to be created in the threadpool
 * @param max_jobs           maximum number of jobs to queue (before blocking)
 * @param default_prio       default priority for created threads
 * @return iot_threadpool_t  created thread pool on success, NULL on error
 */
extern iot_threadpool_t * iot_threadpool_alloc (uint32_t num_threads, uint32_t max_jobs, const int * default_prio);

/**
 * @brief Add work to the thread pool
 *
 * Takes a function pointer and it's argument and adds it to the thread pool's job queue.
 * This function blocks it the maximum number of queued jobs would be excceded.
 *
 * @param  pool          pool to which the work will be added
 * @param  function      function to add as work
 * @param  arg           function argument
 * @param  priority      priority to run thread at (not set if NULL)
 */
extern void iot_threadpool_add_work (iot_threadpool_t * pool, void (*function) (void*), void * arg, const int * priority);

/**
 * @brief Try to add work to the thread pool
 *
 * Takes a function pointer and it's argument and adds it to the thread pool's job queue.
 * This function never blocks and returns whether the work was added. Work is not added to
 * the thread pool if the maximum number of queued jobs would be exceeded.
 *
 * @param  pool          pool to which the work will be added
 * @param  function      function to add as work
 * @param  arg           function argument
 * @param  priority      priority to run thread at (not set if NULL)
 * @returns bool         whether the work was added
 */
extern bool iot_threadpool_try_work (iot_threadpool_t * pool, void (*function) (void*), void * arg, const int * priority);

/**
 * @brief Wait for all queued jobs to finish
 *
 * Will wait for all jobs - both queued and currently running to finish.
 *
 * @param iot_threadpool the thread pool to wait for
 * @return nothing
 */
extern void iot_threadpool_wait (iot_threadpool_t * pool);

/**
 * @brief Start the thread pool
 *
 * This will start the thread pool handling jobs.
 *
 * @param pool the pool to start
 */
extern bool iot_threadpool_start (iot_threadpool_t * pool);

/**
 * @brief Stop the thread pool
 *
 * This will stop the thread pool handling jobs.
 *
 * @param pool the pool to stop
 */
extern void iot_threadpool_stop (iot_threadpool_t * pool);

/**
 * @brief Destroy the thread pool
 *
 * This will wait for the currently active threads to finish then
 * frees the thread pool.
 *
 * @param pool the pool to destroy
 */
extern void iot_threadpool_free (iot_threadpool_t * pool);

/**
 * @brief Show currently working threads
 *
 * @param pool       the pool of interest
 * @return uint32_t  the number of threads working
 */
extern uint32_t iot_threadpool_num_threads_working (iot_threadpool_t * pool);

/* Threadpool factory */

extern const iot_component_factory_t * iot_threadpool_factory (void);

#ifdef __cplusplus
}
#endif
#endif
