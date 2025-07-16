#include <pthread.h>
#include <iot/data.h>
#include <iot/threadpool.h>
#include <iot/time.h>

typedef struct node_t
{
  pthread_mutex_t mutex;
  iot_data_t * request_list;
  iot_threadpool_t * pool;
} node_t;

/*
 * The following setup recreates an edge case for submitting jobs for the threadpool.
 *
 * This occurs in scenarios where the queues for the "nodes" are processed faster than we can submit messages for them and so remain at "0" elements.
 *
 * This means that when we add a message to the queue from the submission thread and check if we should start the task for the message processing
 * we pretty much always have to add a new job into the threadpool's job queue.
 *
 * This can be seen by modifying PROCESS_REQUEST_SLEEP_US. As we increase this value (making it more likely the message queue for each node is never empty)
 * it means that we don't call iot_threadpool_add_work from the single thread submitting messages to each queue.
 *
 */

static const size_t PROCESS_REQUEST_SLEEP_US = 10; // you can see the difference by changing this from 10 to 1000

static const size_t ITER_SLEEP_US = 1000;
static const size_t ITERS = 1000;
static const size_t THREADCOUNT = 14;
static const size_t NODE_COUNT = 100;

static void * node_process_request (void * data)
{
  node_t * node = data;

  pthread_mutex_lock (&node->mutex);
  iot_data_t * request = iot_data_list_head_pop (node->request_list);
  pthread_mutex_unlock (&node->mutex);

  iot_wait_usecs (PROCESS_REQUEST_SLEEP_US);
  iot_data_free (request);

  pthread_mutex_lock (&node->mutex);
  if (iot_data_list_length (node->request_list) != 0)
  {
    iot_threadpool_add_work (node->pool, node_process_request, node, -1);
  }
  pthread_mutex_unlock (&node->mutex);

  return NULL;
}

int main (int argc, char ** argv)
{
  node_t nodes[NODE_COUNT];
  iot_threadpool_t * pool = iot_threadpool_alloc (THREADCOUNT, 0, -1, -1, NULL);

  for (size_t i = 0; i < NODE_COUNT; i++)
  {
    pthread_mutex_init (&nodes[i].mutex, NULL);
    nodes[i].request_list = iot_data_alloc_list ();
    nodes[i].pool = pool;
    iot_threadpool_add_ref (pool);
  }

  iot_threadpool_start (pool);

  uint64_t call_count = 0;
  uint64_t start = iot_time_msecs ();
  for (size_t i = 0; i < ITERS; i++)
  {
    for (size_t j = 0; j < NODE_COUNT; j++)
    {
      node_t * node = &nodes[j];
      pthread_mutex_lock (&node->mutex);
      iot_data_list_tail_push (node->request_list, iot_data_alloc_null ());
      if (iot_data_list_length (node->request_list) == 1) //first item
      {
        iot_threadpool_add_work (pool, node_process_request, node, -1);
        call_count++;
      }
      pthread_mutex_unlock (&node->mutex);
    }
    iot_wait_usecs (ITER_SLEEP_US);
    if (i % 100 == 0)
    {
      fprintf (stderr, "DONE %lu iters\n", i);
    }
  }
  uint64_t time = iot_time_msecs () - start;
  fprintf (stderr, "DONE %lu iters in %lu ms, add_work call count %lu/%lu \n", ITERS, time, call_count, ITERS * NODE_COUNT);

  iot_threadpool_stop (pool);

  for (size_t i = 0; i < NODE_COUNT; i++)
  {
    pthread_mutex_destroy (&nodes[i].mutex);
    iot_data_free (nodes[i].request_list);
    iot_threadpool_free (nodes[i].pool);
  }

  iot_threadpool_free (pool);

  return 0;
}
