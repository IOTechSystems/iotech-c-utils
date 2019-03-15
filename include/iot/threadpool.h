#ifndef _IOT_THREADPOOL_H_
#define _IOT_THREADPOOL_H_

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iot_threadpool_t iot_threadpool_t;

/**
 * @brief  Initialize thread pool
 *
 * Initializes a thread pool. This function will not return until all
 * threads have initialized successfully.
 *
 * @example
 *
 *    ..
 *    iot_threadpool pool;                      //First we declare a thread pool
 *    pool = iot_threadpool_init (4);               //then we initialize it to 4 threads
 *    ..
 *
 * @param num_threads     number of threads to be created in the threadpool
 * @param default_prio    default priority for created threads
 * @return iot_threadpool  created thread pool on success, NULL on error
 */
iot_threadpool_t * iot_threadpool_init (uint32_t num_threads, const int * default_prio);


/**
 * @brief Add work to the job queue
 *
 * Takes an action and its argument and adds it to the thread pool's job queue.
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
 *       iot_threadpool_add_work (pool, (void*) print_num, (void*) a);
 *       ..
 *    }
 *
 * @param  iot_threadpool  pool to which the work will be added
 * @param  function        pointer to function to add as work
 * @param  arg             pointer to an argument
 * @param  priority        priority to run thread at (not set if NULL)
 */
void iot_threadpool_add_work (iot_threadpool_t * pool, void (*function) (void*), void * arg, const int * priority);


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
 * until it reaches max_secs seconds. Then it jumps down to a maximum polling
 * interval assuming that heavy processing is being used in the thread pool.
 *
 * @example
 *
 *    ..
 *    iot_threadpool * pool = iot_threadpool_init (4);
 *    ..
 *    // Add a bunch of work
 *    ..
 *    iot_threadpool_wait (pool);
 *    puts("All added work has finished");
 *    ..
 *
 * @param iot_threadpool the thread pool to wait for
 * @return nothing
 */
void iot_threadpool_wait (iot_threadpool_t * pool);

/**
 * @brief Destroy the thread pool
 *
 * This will wait for the currently active threads to finish and then 'kill'
 * the whole thread pool to free up memory.
 *
 * @example
 * int main() {
 *    iot_threadpool * pool = iot_threadpool_init (2);
 *    ..
 *    iot_threadpool_destroy (pool);
 *    ..
 *    return 0;
 * }
 *
 * @param iot_threadpool the pool to destroy
 * @return nothing
 */
void iot_threadpool_destroy (iot_threadpool_t * pool);


/**
 * @brief Show currently working threads
 *
 * Working threads are the threads that are performing work (not idle).
 *
 * @example
 * int main() {
 *    iot_threadpool * pool = iot_threadpool_init (2);
 *    ..
 *    printf ("Working threads: %d\n", iot_threadpool_num_threads_working (pool));
 *    ..
 *    return 0;
 * }
 *
 * @param iot_threadpool the pool of interest
 * @return integer       number of threads working
 */
uint32_t iot_threadpool_num_threads_working (iot_threadpool_t * pool);

#ifdef __cplusplus
}
#endif
#endif
