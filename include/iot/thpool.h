#ifndef _IOT_THPOOL_H_
#define _IOT_THPOOL_H_

#include "iot/os.h"

typedef struct iot_threadpool iot_threadpool;


/**
 * @brief  Initialize threadpool
 *
 * Initializes a threadpool. This function will not return untill all
 * threads have initialized successfully.
 *
 * @example
 *
 *    ..
 *    iot_threadpool thpool;                     //First we declare a threadpool
 *    thpool = iot_thpool_init(4);               //then we initialize it to 4 threads
 *    ..
 *
 * @param  num_threads   number of threads to be created in the threadpool
 * @return iot_threadpool    created threadpool on success, NULL on error
 */
iot_threadpool * iot_thpool_init (unsigned num_threads);


/**
 * @brief Add work to the job queue
 *
 * Takes an action and its argument and adds it to the threadpool's job queue.
 * If you want to add to work a function with more than one arguments then
 * a way to implement this is by passing a pointer to a structure.
 *
 * NOTICE: You have to cast both the function and argument to not get warnings.
 *
 * @example
 *
 *    void print_num(int num){
 *       printf("%d\n", num);
 *    }
 *
 *    int main() {
 *       ..
 *       int a = 10;
 *       iot_thpool_add_work(thpool, (void*)print_num, (void*)a);
 *       ..
 *    }
 *
 * @param  iot_threadpool  threadpool to which the work will be added
 * @param  function        pointer to function to add as work
 * @param  arg             pointer to an argument
 */
void iot_thpool_add_work (iot_threadpool * thpool, void (*function) (void*), void * arg);


/**
 * @brief Wait for all queued jobs to finish
 *
 * Will wait for all jobs - both queued and currently running to finish.
 * Once the queue is empty and all work has completed, the calling thread
 * (probably the main program) will continue.
 *
 * Smart polling is used in wait. The polling is initially 0 - meaning that
 * there is virtually no polling at all. If after 1 seconds the threads
 * haven't finished, the polling interval starts growing exponentially
 * untill it reaches max_secs seconds. Then it jumps down to a maximum polling
 * interval assuming that heavy processing is being used in the threadpool.
 *
 * @example
 *
 *    ..
 *    iot_threadpool thpool = iot_thpool_init(4);
 *    ..
 *    // Add a bunch of work
 *    ..
 *    iot_thpool_wait (thpool);
 *    puts("All added work has finished");
 *    ..
 *
 * @param iot_threadpool the threadpool to wait for
 * @return nothing
 */
void iot_thpool_wait (iot_threadpool * thpool);

/**
 * @brief Destroy the threadpool
 *
 * This will wait for the currently active threads to finish and then 'kill'
 * the whole threadpool to free up memory.
 *
 * @example
 * int main() {
 *    iot_threadpool thpool1 = iot_thpool_init(2);
 *    iot_threadpool thpool2 = iot_thpool_init(2);
 *    ..
 *    iot_thpool_destroy (thpool1);
 *    ..
 *    return 0;
 * }
 *
 * @param iot_threadpool the threadpool to destroy
 * @return nothing
 */
void iot_thpool_destroy (iot_threadpool * thpool);


/**
 * @brief Show currently working threads
 *
 * Working threads are the threads that are performing work (not idle).
 *
 * @example
 * int main() {
 *    iot_threadpool thpool1 = iot_thpool_init (2);
 *    iot_threadpool thpool2 = iot_thpool_init (2);
 *    ..
 *    printf ("Working threads: %d\n", iot_thpool_num_threads_working (thpool1));
 *    ..
 *    return 0;
 * }
 *
 * @param iot_threadpool the threadpool of interest
 * @return integer       number of threads working
 */
unsigned iot_thpool_num_threads_working (iot_threadpool * thpool);

#endif
