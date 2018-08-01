#ifndef _IOT_SCHEDULER_H_
#define _IOT_SCHEDULER_H_
#include "thpool.h"
#include <stdint.h>

#define BILLION  1000000000L
#define SCHD_MILSEC2NS(MILLISECONDS) (MILLISECONDS * 1000000)
#define SCHD_SEC2NS(SECONDS) (SECONDS * BILLION)
#define SCHD_MIN2NS(MINUTES) (MINUTES * BILLION * 60)

typedef void * iot_scheduler;
typedef void * iot_schedule;


/**
 * @brief  Initialise the scheduler
 *
 * This function initialises the scheduler.
 *
 * @example
 *
 *    ..
 *    edgex_schedule_queue myqueue = scheduler_init();
 *    ..
 *
 * @return iot_scheduler      A pointer to the created scheduler
 *                              NULL on error
 */
iot_scheduler iot_scheduler_init(threadpool * thpool);


/**
 * @brief  Start the scheduler
 *
 * This function starts the scheduler.
 *
 * @example
 *
 *    ..
 *    iot_scheduler_start(schd);
 *    ..
 */
void iot_scheduler_start(iot_scheduler * scheduler);


/**
 * @brief  Create a new schedule
 *
 * This function creates a new schedule and adds it to the schedule queue.
 *
 * @example
 *
 *    ..
 *    iot_schedule mySchedule = edgex_create_schedule(schd,"TestSchedule",testFunc,NULL,1,1,1);
 *    ..
 *
 * @param  schd                     A pointer to the iot_scheduler.
 * @param  function                 The function that should be called when the
 *                                  schedule is triggered.
 * @param  arg                      The argument to be passed to the function.
 * @param  period                   The period of the schedule.
 * @param  start                    The start time of the schedule.
 * @param  repeat                   The number of times the schedule should
 *                                  repeat, (0 = infinite)
 * @return iot_schedule           A pointer to the created edgex schedule
 *                                  NULL on error.
 */
iot_schedule iot_schedule_create
(
    iot_scheduler schd,
    void (*function)(void* arg), 
    void * arg, 
    uint64_t period, 
    uint64_t start, 
    uint32_t repeat
);


/**
 * @brief  Delete a schedule
 *
 * This function removes a schedule from the queue and deletes it.
 *
 * @example
 *
 *    ..
 *    int return = iot_schedule_delete(schd,schedule);
 *    ..
 *
 * @param  queue                    A pointer to the iot_scheduler. 
 * @param  iot_schedule           A pointer to the iot_schedule to be deleted.
 * @return                          1 on success.
 *                                  0 on error.
 */
int iot_schedule_delete(iot_scheduler scheduler, iot_schedule schedule);


/**
 * @brief  Stops execution of the scheduler
 *
 * This function stops the scheduler. The schedule queue may be updated while 
 * the scheduler is stopped.
 *
 * @example
 *
 *    ..
 *    iot_scheduler_stop(schd);
 *    ..
 */
void iot_scheduler_stop(iot_scheduler * scheduler);


/**
 * @brief  Destory scheduler
 *
 * This function destroys the scheduler and all associated data.
 *
 * @example
 *
 *    ..
 *    iot_scheduler_fini(schd);
 *    ..
 */
void iot_scheduler_fini(iot_scheduler * scheduler);


#endif
