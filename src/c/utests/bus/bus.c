#include "bus.h"
#include "CUnit.h"

static int prio1 = 10;
static int prio2 = 11;
static int prio3 = 12;

static iot_threadpool_t * pool = NULL;
static iot_scheduler_t * scheduler = NULL;

static int suite_init (void)
{
  iot_data_init ();
  pool = iot_threadpool_alloc (2, 0, NULL);
  scheduler = iot_scheduler_alloc (pool);
  iot_threadpool_start (pool);
  iot_scheduler_start (scheduler);
  return 0;
}

static int suite_clean (void)
{
  iot_scheduler_stop (scheduler);
  iot_threadpool_stop (pool);
  iot_scheduler_free (scheduler);
  iot_threadpool_free (pool);
  iot_data_fini ();
  return 0;
}

static void test_bus_add_topic (void)
{
  iot_bus_t * bus = iot_bus_alloc (scheduler, pool, 0);
  iot_bus_topic_create (bus, "utest/topic-p0", false, NULL);
  iot_bus_topic_create (bus, "utest/topic-p2", false, &prio2);
  iot_bus_topic_create (bus, "utest/topic-p1", false, &prio1);
  iot_bus_topic_create (bus, "utest/topic-p3", false, &prio3);
  iot_bus_topic_create (bus, "utest/topic-p2b", false, &prio2);
  iot_bus_topic_create (bus, "utest/topic-p3b", false, &prio3);
  iot_bus_topic_create (bus, "utest/topic-p1b", false, &prio1);
  iot_bus_topic_create (bus, "utest/topic-p0b", false, NULL);
  iot_bus_free (bus);
}

static void test_bus_pub_free (void)
{
  iot_bus_t * bus = iot_bus_alloc (NULL, NULL, 0);
  iot_bus_pub_t * pub1 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic1");
  iot_bus_pub_t * pub2 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic1");
  iot_bus_pub_t * pub3 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic2");
  iot_bus_pub_free (pub1);
  iot_bus_pub_free (pub2);
  iot_bus_pub_free (pub3);
  iot_bus_free (bus);
}

static void test_bus_sub_free (void)
{
  iot_bus_t * bus = iot_bus_alloc (NULL, NULL, 0);
  iot_bus_pub_t * pub1 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic1");
  iot_bus_sub_t * sub1 = iot_bus_sub_alloc (bus, NULL, NULL, "utest/+");
  iot_bus_sub_t * sub2 = iot_bus_sub_alloc (bus, NULL, NULL, "utest/#");
  iot_bus_sub_t * sub3 = iot_bus_sub_alloc (bus, NULL, NULL, "#");
  iot_bus_sub_free (sub1);
  iot_bus_sub_free (sub2);
  iot_bus_sub_free (sub3);
  iot_bus_free (bus);
}

static void test_bus_no_threads (void)
{
  iot_bus_t * bus = iot_bus_alloc (NULL, NULL, 0);
  iot_bus_topic_create (bus, "utest/nothread", true, NULL);
  iot_data_t * data = iot_data_alloc_ui32 (45u);
  iot_bus_pub_t * pub = iot_bus_pub_alloc (bus, NULL, NULL, "utest/nothread");
  iot_bus_sub_t * sub = iot_bus_sub_alloc (bus, NULL, NULL, "utest/#");
  iot_bus_pub_push (pub, data, false);
  iot_data_t * data2 = iot_bus_sub_pull (sub);
  CU_ASSERT (data2 != NULL);
  CU_ASSERT (data2 == data);
  CU_ASSERT (iot_data_type (data2) == IOT_DATA_UINT32);
  iot_data_free (data2);
  iot_bus_free (bus);
}

static void test_bus_no_threads_priority (void)
{
  iot_bus_t * bus = iot_bus_alloc (NULL, NULL, 0);
  iot_bus_topic_create (bus, "utest/topic-p0", true, NULL);
  iot_bus_topic_create (bus, "utest/topic-p2", true, &prio2);
  iot_bus_topic_create (bus, "utest/topic-p1", true, &prio1);
  iot_data_t * data0 = iot_data_alloc_ui32 (0u);
  iot_data_t * data1 = iot_data_alloc_ui32 (1u);
  iot_data_t * data2 = iot_data_alloc_ui32 (2u);
  iot_bus_pub_t * pub0 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic-p0");
  iot_bus_pub_t * pub2 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic-p2");
  iot_bus_pub_t * pub1 = iot_bus_pub_alloc (bus, NULL, NULL, "utest/topic-p1");
  iot_bus_sub_t * sub = iot_bus_sub_alloc (bus, NULL, NULL, "utest/+");
  iot_bus_pub_push (pub0, data0, false);
  iot_bus_pub_push (pub2, data2, false);
  iot_bus_pub_push (pub1, data1, false);
  iot_data_t * data = iot_bus_sub_pull (sub);
  CU_ASSERT (data != NULL);
  CU_ASSERT (data == data2);
  iot_data_free (data);
  data = iot_bus_sub_pull (sub);
  CU_ASSERT (data != NULL);
  CU_ASSERT (data == data1);
  iot_data_free (data);
  data = iot_bus_sub_pull (sub);
  CU_ASSERT (data != NULL);
  CU_ASSERT (data == data0);
  iot_data_free (data);
  data = iot_bus_sub_pull (sub);
  CU_ASSERT (data == NULL);
  iot_bus_free (bus);
}

void cunit_bus_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("bus", suite_init, suite_clean);
  CU_add_test (suite, "bus_add_topic", test_bus_add_topic);
  CU_add_test (suite, "bus_pub_free", test_bus_pub_free);
  CU_add_test (suite, "bus_sub_free", test_bus_sub_free);
  CU_add_test (suite, "bus_no_threads", test_bus_no_threads);
  CU_add_test (suite, "bus_no_threads_priority", test_bus_no_threads_priority);
}
