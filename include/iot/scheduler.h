//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
/** @file
 *  @brief Scheduler API
 */
#ifndef _IOT_SCHEDULER_H_
#define _IOT_SCHEDULER_H_

#include "iot/os.h"
#include "iot/thpool.h"

#define IOT_BILLION 1000000000L
#define IOT_MS_TO_NS(MILLISECONDS) (MILLISECONDS * 1000000)
#define IOT_SEC_TO_NS(SECONDS) (SECONDS * IOT_BILLION)
#define IOT_MIN_TO_NS(MINUTES) (MINUTES * IOT_BILLION * 60)

typedef struct iot_scheduler_t iot_scheduler_t;
typedef struct iot_schedule_t iot_schedule_t;

/**
 * @brief  Initialise the scheduler
 *
 * This function initialises the scheduler.
 *
 * @code
 *
 *    iot_scheduler_t * myScheduler  = iot_scheduler_init (&thpool);
 *
 * @endcode
 *
 * @param  pool              A pointer to the associated thread pool.
 * @return iot_scheduler_t   A pointer to the created scheduler. NULL on error.
 */
iot_scheduler_t * iot_scheduler_init (iot_threadpool_t * pool);

/**
 * @brief  Start the scheduler
 *
 * This function starts the scheduler.
 *
 * @code
 *
 *    iot_scheduler_start (myScheduler);
 *
 * @endcode
 * @param  scheduler  A pointer to the iot_scheduler_t.
 */
void iot_scheduler_start (iot_scheduler_t * scheduler);


/**
 * @brief  Create a new schedule
 *
 * This function creates a new schedule.
 *
 * @code
 *
 *    iot_schedule_t * mySchedule = iot_schedule_create (sched, func, NULL,IOT_SEC_TO_NS(1),0,0);
 *
 * @endcode
 * @param  schd               A pointer to the iot_scheduler_t.
 * @param  function           The function that should be called when the schedule is triggered.
 * @param  arg                The argument to be passed to the function.
 * @param  period             The period of the schedule (in nanoseconds).
 * @param  start              The start time of the schedule (in nanoseconds).
 * @param  repeat             The number of times the schedule should repeat, (0 = infinite).
 * @param  priority           The thread priority for running the schedule, (NULL = not set).
 * @return iot_schedule       A pointer to the created schedule. NULL on error.
 */
iot_schedule_t * iot_schedule_create
(
  iot_scheduler_t * schd,
  void (*function) (void* arg),
  void * arg,
  uint64_t period,
  uint64_t start,
  uint64_t repeat,
  const int * priority
);

/**
 * @brief  Delete a schedule
 *
 * This function adds a schedule to the queue.
 *
 * @code
 *
 *    int return = iot_schedule_add(myScheduler,mySchedule);
 *
 * @endcode
 * @param  scheduler                A pointer to the iot_scheduler_t. 
 * @param  schedule                 A pointer to the iot_schedule to be deleted.
 * @return                          1 on success.
 *                                  0 on error.
 */
int iot_schedule_add (iot_scheduler_t * scheduler, iot_schedule_t * schedule);

/**
 * @brief  Delete a schedule
 *
 * This function removes a schedule from the queue.
 *
 * @code
 *
 *    int return = iot_schedule_remove(myScheduler,mySchedule);
 *
 * @endcode
 * @param  scheduler                A pointer to the iot_scheduler_t. 
 * @param  schedule                 A pointer to the iot_schedule to be deleted.
 * @return                          1 on success.
 *                                  0 on error.
 */
int iot_schedule_remove (iot_scheduler_t * scheduler, iot_schedule_t * schedule);

/**
 * @brief  Delete a schedule
 *
 * This function deletes a schedule.
 *
 * @code
 *
 *    int return = iot_schedule_delete(myScheduler,mySchedule);
 *
 * @endcode
 * @param  scheduler                A pointer to the iot_scheduler_t. 
 * @param  schedule                 A pointer to the iot_schedule to be deleted.
 */
void iot_schedule_delete (iot_scheduler_t * scheduler, iot_schedule_t * schedule);


/**
 * @brief  Stops execution of the scheduler
 *
 * This function stops the scheduler. The schedule queue may be updated while 
 * the scheduler is stopped.
 *
 * @code
 *
 *    iot_scheduler_t_stop(myScheduler);
 *
 * @endcode
 * @param  scheduler                A pointer to the iot_scheduler_t. 
 */
void iot_scheduler_stop (iot_scheduler_t * scheduler);


/**
 * @brief  Destory scheduler
 *
 * This function destroys the scheduler and all associated data.
 *
 * @code
 *
 *    iot_scheduler_t_fini(myScheduler);
 *
 * @endcode
 * @param  scheduler                A pointer to the iot_scheduler_t. 
 */
void iot_scheduler_fini (iot_scheduler_t * scheduler);

#endif
