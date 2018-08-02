#include "iot/scheduler.h"

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
void testFunc(void *);
void testFunc2(void *);
void testFunc3(void *);

void testFunc(void * in)
{
    printf("I am a test function 1\n");
}

void testFunc2(void * in)
{
    printf("I am a test function 2\n");
}

void testFunc3(void * in)
{
    printf("I am a test function 3\n");
}


int main()
{
    /* Create a threadpool */
    printf("Create the threadpool\n");
    threadpool thpool = thpool_init(4);
    
    /* Initialise the scheduler */
    printf("Initialise the scheduler\n");
    iot_scheduler schd  = iot_scheduler_init(&thpool);
    
    /* Add two schedules to the scheduler */
    printf("Add two schedules\n");
    iot_schedule test1 = iot_schedule_create(schd,testFunc,NULL,IOT_MS_TO_NS(500),0,0);
    iot_schedule test2 = iot_schedule_create(schd,testFunc2,NULL,IOT_SEC_TO_NS(1),0,0);
    
    /* Start the scheduler */
    printf("Start the scheduler\n");
    iot_scheduler_start(schd);
    sleep(10);
    
    /* Remove a schedule */
    printf("Remove schedule 1\n");
    iot_schedule_delete(schd,test1);
    sleep(10);
    
    /* Stop the scheduler */
    printf("Stop the scheduler\n");
    iot_scheduler_stop(schd);
    sleep(10);
    
    /* Re-add the first schedule */
    printf("Re-add schedule 1\n");
    test1 = iot_schedule_create(schd,testFunc,NULL,IOT_MS_TO_NS(500),0,0);
    
     /* Start the scheduler */
     printf("Start the scheduler\n");
    iot_scheduler_start(schd);
    sleep(10);
    
    /* Add a third schedule */
    printf("Add schedule 3\n");
    iot_schedule test3 = iot_schedule_create(schd,testFunc3,NULL,IOT_SEC_TO_NS(2),0,0);
    sleep(10);
    
    /* Delete the schedules */
    printf("Delete all schedules\n");
    iot_schedule_delete(schd,test1);
    iot_schedule_delete(schd,test2);
    iot_schedule_delete(schd,test3);
    
    /* Stop and delete the scheduler (and associated schedules) */
    printf("Stop and delete the scheduler\n");
    iot_scheduler_fini(schd);
    
    /* Destroy the threadpool */
    printf("Destroy the threadpool\n");
    thpool_destroy(thpool);
}
