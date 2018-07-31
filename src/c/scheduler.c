#include "iot/scheduler.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <semaphore.h>
#include <time.h>
 
#define SCHD_NS_TO_SEC(SECONDS) (SECONDS / BILLION)
#define SCHD_NS_REMAINING(SECONDS) (SECONDS % BILLION)

/* ========================== STRUCTURES ============================ */


/* Edgex Schedule */
typedef struct iot_schd_t
{
    struct iot_schd_t * next;           /* The next schedule */
    struct iot_schd_t * previous;       /* The previous schedule */
    void (*function)(void* arg);        /* The function called by the schedule */
    void * arg;                         /* Function input arg */
    uint64_t period;                    /* The period of the schedule, in ns */
    uint64_t start;                     /* The start time of the schedule, in ns, */
    uint32_t repeat;                    /* The number of repetitions, 0 = infinite */
}iot_schd_t;


/* Edgex Schedule Queue */
typedef struct iot_schd_queue_t
{
    struct iot_schd_t * front;          /* Pointer to the front of queue */
    struct iot_schd_t * back;           /* Pointer to the back of queue */
    uint32_t length;                    /* Number of jobs in the queue   */
    threadpool * iot_thpool;            /* Threadpool to post jobs to */
}iot_schd_queue_t;


/* Edgex Schedule Thread and Queue */
typedef struct iot_schd_thread_t
{
    pthread_t * iot_thread;               /* Scheduler thread */
    iot_schd_queue_t * iot_queue;       /* Schedule queue */
    pthread_mutex_t iot_mutex;          /* Mutex to control access to the scheduler */
    sem_t iot_sem;                      /* Semaphore to control schedule execution */
    bool running;                       /* Flag to indicate if the scheduler is running */
}iot_schd_thread_t;


/* ========================== PROTOTYPES ============================ */


iot_schd_queue_t *  create_schedule_queue(threadpool * thpool_ex);

void delete_schedule_queue(iot_schd_queue_t * schd_queue_ex);

void add_schedule_to_queue
(
    iot_schd_queue_t * schedule_queue_ex, 
    iot_schd_t * new_schedule_ex
);

void remove_schedule_from_queue
(
    iot_schd_queue_t * schedule_queue_ex, 
    iot_schd_t * remove_schedule_ex
);                  
                    
iot_schd_t *  create_schedule
(  
    void (*function_ex)(void* arg), 
    void * arg_ex, 
    uint64_t period_ex, 
    uint64_t start_ex, 
    uint32_t repeat_ex
);

int delete_schedule(iot_schd_t * del_schedule_ex);

void * edgex_scheduler_thread(void * params);

void nsToTimespec(uint64_t ns, struct timespec * ts);
/* ========================== Scheduler ============================ */

void nsToTimespec(uint64_t ns, struct timespec * ts_i) 
{   
    ts_i->tv_sec  = SCHD_NS_TO_SEC(ns);
    ts_i->tv_nsec = SCHD_NS_REMAINING(ns);
}

void * edgex_scheduler_thread(void * params_i)
{
    bool alive = true;
    
    struct timespec currentTime;
    struct timespec schdTime;
    
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)params_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    
    clock_gettime(CLOCK_REALTIME, &schdTime);
    while (alive)
    {
        
        /* Wait until the next schedule is due to execute*/
        if (sem_timedwait(&scheduler->iot_sem,&schdTime)==0)
        {
            pthread_mutex_lock(&scheduler->iot_mutex);
            /* If thread shutdown has been requested */
            if (scheduler->running==false)
            {
                alive=false;
            }
            else
            {
                /* Convert the next execution time (in MS) to timespec */
                nsToTimespec(queue->front->start,&schdTime);
            }
            pthread_mutex_unlock(&scheduler->iot_mutex);
        }
        else
        {
            pthread_mutex_lock(&scheduler->iot_mutex);
            
            /* Get the schedule at the front of the queue */
            iot_schd_t * current_schd = queue->front;
            
            /* Post the work to the threadpool */
            thpool_add_work(*(threadpool*)queue->iot_thpool, current_schd->function, current_schd->arg);
            
            clock_gettime(CLOCK_REALTIME, &currentTime);
            uint64_t time_now = (uint64_t) currentTime.tv_sec * BILLION + (uint64_t) currentTime.tv_nsec;
            
            /* Recalculate the next start time for the schedule */
            current_schd->start = time_now+current_schd->period;
            
            if (current_schd->repeat!=0)
            {
                current_schd->repeat-=1;
                /* If the number of repetitions has just become 0 */
                if (current_schd->repeat==0)
                {
                    /* Remove the schedule from the queue */
                    remove_schedule_from_queue(queue,current_schd);
                }
            }
            else 
            {
                /* Remove from current position and add in new location */
                remove_schedule_from_queue(queue,current_schd);
                add_schedule_to_queue(queue,current_schd);
            }
            
            /* Convert the next execution time (in MS) to timespec */
            nsToTimespec(queue->front->start,&schdTime);
            
            pthread_mutex_unlock(&scheduler->iot_mutex);
        }
    }
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}


/* ========================== API ============================ */


/* Initialise the schedule queue and processing thread */
void * iot_scheduler_init(threadpool * thpool_i)
{
    iot_schd_thread_t * scheduler;
    scheduler=(struct iot_schd_thread_t*)malloc(sizeof(struct iot_schd_thread_t));
    
    /* Init mutex */
    pthread_mutex_init(&(scheduler->iot_mutex), NULL);
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    /* Create the schedule queue */
    scheduler->iot_queue = create_schedule_queue(thpool_i);
    
    /* Init semaphore */
    sem_init(&scheduler->iot_sem, 0, 0);
    
    pthread_mutex_unlock(&scheduler->iot_mutex);
    return (void*)scheduler;
} 

/* Start the scheduler thread */
void iot_scheduler_start(iot_scheduler * scheduler_i)
{
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    scheduler->running=true;
    scheduler->iot_thread = malloc(sizeof(pthread_t));
    
    /* Start scheduler thread, pass in threadpool provided */
    pthread_create(scheduler->iot_thread, NULL, edgex_scheduler_thread, (void*)scheduler);
    
    pthread_mutex_unlock(&scheduler->iot_mutex);
}

/* Create a schedule and insert it into the queue */
iot_schedule iot_schedule_create
(
    iot_scheduler scheduler_i,
    void (*function_i)(void* arg), 
    void * arg_i, 
    uint64_t period_i, 
    uint64_t start_i, 
    uint32_t repeat_i
)
{
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    
    /* Create the schedule */
    iot_schd_t * schedule = create_schedule(function_i,arg_i,period_i,start_i,repeat_i);
    
    /* Inset new schedule into queue */
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    
    pthread_mutex_lock(&scheduler->iot_mutex);
    add_schedule_to_queue(queue,schedule);
    /* If the schedule was placed and the front of the queue & the scheduler is running */
    if (queue->front==schedule&&scheduler->running==true)
    {
        /* Post on the semaphore */
        sem_post(&scheduler->iot_sem);
    }
    
    pthread_mutex_unlock(&scheduler->iot_mutex);
    
    return schedule;
}

/* Remove a schedule from the queue and delete it */
int iot_schedule_delete(iot_scheduler scheduler_i, iot_schedule schedule_i)
{
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    remove_schedule_from_queue(queue,schedule_i);
    int success = delete_schedule(schedule_i);
    
    pthread_mutex_unlock(&scheduler->iot_mutex);
    
    return success;
}

/* Stop the scheduler thread */
void iot_scheduler_stop(iot_scheduler * scheduler_i)
{   
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    scheduler->running=false;
    /* Post on the semaphore */
    sem_post(&scheduler->iot_sem);
    
    /* wait for threadpool processing to complete */
    thpool_wait(*(threadpool*)scheduler->iot_queue->iot_thpool);
    
    pthread_mutex_unlock(&scheduler->iot_mutex);
    pthread_join(*(pthread_t*)scheduler->iot_thread,NULL);
}

/* Destroy all remaining scheduler resouces */
void iot_scheduler_fini(iot_scheduler * scheduler_i)
{
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    
    iot_scheduler_stop(scheduler_i);
    
    /* Empty the queue */
    while (queue->length>0)
    {
        iot_schedule_delete(scheduler,queue->front);
    }
    
    /* Delete the queue */
    pthread_mutex_lock(&scheduler->iot_mutex);
    delete_schedule_queue(queue);
    pthread_mutex_unlock(&scheduler->iot_mutex);
    
    
    /* Delete the scheduler */
    free(scheduler->iot_thread);
    free(scheduler);
}


/* ========================== Queue Management ============================ */


/* Create a schedule queue */
iot_schd_queue_t * create_schedule_queue(threadpool * thpool_i)
{
    iot_schd_queue_t * queue;
    queue=(struct iot_schd_queue_t*)malloc(sizeof(struct iot_schd_queue_t));
    if (queue==NULL)
    {
	    /* Err could not allocate queue */
		return NULL;
    }
    queue->front=NULL;
    queue->back=NULL;
    queue->length=0;
    queue->iot_thpool=thpool_i;
    return queue;
}


/* Delete a schedule queue */
void delete_schedule_queue(iot_schd_queue_t * queue_i)
{
    if (queue_i == NULL)return;
    free(queue_i);
}


/* ========================== Schedule Management ============================ */


/* Create a schedule */
iot_schd_t * create_schedule
(
    void (*function_i)(void* arg), 
    void * arg_i, 
    uint64_t period_i, 
    uint64_t start_i, 
    uint32_t repeat_i
)
{
    iot_schd_t * schedule; 
    schedule=(struct iot_schd_t*)malloc(sizeof(struct iot_schd_t));
    if (schedule==NULL)
    {
	    /* Err could not allocate schedule */
		return NULL;
    }
    schedule->next=NULL;
    schedule->previous=NULL;
    schedule->function=function_i;
    schedule->arg=arg_i;
    schedule->period=period_i;
    schedule->start=start_i;
    schedule->repeat=repeat_i;
    return schedule;
}


/* Add a schedule to the queue */
void add_schedule_to_queue
(
    iot_schd_queue_t * queue_i, 
    iot_schd_t * schedule_i   
)
{   
    /* Search for the correct schedule location */
    iot_schd_t * next_schedule = NULL;
    iot_schd_t * previous_schedule = NULL;
    
    iot_schd_t * current_sched = queue_i->front;
    for (int i = 0;  i<queue_i->length ; i++)
    {
        if (schedule_i->start<current_sched->start)
        {
            next_schedule = current_sched;
            previous_schedule = current_sched->previous;
            i=queue_i->length;
        }
        else
        {
            previous_schedule=current_sched;
        }
        current_sched=current_sched->next;
    }

    /* Insert new schedule in correct location (smallest start to next execution) */
    if (queue_i->length==0)
    {
        schedule_i->next=NULL;
        schedule_i->previous=NULL;
    }
    else
    {
        /* Set references in new entry */
        schedule_i->next=next_schedule;
        schedule_i->previous=previous_schedule;
        
        /* Update exsiting references, check if either is at the front or back */
        if (previous_schedule!=NULL)
        {
            previous_schedule->next=schedule_i;
        }
        if (next_schedule!=NULL)
        {
            next_schedule->previous=schedule_i;
        }
    }
    /* Increment the queue length */
    queue_i->length+=1;
    
    /* If no pervious schedule, set as front */
    if (previous_schedule==NULL)
    {
        queue_i->front=schedule_i;
    }
    
    /* if no next schedule, set as back */
    if (next_schedule==NULL)
    {
        queue_i->back=schedule_i;
    }
}


/* Remove a schedule from the queue */
void remove_schedule_from_queue
(
    iot_schd_queue_t * queue_i, 
    iot_schd_t * schedule_i
)
{   
    if (schedule_i->next==NULL&&schedule_i->previous==NULL)
    {
         /* If only one schedule exsists in the queue */
        queue_i->front=NULL;
        queue_i->back=NULL;
    }
    else if (schedule_i->next==NULL&&schedule_i->previous!=NULL)
    {
        /* If the schedule to remove is at the back of the queue */
        schedule_i->previous->next=NULL;
        queue_i->back=schedule_i->previous;
    }
    else if (schedule_i->previous==NULL&&schedule_i->next!=NULL)
    {
        /* If the schedule to remove is at the front of the queue */ 
        schedule_i->next->previous=NULL;
        queue_i->front=schedule_i->next;
    }
    else
    {
        /* If the schedule is in the middle of the queue */
        schedule_i->next->previous=schedule_i->previous;
        schedule_i->previous->next=schedule_i->next;
    }
    schedule_i->next=NULL;
    schedule_i->previous=NULL;
    
    /* Decrement the number of schedules */
    queue_i->length-=1;
}


/* Delete a schedule */
int delete_schedule(iot_schd_t * schedule_i)
{
    if (schedule_i == NULL)
    {
        return 0;
    }
    
    /* Check that the schedule isn't in the queue */
    if (schedule_i->next==NULL&&schedule_i->previous==NULL)
    {
        free(schedule_i);
        return 1;
    }
    else
    {
        printf("N\n");
        return 0;
    }
}
