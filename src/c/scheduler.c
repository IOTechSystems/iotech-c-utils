#include "iotech/scheduler.h"

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
typedef struct edgex_schd_t
{
    struct edgex_schd_t * next;         /* The next schedule */
    struct edgex_schd_t * previous;     /* The previous schedule */
    void (*function)(void* arg);        /* The function called by the schedule */
    void * arg;                         /* Function input arg */
    uint64_t period;                    /* The period of the schedule, in ns */
    uint64_t start;                     /* The start time of the schedule, in ns, */
    uint32_t repeat;                    /* The number of repetitions, 0 = infinite */
}edgex_schd_t;


/* Edgex Schedule Queue */
typedef struct edgex_schd_queue_t
{
    pthread_mutex_t rwmutex;            /* Mutex to control access to the queue */
    struct edgex_schd_t * front;        /* Pointer to the front of queue */
    struct edgex_schd_t * back;         /* Pointer to the back of queue */
    uint32_t length;                    /* Number of jobs in the queue   */
    threadpool * edgex_thpool;          /* Threadpool to post jobs to */
}edgex_schd_queue_t;


/* Edgex Schedule Thread and Queue */
typedef struct edgex_schd_thread_t
{
    pthread_t edgex_thread;             /* Scheduler thread */
    edgex_schd_queue_t * edgex_queue;   /* Edgex schedule queue */
    sem_t edgex_sem;                    /* Semaphore to control schedule execution */
}edgex_schd_thread_t;


/* ========================== PROTOTYPES ============================ */


edgex_schd_queue_t *  create_schedule_queue(threadpool * thpool_ex);

void delete_schedule_queue(edgex_schd_queue_t * schd_queue_ex);

void add_schedule_to_queue
(
    edgex_schd_queue_t * schedule_queue_ex, 
    edgex_schd_t * new_schedule_ex
);

void remove_schedule_from_queue
(
    edgex_schd_queue_t * schedule_queue_ex, 
    edgex_schd_t * remove_schedule_ex
);                  
                    
edgex_schd_t *  create_schedule
(  
    void (*function_ex)(void* arg), 
    void * arg_ex, 
    uint64_t period_ex, 
    uint64_t start_ex, 
    uint32_t repeat_ex
);

int delete_schedule(edgex_schd_t * del_schedule_ex);

void * edgex_scheduler_thread(void * params);


/* ========================== Scheduler ============================ */

void nsToTimespec(uint64_t ns, struct timespec * ts) 
{   
    ts->tv_sec  = SCHD_NS_TO_SEC(ns);
    ts->tv_nsec = SCHD_NS_REMAINING(ns);
}

void * edgex_scheduler_thread(void * params)
{
    struct timespec currentTime;
    struct timespec schdTime;
    edgex_schd_thread_t * sched_thread = (edgex_schd_thread_t*)params;
    edgex_schd_queue_t * queue = (edgex_schd_queue_t*)sched_thread->edgex_queue;
    while (true)
    {
        /* Convert the next execution time (in MS) to timespec */
        nsToTimespec(queue->front->start,&schdTime);

        /* Wait until the next schedule is due to execute*/
        sem_timedwait(&sched_thread->edgex_sem,&schdTime);
        
        pthread_mutex_lock(&queue->rwmutex);
        
        /* Get the schedule at the front of the queue */
        edgex_schd_t * current_schd = queue->front;
        
        /* Post the work to the threadpool */
        thpool_add_work(*(threadpool*)queue->edgex_thpool, current_schd->function, current_schd->arg);
        
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
        
        pthread_mutex_unlock(&queue->rwmutex);
    }
}


/* ========================== API ============================ */


/* Initialise the schedule queue and processing thread */
void * edgex_scheduler_init(threadpool * thpool_ex)
{
    edgex_schd_thread_t * scheduler;
    scheduler=(struct edgex_schd_thread_t*)malloc(sizeof(struct edgex_schd_thread_t));
    
    /* Create the schedule queue */
    scheduler->edgex_queue = create_schedule_queue(thpool_ex);
    
    /* Init semaphore */
    sem_init(&scheduler->edgex_sem, 0, 0);
    
    return (void*)scheduler;
} 

/* Start the scheduler thread */
void edgex_scheduler_start(edgex_scheduler * scheduler_ex)
{
    edgex_schd_thread_t * schd = (edgex_schd_thread_t*)scheduler_ex;
    
    /* Start scheduler thread, pass in threadpool provided */
    pthread_create(&schd->edgex_thread, NULL, edgex_scheduler_thread, (void*)schd);
}

/* Create a schedule and insert it into the queue */
edgex_schedule edgex_schedule_create
(
    edgex_scheduler schd_ex,
    void (*function_ex)(void* arg), 
    void * arg_ex, 
    uint64_t period_ex, 
    uint64_t start_ex, 
    uint32_t repeat_ex
)
{
    edgex_schd_thread_t * schd = (edgex_schd_thread_t*)schd_ex;
    
    
    /* Create the schedule */
    edgex_schd_t * schedule = create_schedule(function_ex,arg_ex,period_ex,start_ex,repeat_ex);
    
    /* Inset new schedule into queue */
    edgex_schd_queue_t * queue = (edgex_schd_queue_t*)schd->edgex_queue;
    
    pthread_mutex_lock(&queue->rwmutex);
    add_schedule_to_queue(queue,schedule);
    pthread_mutex_unlock(&queue->rwmutex);
    
    return schedule;
}

/* Remove a schedule from the queue and delete it */
int edgex_schedule_delete(edgex_scheduler scheduler_ex, edgex_schedule schedule_ex)
{
    edgex_schd_thread_t * schd_t = (edgex_schd_thread_t*)scheduler_ex;
    
    edgex_schd_queue_t * queue = (edgex_schd_queue_t*)schd_t->edgex_queue;
    
    pthread_mutex_lock(&queue->rwmutex);
    remove_schedule_from_queue(queue,schedule_ex);
    pthread_mutex_unlock(&queue->rwmutex);
    
    return delete_schedule(schedule_ex);
}

/* Stop the scheduler thread */
void edgex_scheduler_stop(edgex_scheduler * scheduler_ex)
{
    
}

/* Destroy all remaining scheduler resouces */
void edgex_scheduler_fini(edgex_scheduler * scheduler_ex)
{
    edgex_schd_thread_t * sched_thread = (edgex_schd_thread_t*)scheduler_ex;
    edgex_schd_queue_t * queue = (edgex_schd_queue_t*)sched_thread->edgex_queue;
    
    // stop the scheduler thread
    
    /* Empty the queue */
    while (queue->length>0)
    {
        pthread_mutex_lock(&queue->rwmutex);
        remove_schedule_from_queue(queue,queue->front);
        pthread_mutex_unlock(&queue->rwmutex); 
    }
    
    /* Delete the queue */
    delete_schedule_queue(queue);
}


/* ========================== Queue Management ============================ */


/* Create a schedule queue */
edgex_schd_queue_t * create_schedule_queue(threadpool * thpool_ex)
{
    edgex_schd_queue_t * queue;
    queue=(struct edgex_schd_queue_t*)malloc(sizeof(struct edgex_schd_queue_t));
    if (queue==NULL)
    {
	    /* Err could not allocate queue */
		return NULL;
    }
    pthread_mutex_init(&(queue->rwmutex), NULL);
    queue->front=NULL;
    queue->back=NULL;
    queue->length=0;
    queue->edgex_thpool=thpool_ex;
    return queue;
}


/* Delete a schedule queue */
void delete_schedule_queue(edgex_schd_queue_t * schd_queue_ex)
{
    if (schd_queue_ex == NULL)return;
    free(schd_queue_ex);
}


/* ========================== Schedule Management ============================ */


/* Create a schedule */
edgex_schd_t * create_schedule
(
    void (*function_ex)(void* arg), 
    void * arg_ex, 
    uint64_t period_ex, 
    uint64_t start_ex, 
    uint32_t repeat_ex
)
{
    edgex_schd_t * schedule_p; 
    schedule_p=(struct edgex_schd_t*)malloc(sizeof(struct edgex_schd_t));
    if (schedule_p==NULL)
    {
	    /* Err could not allocate schedule */
		return NULL;
    }
    schedule_p->next=NULL;
    schedule_p->previous=NULL;
    schedule_p->function=function_ex;
    schedule_p->arg=arg_ex;
    schedule_p->period=period_ex;
    schedule_p->start=start_ex;
    schedule_p->repeat=repeat_ex;
    return schedule_p;
}


/* Add a schedule to the queue */
void add_schedule_to_queue
(
    edgex_schd_queue_t * schedule_queue_ex, 
    edgex_schd_t * new_schedule_ex   
)
{   
    /* Search for the correct schedule location */
    edgex_schd_t * next_schedule = NULL;
    edgex_schd_t * previous_schedule = NULL;
    
    edgex_schd_t * current_sched = schedule_queue_ex->front;
    for (int i = 0;  i<schedule_queue_ex->length ; i++)
    {
        if (new_schedule_ex->start<current_sched->start)
        {
            next_schedule = current_sched;
            previous_schedule = current_sched->previous;
            i=schedule_queue_ex->length;
        }
        else
        {
            previous_schedule=current_sched;
        }
        current_sched=current_sched->next;
    }

    /* Insert new schedule in correct location (smallest start to next execution) */
    if (schedule_queue_ex->length==0)
    {
        new_schedule_ex->next=NULL;
        new_schedule_ex->previous=NULL;
    }
    else
    {
        /* Set references in new entry */
        new_schedule_ex->next=next_schedule;
        new_schedule_ex->previous=previous_schedule;
        
        /* Update exsiting references, check if either is at the front or back */
        if (previous_schedule!=NULL)
        {
            previous_schedule->next=new_schedule_ex;
        }
        if (next_schedule!=NULL)
        {
            next_schedule->previous=new_schedule_ex;
        }
    }
    /* Increment the queue length */
    schedule_queue_ex->length+=1;
    
    /* If no pervious schedule, set as front */
    if (previous_schedule==NULL)
    {
        schedule_queue_ex->front=new_schedule_ex;
    }
    
    /* if no next schedule, set as back */
    if (next_schedule==NULL)
    {
        schedule_queue_ex->back=new_schedule_ex;
    }
}


/* Remove a schedule from the queue */
void remove_schedule_from_queue
(
    edgex_schd_queue_t * schedule_queue_ex, 
    edgex_schd_t * remove_schedule_ex
)
{   
    if (remove_schedule_ex->next==NULL&&remove_schedule_ex->previous==NULL)
    {
         /* If only one schedule exsists in the queue */
        schedule_queue_ex->front=NULL;
        schedule_queue_ex->back=NULL;
    }
    else if (remove_schedule_ex->next==NULL&&remove_schedule_ex->previous!=NULL)
    {
        /* If the schedule to remove is at the back of the queue */
        remove_schedule_ex->previous->next=NULL;
        schedule_queue_ex->back=remove_schedule_ex->previous;
    }
    else if (remove_schedule_ex->previous==NULL&&remove_schedule_ex->next!=NULL)
    {
        /* If the schedule to remove is at the front of the queue */ 
        remove_schedule_ex->next->previous=NULL;
        schedule_queue_ex->front=remove_schedule_ex->next;
    }
    else
    {
        /* If the schedule is in the middle of the queue */
        remove_schedule_ex->next->previous=remove_schedule_ex->previous;
        remove_schedule_ex->previous->next=remove_schedule_ex->next;
    }
    
    /* Decrement the number of schedules */
    schedule_queue_ex->length-=1;
}


/* Delete a schedule */
int delete_schedule(edgex_schd_t * del_schedule_ex)
{
    if (del_schedule_ex == NULL)
    {
        return 0;
    }
    
    /* Check that the schedule isn't in the queue */
    if (del_schedule_ex->next==NULL&&del_schedule_ex->previous==NULL)
    {
        free(del_schedule_ex);
        return 1;
    }
    else
    {
        return 0;
    }
}
