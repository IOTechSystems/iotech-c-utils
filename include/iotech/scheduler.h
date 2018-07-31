#ifndef _EDGEX_DEVICE_SCHEDULER_H_
#define _EDGEX_DEVICE_SCHEDULER_H_ 1
#include "thpool.h"
#include <stdint.h>

#define BILLION  1000000000L
#define SCHD_MILSEC2NS(MILLISECONDS) (MILLISECONDS * 1000000)
#define SCHD_SEC2NS(SECONDS) (SECONDS * BILLION)
#define SCHD_MIN2NS(MINUTES) (MINUTES * BILLION * 60)

typedef void * edgex_scheduler;
typedef void * edgex_schedule;


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
 * @return edgex_scheduler      A pointer to the created scheduler
 *                              NULL on error
 */
edgex_scheduler edgex_scheduler_init(threadpool * thpool);


/**
 * @brief  Start the scheduler
 *
 * This function starts the scheduler.
 *
 * @example
 *
 *    ..
 *    edgex_scheduler_start(schd);
 *    ..
 */
void edgex_scheduler_start(edgex_scheduler * scheduler);


/**
 * @brief  Create a new schedule
 *
 * This function creates a new schedule and adds it to the schedule queue.
 *
 * @example
 *
 *    ..
 *    edgex_schedule mySchedule = edgex_create_schedule(schd,"TestSchedule",testFunc,NULL,1,1,1);
 *    ..
 *
 * @param  schd                     A pointer to the edgex_scheduler.
 * @param  function                 The function that should be called when the
 *                                  schedule is triggered.
 * @param  arg                      The argument to be passed to the function.
 * @param  period                   The period of the schedule.
 * @param  start                    The start time of the schedule.
 * @param  repeat                   The number of times the schedule should
 *                                  repeat, (0 = infinite)
 * @return edgex_schedule           A pointer to the created edgex schedule
 *                                  NULL on error.
 */
edgex_schedule edgex_schedule_create
(
    edgex_scheduler schd,
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
 *    int return = edgex_schedule_delete(schd,schedule);
 *    ..
 *
 * @param  queue                    A pointer to the edgex_scheduler. 
 * @param  edgex_schedule           A pointer to the edgex_schedule to be deleted.
 * @return                          1 on success.
 *                                  0 on error.
 */
int edgex_schedule_delete(edgex_scheduler scheduler, edgex_schedule schedule);


/**
 * @brief  Stops execution of the scheduler
 *
 * This function stops the scheduler. The schedule queue may be updated while 
 * the scheduler is stopped.
 *
 * @example
 *
 *    ..
 *    edgex_scheduler_stop(schd);
 *    ..
 */
void edgex_scheduler_stop(edgex_scheduler * scheduler);


/**
 * @brief  Destory scheduler
 *
 * This function destroys the scheduler and all associated data.
 *
 * @example
 *
 *    ..
 *    edgex_scheduler_fini(schd);
 *    ..
 */
void edgex_scheduler_fini(edgex_scheduler * scheduler);


#endif
