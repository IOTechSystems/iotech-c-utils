//
// Copyright (c) 2018 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_SCHEDULER_H_
#define _IOT_SCHEDULER_H_

/**
 * @file
 * @brief IOTech Scheduler API
 */

#include "iot/threadpool.h"
#include "iot/component.h"
#include "iot/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_BILLION 1000000000ULL
#define IOT_MS_TO_NS(MILLISECONDS) (MILLISECONDS * 1000000)
#define IOT_SEC_TO_NS(SECONDS) (SECONDS * IOT_BILLION)
#define IOT_MIN_TO_NS(MINUTES) (MINUTES * IOT_BILLION * 60)

typedef struct iot_scheduler_t iot_scheduler_t;
typedef struct iot_schedule_t iot_schedule_t;
typedef void * (*iot_schedule_fn_t) (void * arg);

#define IOT_SCHEDULER_TYPE "IOT::Scheduler"

/**
 * @brief Allocate memory and initialise scheduler
 *
 * @code
 *
 *    iot_scheduler_t * myScheduler  = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
 *
 * @endcode
 *
 * @param  priority          The thread priority for running the scheduler, (not set if -1)
 * @param  affinity          The processor affinity for the scheduler (not set if less than zero)
 * @param  logger            logger, can be NULL
 * @return iot_scheduler_t   Pointer to the created scheduler, NULL on error
 */
extern iot_scheduler_t * iot_scheduler_alloc (int priority, int affinity, iot_logger_t * logger);

/**
 * @brief Increment the scheduler reference count
 *
 * @param scheduler  Pointer to the scheduler
 */
extern void iot_scheduler_add_ref (iot_scheduler_t * scheduler);

/**
 * @brief Get the thread pool associated to the scheduler
 *
 * @param  scheduler  Pointer to a scheduler
 * @return            Pointer to a thread pool associated with the scheduler
 */
extern iot_threadpool_t * iot_scheduler_thread_pool (iot_scheduler_t * scheduler);

/**
 * @brief  Start scheduler and set the component state to IOT_COMPONENT_RUNNING
 *
 * @code
 *
 *    iot_scheduler_start (myScheduler);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler
 * @return            'true' on successfully starting the scheduler, 'false' otherwise
 */
extern bool iot_scheduler_start (iot_scheduler_t * scheduler);


/**
 * @brief  Create a new schedule
 *
 * @code
 *
 *    iot_schedule_t * mySchedule = iot_schedule_create (sched, func, NULL,IOT_SEC_TO_NS(1),0,0);
 *
 * @endcode
 *
 * @param  schd               Pointer to a scheduler
 * @param  function           The function that should be called when the schedule is triggered
 * @param  arg                The argument to be passed to the function
 * @param  period             The period of the schedule (in nanoseconds)
 * @param  start              The start time of the schedule (in nanoseconds)
 * @param  repeat             The number of times the schedule should repeat, (0 = infinite)
 * @param  pool               The thread pool used to run the schedule
 * @param  priority           The thread priority for running the schedule, (not set if -1)
 * @return iot_schedule       Pointer to the created schedule, NULL on error
 */
extern iot_schedule_t * iot_schedule_create
  (iot_scheduler_t * schd, iot_schedule_fn_t func, void * arg, uint64_t period, uint64_t start, uint64_t repeat, iot_threadpool_t * pool, int priority);

/**
 * @brief  Add a schedule to the queue
 *
 * @code
 *
 *    bool return = iot_schedule_add(myScheduler, mySchedule);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler
 * @param  schedule   Pointer to the schedule to be added
 * @return            'true' on success, 'false' on error
 */
extern bool iot_schedule_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule);

/**
 * @brief  Remove a schedule from the queue
 *
 * @code
 *
 *    bool return = iot_schedule_remove (myScheduler, mySchedule);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler
 * @param  schedule   Pointer to the schedule to be removed from the queue
 * @return            'true' on success, 'false' on error
 */
extern bool iot_schedule_remove (iot_scheduler_t * scheduler, iot_schedule_t * schedule);

/**
 * @brief  Delete a schedule
 *
 * @code
 *
 *    iot_schedule_delete (myScheduler, mySchedule);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler
 * @param  schedule   Pointer to the schedule to be deleted.
 */
extern void iot_schedule_delete (iot_scheduler_t * scheduler, iot_schedule_t * schedule);


/**
 * @brief  Stops execution of the scheduler and set the scheduler state to IOT_COMPONENT_STOPPED
 *
 * @code
 *
 *    iot_scheduler_t_stop (myScheduler);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler
 */
extern void iot_scheduler_stop (iot_scheduler_t * scheduler);


/**
 * @brief  Free resources used by the scheduler, only if it is the last reference i.e reference count <= 1
 *
 * @code
 *
 *    iot_scheduler_t_free (myScheduler);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler.
 */
extern void iot_scheduler_free (iot_scheduler_t * scheduler);


/**
 * @brief  Create Scheduler component factory
 *
 * @return  Pointer to Scheduler component factory
 *
 */
extern const iot_component_factory_t * iot_scheduler_factory (void);

#ifdef __cplusplus
}
#endif
#endif
