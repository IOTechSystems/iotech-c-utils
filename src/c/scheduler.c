#include "iot/scheduler.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
 
#define IOT_NS_TO_SEC(SECONDS) (SECONDS / BILLION)
#define IOT_NS_REMAINING(SECONDS) (SECONDS % BILLION)

/* ========================== STRUCTURES ============================ */


/* Schedule */
typedef struct iot_schd_t
{
    struct iot_schd_t * next;           /* The next schedule */
    struct iot_schd_t * previous;       /* The previous schedule */
    void (*function)(void* arg);        /* The function called by the schedule */
    void * arg;                         /* Function input arg */
    unsigned  long long period;         /* The period of the schedule, in ns */
    unsigned  long long start;          /* The start time of the schedule, in ns, */
    unsigned long repeat;               /* The number of repetitions, 0 = infinite */
    bool scheduled;                     /* A flag to indicate schedule status */
}iot_schd_t;


/* Schedule Queue */
typedef struct iot_schd_queue_t
{
    struct iot_schd_t * front;          /* Pointer to the front of queue */
    struct iot_schd_t * back;           /* Pointer to the back of queue */
    unsigned long length;               /* Number of jobs in the queue   */
}iot_schd_queue_t;


/* Scheduler */
typedef struct iot_schd_thread_t
{
    pthread_t iot_thread;               /* Scheduler thread */
    iot_schd_queue_t * iot_queue;       /* Schedule queue */
    iot_schd_queue_t * iot_idle_queue;  /* The queue of idle schedules */
    pthread_mutex_t iot_mutex;          /* Mutex to control access to the scheduler */
    sem_t iot_sem;                      /* Semaphore to control schedule execution */
    bool running;                       /* Flag to indicate if the scheduler is running */
    threadpool * iot_thpool;            /* Threadpool to post jobs to */
}iot_schd_thread_t;


/* ========================== PROTOTYPES ============================ */


static iot_schd_queue_t *  create_schedule_queue(void);
static int delete_schedule_queue(iot_schd_queue_t * schd_queue_ex);
static void add_schedule_to_queue
(
    iot_schd_queue_t * schedule_queue_ex, 
    iot_schd_t * new_schedule_ex
);
static void remove_schedule_from_queue
(
    iot_schd_queue_t * schedule_queue_ex, 
    iot_schd_t * remove_schedule_ex
);                  
                    
static iot_schd_t *  create_schedule
(  
    void (*function_ex)(void* arg), 
    void * arg_ex, 
    unsigned  long long period_ex, 
    unsigned  long long start_ex, 
    unsigned long repeat_ex
);
static int delete_schedule(iot_schd_t * del_schedule_ex);

static int delete_scheduler(iot_schd_thread_t * scheduler_i);

static void * iot_scheduler_thread(void * params);
static void nsToTimespec(unsigned  long long ns, struct timespec * ts);
static unsigned long long getTimeAsUInt64(struct timespec * ts_i);

/* ========================== Scheduler ============================ */

/* Convert time in ns to timespec */
static void nsToTimespec(unsigned  long long ns, struct timespec * ts_i) 
{   
    ts_i->tv_sec  = IOT_NS_TO_SEC(ns);
    ts_i->tv_nsec = IOT_NS_REMAINING(ns);
}

/* Get the current time as an unsigned 64bit int */
static unsigned long long getTimeAsUInt64(struct timespec * ts_i)
{
    clock_gettime(CLOCK_REALTIME, ts_i);
    return (unsigned  long long) ts_i->tv_sec * 
        BILLION + (unsigned  long long) ts_i->tv_nsec;
}
/* Scheduler Thread */
static void * iot_scheduler_thread(void * params_i)
{
    bool alive = true;
    
    struct timespec currentTime;
    struct timespec schdTime;
    
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)params_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    iot_schd_queue_t * idle_queue = (iot_schd_queue_t*)scheduler->iot_idle_queue;
    
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
            
            /* Check if the queue is populated */
            if (queue->length>0)
            {
                /* Get the schedule at the front of the queue */
                iot_schd_t * current_schd = queue->front;
                
                /* Post the work to the threadpool */
                thpool_add_work(*(threadpool*)scheduler->iot_thpool, 
                    current_schd->function, current_schd->arg);
                
                unsigned long long time_now = getTimeAsUInt64(&currentTime);
                
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
                        /* Add schedule to idle queue */
                        add_schedule_to_queue(idle_queue,current_schd);
                        current_schd->scheduled=false;
                    }
                    else
                    {
                        /* Remove from current position and add in new location */
                        remove_schedule_from_queue(queue,current_schd);
                        add_schedule_to_queue(queue,current_schd);
                    }
                }
                else 
                {
                    /* Remove from current position and add in new location */
                    remove_schedule_from_queue(queue,current_schd);
                    add_schedule_to_queue(queue,current_schd);
                }
                
                /* Convert the next execution time (in MS) to timespec */
                if (queue->length>0)
                {
                    nsToTimespec(queue->front->start,&schdTime);
                }
                else
                {
                    nsToTimespec(getTimeAsUInt64(&currentTime)+IOT_SEC_TO_NS(1),&schdTime);   
                }
            }
            else
            {
                /* Set the wait time to 1 second if the queue is not populated */
                nsToTimespec(getTimeAsUInt64(&currentTime)+IOT_SEC_TO_NS(1),&schdTime);   
            }
            pthread_mutex_unlock(&scheduler->iot_mutex);
        }
    }
    pthread_exit(NULL);
}


/* ========================== API ============================ */


/* Initialise the schedule queue and processing thread */
void * iot_scheduler_init(threadpool * thpool_i)
{
    iot_schd_thread_t * scheduler;
    scheduler=(struct iot_schd_thread_t*)malloc(sizeof(struct iot_schd_thread_t));
    if (scheduler==NULL)
    {
	    /* LOG: Err could not allocate scheduler */
	    printf("ERROR: Failed to allocate the scheduler\n");
		return NULL;
    }
    
    /* Init mutex */
    pthread_mutex_init(&(scheduler->iot_mutex), NULL);
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    /* Create the queues */
    scheduler->iot_queue = create_schedule_queue();
    scheduler->iot_idle_queue = create_schedule_queue();
    
    /* Set the running state */
    scheduler->running=false;
    
    /* Add the threadpool */
    scheduler->iot_thpool=thpool_i;
    
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
    
    /* Start scheduler thread, pass in threadpool provided */
    pthread_create(&scheduler->iot_thread, NULL, &iot_scheduler_thread, (void*)scheduler);

    pthread_mutex_unlock(&scheduler->iot_mutex);
}

/* Create a schedule and insert it into the queue */
iot_schedule iot_schedule_create
(
    iot_scheduler scheduler_i,
    void (*function_i)(void* arg), 
    void * arg_i, 
    unsigned  long long period_i, 
    unsigned  long long start_i, 
    unsigned long repeat_i
)
{
    
    iot_schd_t * schedule = create_schedule(function_i,arg_i,period_i,start_i,repeat_i);
  
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    iot_schd_queue_t * idle_queue = (iot_schd_queue_t*)scheduler->iot_idle_queue;
    
    pthread_mutex_lock(&scheduler->iot_mutex);
    /* Add schedule to idle queue */
    add_schedule_to_queue(idle_queue,schedule);
    pthread_mutex_unlock(&scheduler->iot_mutex);
    
    return schedule;
}

/* Add a schedule to the queue */
int iot_schedule_add(iot_scheduler scheduler_i, iot_schedule schedule_i)
{
    iot_schd_t * schedule  = (iot_schd_t*)schedule_i;
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    iot_schd_queue_t * idle_queue = (iot_schd_queue_t*)scheduler->iot_idle_queue;
    
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    if (!schedule->scheduled)
    {
        /* Remove from idle queue, add to scheduled queue */
        remove_schedule_from_queue(idle_queue,schedule);
        add_schedule_to_queue(queue,schedule);
        /* If the schedule was placed and the front of the queue & the scheduler is running */
        if (queue->front==schedule&&scheduler->running==true)
        {
            /* Post on the semaphore */
            sem_post(&scheduler->iot_sem);
        }
        
        /* Set the schedules status as scheduled */
        schedule->scheduled=true;
    }
    else
    {
        /* ERROR Schedule is already scheduled */
        pthread_mutex_unlock(&scheduler->iot_mutex);
        return 0;
    }
    pthread_mutex_unlock(&scheduler->iot_mutex);
    return 1;
}

/* Remove a schedule from the queue */
int iot_schedule_remove(iot_scheduler scheduler_i, iot_schedule schedule_i)
{
    iot_schd_t * schedule  = (iot_schd_t*)schedule_i;
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    iot_schd_queue_t * idle_queue = (iot_schd_queue_t*)scheduler->iot_idle_queue;
    
    pthread_mutex_lock(&scheduler->iot_mutex);
    if (schedule->scheduled)
    {
        remove_schedule_from_queue(queue,schedule);
        add_schedule_to_queue(idle_queue,schedule);
        schedule->scheduled=false;
    }
    else
    {
        pthread_mutex_unlock(&scheduler->iot_mutex);
        return 0;
    }
    pthread_mutex_unlock(&scheduler->iot_mutex);
    return 1;
}

/* Delete a schedule */
int iot_schedule_delete(iot_scheduler scheduler_i, iot_schedule schedule_i)
{
    iot_schd_t * schedule  = (iot_schd_t*)schedule_i;
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    iot_schd_queue_t * idle_queue = (iot_schd_queue_t*)scheduler->iot_idle_queue;
    
    int success=0;
    pthread_mutex_lock(&scheduler->iot_mutex);
    if (schedule->scheduled)
    {
        remove_schedule_from_queue(queue,schedule);
        success = delete_schedule(schedule);
    }
    else
    {
        remove_schedule_from_queue(idle_queue,schedule);
        success = delete_schedule(schedule);
    }
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
    thpool_wait(*(threadpool*)scheduler->iot_thpool);
    pthread_mutex_unlock(&scheduler->iot_mutex);
    
    pthread_join(scheduler->iot_thread,NULL);
}

/* Destroy all remaining scheduler resouces */
void iot_scheduler_fini(iot_scheduler * scheduler_i)
{
    iot_schd_thread_t * scheduler = (iot_schd_thread_t*)scheduler_i;
    iot_schd_queue_t * queue = (iot_schd_queue_t*)scheduler->iot_queue;
    iot_schd_queue_t * idle_queue = (iot_schd_queue_t*)scheduler->iot_idle_queue;
    
    iot_scheduler_stop(scheduler_i);
    
    /* Empty the scheduled queue */
    while (queue->length>0)
    {
        iot_schedule_delete(scheduler,queue->front);
    }
    
    /* Empty the idle queue */
    while (idle_queue->length>0)
    {
        iot_schedule_delete(scheduler,idle_queue->front);
    }
    
    /* Delete the queues */
    pthread_mutex_lock(&scheduler->iot_mutex);
    
    delete_schedule_queue(queue);
    delete_schedule_queue(idle_queue);
    
    pthread_mutex_unlock(&scheduler->iot_mutex);

    /* Delete the scheduler */
    delete_scheduler(scheduler);
}


/* ========================== Scheduler Management ============================ */

/* Delete a scheduler */
static int delete_scheduler(iot_schd_thread_t * scheduler_i)
{
    if (scheduler_i == NULL)
    {   
        /* ERROR: Scheduler is NULL */
        printf("ERROR: Failed to free scheduler, Scheduler is NULL\n");
        return 0;
    }
    else
    {
        free(scheduler_i);
        return 1;
    }
}

/* ========================== Queue Management ============================ */


/* Create a schedule queue */
static iot_schd_queue_t * create_schedule_queue()
{
    iot_schd_queue_t * queue;
    queue=(struct iot_schd_queue_t*)malloc(sizeof(struct iot_schd_queue_t));
    if (queue==NULL)
    {
	    /* ERROR: Failed to allocate the queue */
	    printf("ERROR: Failed to allocate the queue\n");
		return NULL;
    }
    queue->front=NULL;
    queue->back=NULL;
    queue->length=0;
    return queue;
}


/* Delete a schedule queue */
static int delete_schedule_queue(iot_schd_queue_t * queue_i)
{
    if (queue_i == NULL)
    {   
        /* ERROR: Schedule queue is NULL */
        printf("ERROR: Failed to free schedule queue, Schedule queue is NULL\n");
        return 0;
    }
    else
    {
        free(queue_i);
        return 1;
    }
}


/* ========================== Schedule Management ============================ */


/* Create a schedule */
static iot_schd_t * create_schedule
(
    void (*function_i)(void* arg), 
    void * arg_i, 
    unsigned  long long period_i, 
    unsigned  long long start_i, 
    unsigned long repeat_i
)
{
    iot_schd_t * schedule; 
    schedule=(struct iot_schd_t*)malloc(sizeof(struct iot_schd_t));
    if (schedule==NULL)
    {
        /* ERROR: Failed to allocate the schedule */
        printf("ERROR: Failed to allocate the schedule\n");
        return NULL;
    }
    schedule->next=NULL;
    schedule->previous=NULL;
    schedule->function=function_i;
    schedule->arg=arg_i;
    schedule->period=period_i;
    schedule->start=start_i;
    schedule->repeat=repeat_i;
    schedule->scheduled=false;
    return schedule;
}


/* Add a schedule to the queue */
static void add_schedule_to_queue
(
    iot_schd_queue_t * queue_i, 
    iot_schd_t * schedule_i   
)
{   
    /* Search for the correct schedule location */
    iot_schd_t * next_schedule = NULL;
    iot_schd_t * previous_schedule = NULL;
    
    iot_schd_t * current_sched = queue_i->front;
    for (unsigned long i = 0;  i<queue_i->length ; i++)
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

    /* Insert new schedule in correct location */
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
static void remove_schedule_from_queue
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
static int delete_schedule(iot_schd_t * schedule_i)
{
    if (schedule_i == NULL)
    {
        /* ERROR: Schedule queue is NULL */
        printf("ERROR: Failed to free schedule, Schedule is NULL\n");
        return 0;
    }
    
    free(schedule_i);
    return 1;
}
