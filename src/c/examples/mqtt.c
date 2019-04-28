#include <iot/bus.h>
#include <svc/export-mqtt.h>
#include <MQTTClientPersistence.h>
#include <MQTTClient.h>

#define ARRAY_SIZE 5
#define ARRAY_MOD 1000
#define broker_add "54.171.53.113:13939"


static void testFunc1 (void *in)
{
  printf ("FN-1 ");
}

//data to be pushed to broker.
static iot_data_t * iot_data_array (void)
{
  iot_data_t * array = iot_data_alloc_array (ARRAY_SIZE);
  for (int i = 0; i < ARRAY_SIZE; ++i)
  {
    iot_data_t * value = iot_data_alloc_i16 (rand () % ARRAY_MOD);
    iot_data_array_add (array, i, value);
  }
  return array;
}

static void subscriber_callback (iot_data_t * data, void * self, const char * match)
{
#ifndef NDEBUG
  printf ("Subscription (%s): ", match);
  char * json = iot_data_to_json (data, true);
  printf ("%s\n", json);
  free (json);
#endif
}

int main (void)
{
 // iot_data_t * array = iot_data_array ();
  iot_threadpool_t * thread_pool = iot_threadpool_alloc (4, NULL);

  iot_scheduler_t * scheduler = iot_scheduler_alloc (thread_pool);

  iot_bus_t * bus = iot_bus_alloc (scheduler, thread_pool, 500);
  iot_bus_sub_alloc (bus, NULL, subscriber_callback, "test");

  char * const * server = NULL;
  xrt_mqtt_exporter_t * mqtt_export = xrt_mqtt_exporter_alloc (bus, "#", broker_add, "clientid", "hsfunheq", "6n5OVDPYRnCr", "", 500, 20, 20, MQTTCLIENT_PERSISTENCE_DEFAULT, server);

  iot_schedule_t * sched1 = iot_schedule_create (scheduler, testFunc1, NULL, IOT_MS_TO_NS (500), 0, 0, NULL);
  iot_schedule_add (scheduler, sched1);



    printf ("Hi");
    iot_data_t * test = iot_data_alloc_i16 (5);
    push_topic (test, mqtt_export, "test3-test", "");

  printf ("Hi2");
  return 0;
}