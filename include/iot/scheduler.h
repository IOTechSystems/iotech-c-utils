//
// Copyright (c) 2018-2020 IOTech Ltd
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

/** Object-like macro - Defines 1,000,000,000 */
#define IOT_BILLION 1000000000ULL
/** Object-like macro - Defines 1,000,000 */
#define IOT_MILLION 1000000ULL
/** Function-like macro - Conversion from milliseconds to nanoseconds */
#define IOT_MS_TO_NS(m) ((m) * IOT_MILLION)
/** Function-like macro - Conversion from seconds to nanoseconds */
#define IOT_SEC_TO_NS(s) ((s) * IOT_BILLION)
/** Function-like macro - Conversion from minutes to nanoseconds */
#define IOT_MIN_TO_NS(m) (IOT_SEC_TO_NS ((m) * 60))
/** Function-like macro - Conversion from hours to nanoseconds */
#define IOT_HOUR_TO_NS(h) (IOT_MIN_TO_NS ((h) * 60))

/** Alias for scheduler structure */
typedef struct iot_scheduler_t iot_scheduler_t;
/** Alias for schedule structure */
typedef struct iot_schedule_t iot_schedule_t;
/** Alias for schedule function pointer */
typedef void * (*iot_schedule_fn_t) (void * arg);
/** Alias for schedule free function pointer */
typedef void (*iot_schedule_free_fn_t) (void * arg);

/** Scheduler component name */
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
 */
extern void iot_scheduler_start (iot_scheduler_t * scheduler);

/**
 * @brief  Create a new schedule
 *
 * @code
 *
 *    iot_schedule_t * mySchedule = iot_schedule_create (sched, func, NULL, NULL, IOT_SEC_TO_NS(1),0,0);
 *
 * @endcode
 *
 * @param  schd               Pointer to a scheduler
 * @param  func               The function that should be called when the schedule is triggered
 * @param  free_func          The function to free the arg parameter when scheduler is deleted, could be NULL
 * @param  arg                The argument to be passed to the function
 * @param  period             The period of the schedule (in nanoseconds)
 * @param  start              The time to wait until the schedule is first triggered (in nanoseconds)
 * @param  repeat             The number of times the schedule should repeat, (0 = infinite)
 * @param  pool               The thread pool used to run the schedule
 * @param  priority           The thread priority for running the schedule, (not set if -1)
 * @return iot_schedule       Pointer to the created schedule, NULL on error
 */
extern iot_schedule_t * iot_schedule_create
  (iot_scheduler_t * schd, iot_schedule_fn_t func, iot_schedule_free_fn_t free_func, void * arg, uint64_t period, uint64_t start, uint64_t repeat, iot_threadpool_t * pool, int priority);

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
 * @brief  Reset a schedule to start from the current time
 *
 * @code
 *
 *    iot_schedule_reset (myScheduler, mySchedule);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler
 * @param  schedule   Pointer to the schedule to be reset
 */
extern void iot_schedule_reset (iot_scheduler_t * scheduler, iot_schedule_t * schedule);
/**
 * @brief  Add callback function to be invoked when a schedule is run
 *
 * @code
 *
 *    iot_schedule_add_run_callback (myScheduler, mySchedule, myFunc);
 *
 * @endcode
 *
 * @param   scheduler  Pointer to a scheduler
 * @param   schedule   Pointer to the run callback function
 * @param   func       Function to be invoked. A NULL value implies no callback.
 */
extern void iot_schedule_add_run_callback (iot_scheduler_t * scheduler, iot_schedule_t * schedule, iot_schedule_fn_t func);

/**
 * @brief  Add callback function to be invoked when a schedule run is aborted
 *
 * @code
 *
 *    iot_schedule_add_abort_callback (myScheduler, mySchedule, myFunc);
 *
 * @endcode
 *
 * @param   scheduler  Pointer to a scheduler
 * @param   schedule   Pointer to the abort callback function
 * @param   func       Function to be invoked. A NULL value implies no callback.
 */
extern void iot_schedule_add_abort_callback (iot_scheduler_t * scheduler, iot_schedule_t * schedule, iot_schedule_fn_t func);

/**
 * @brief  Add callback function to be invoked when a schedule is run
 *
 * @code
 *
 *    iot_schedule_add_run_callback (myScheduler, mySchedule, myFunc);
 *
 * @endcode
 *
 * @param   scheduler  Pointer to a scheduler
 * @param   schedule   Pointer to the run callback function
 * @pointer func       Function to be invoked. A NULL value implies no callback.
 */
extern void iot_schedule_add_run_callback (iot_scheduler_t * scheduler, iot_schedule_t * schedule, iot_schedule_fn_t func);

/**
 * @brief  Add callback function to be invoked when a schedule run is aborted
 *
 * @code
 *
 *    iot_schedule_add_abort_callback (myScheduler, mySchedule, myFunc);
 *
 * @endcode
 *
 * @param   scheduler  Pointer to a scheduler
 * @param   schedule   Pointer to the abort callback function
 * @pointer func       Function to be invoked. A NULL value implies no callback.
 */
extern void iot_schedule_add_abort_callback (iot_scheduler_t * scheduler, iot_schedule_t * schedule, iot_schedule_fn_t func);

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
 *    iot_scheduler_stop (myScheduler);
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
 *    iot_scheduler_free (myScheduler);
 *
 * @endcode
 *
 * @param  scheduler  Pointer to a scheduler.
 */
extern void iot_scheduler_free (iot_scheduler_t * scheduler);

/**
 * @brief  Return the number of scheduled events dropped
 *
 * @code
 *
 *    iot_schedule_dropped (schedule);
 *
 * @endcode
 *
 * @param  schedule  Pointer to a schedule.
 * @return Number of events dropped
 */
 extern uint64_t iot_schedule_dropped (const iot_schedule_t * schedule);

 /**
 * @brief  Return unique schedule id
 *
 * @param  schedule  Pointer to a schedule
 * @return           The schedule id
 */
extern uint64_t iot_schedule_id (const iot_schedule_t * schedule);

/**
 * @brief  Create Scheduler component factory
 *
 * @return  Pointer to Scheduler component factory
 */
extern const iot_component_factory_t * iot_scheduler_factory (void);

#ifdef __cplusplus
}
#endif
#endif
