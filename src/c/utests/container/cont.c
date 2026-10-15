/*
 * Copyright (c) 2020-2023
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cont.h"
#include "CUnit.h"
#include "iot/config.h"
#include <pthread.h>
#include <sched.h>

static const char * main_config =
"{"
  "\"ref\":\"logger\""
"}";

static const char * logger_config =
"{"
  "\"Name\":\"${USER}\","
  "\"Level\":\"Info\""
"}";

static const char * bad_logger_config_1 =
  "{"
  "\"Name\":\"${}\","
  "\"Level\":\"Info\""
  "}";

static const char * bad_logger_config_2 =
  "{"
  "\"Name\":\"${aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa}\","
  "\"Level\":\"Info\""
  "}";

static const char * bad_logger_config_3 =
  "{"
  "\"Name\":\"${\","
  "\"Level\":\"Info\""
  "}";

static const char * bad_logger_config_4 =
  "{"
  "\"Name\":\"${I_DO_NOT_EXIST}\","
  "\"Level\":\"Info\""
  "}";

static bool running_called = false;
static bool stopping_called = false;

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void test_alloc (void)
{
  iot_container_t * cont1 = iot_container_alloc ("one");
  CU_ASSERT (cont1 != NULL)
  iot_container_t * cont2 = iot_container_alloc ("two");
  CU_ASSERT (cont2 != NULL)
  iot_container_free (cont1);
  iot_container_free (cont2);
}

static void test_logger_running_callback (iot_component_t * comp, bool timeout)
{
  (void) comp;
  CU_ASSERT (! timeout)
  running_called = true;
}

static void test_logger_stopping_callback (iot_component_t * comp)
{
  (void) comp;
  stopping_called = true;
}

static void test_add_component (void)
{
  iot_container_t * cont = iot_container_alloc ("test");
  iot_component_factory_add (iot_logger_factory ());
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);

  const iot_component_t * comp = iot_container_find_component (cont, "logger");

  CU_ASSERT (comp != NULL)
  CU_ASSERT (comp && strcmp (comp->factory->type, IOT_LOGGER_TYPE) == 0)
  CU_ASSERT (comp && strcmp (comp->name, "logger") == 0)

  iot_data_t * map = iot_data_from_json (main_config);
  const iot_component_t * cmp = iot_config_component (map, "ref", cont, NULL);
  CU_ASSERT (cmp != NULL)
  cmp = iot_config_component (map, "Nope", cont, NULL);
  CU_ASSERT (cmp == NULL)
  iot_data_free (map);
  iot_container_free (cont);
}


static void test_bad_env_vars_in_config (void) // Tests bad environment variable in config don't crash the program
{
  iot_container_t * cont = iot_container_alloc ("test");
  iot_component_factory_add (iot_logger_factory ());
  iot_component_t * comp;
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", bad_logger_config_1);
  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp == NULL)
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", bad_logger_config_2);
  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp == NULL)
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", bad_logger_config_3);
  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp == NULL)
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", bad_logger_config_4);
  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp == NULL)
  iot_container_free (cont);
}

static void test_delete_component (void)
{
  iot_component_t * comp;
  iot_container_t * cont = iot_container_alloc ("test");
  iot_component_factory_add (iot_logger_factory ());
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);

  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp != NULL)
  iot_component_set_running_callback (comp, test_logger_running_callback);
  iot_component_set_stopping_callback (comp, test_logger_stopping_callback);
  iot_container_start (cont);
  CU_ASSERT (running_called)
  iot_container_stop (cont);
  CU_ASSERT (stopping_called)
  iot_container_delete_component (cont, "logger");
  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp == NULL)
  iot_container_free (cont);
}

// Component whose stop waits for a thread that uses the container

typedef struct test_worker_t
{
  iot_component_t component;
  iot_container_t * cont;
  pthread_t tid;
  atomic_bool stop;
  atomic_uint lookups;
} test_worker_t;

static void * test_worker_thread (void * arg)
{
  test_worker_t * worker = arg;
  while (! atomic_load (&worker->stop))
  {
    if (iot_container_find_component (worker->cont, "worker")) atomic_fetch_add (&worker->lookups, 1u);
    sched_yield ();
  }
  return NULL;
}

static void test_worker_start (iot_component_t * comp)
{
  test_worker_t * worker = (test_worker_t*) comp;
  atomic_store (&worker->stop, false);
  pthread_create (&worker->tid, NULL, test_worker_thread, worker);
  iot_component_set_running (comp);
}

static void test_worker_stop (iot_component_t * comp)
{
  test_worker_t * worker = (test_worker_t*) comp;
  atomic_store (&worker->stop, true);
  pthread_join (worker->tid, NULL);
  iot_component_set_stopped (comp);
}

static iot_component_t * test_worker_config (iot_container_t * cont, const iot_data_t * map)
{
  (void) map;
  test_worker_t * worker = calloc (1, sizeof (*worker));
  worker->cont = cont;
  iot_component_init (&worker->component, NULL, test_worker_start, test_worker_stop);
  return &worker->component;
}

static void test_worker_free (iot_component_t * comp)
{
  iot_component_fini (comp);
  free (comp);
}

static iot_component_factory_t test_worker_factory = { "test::worker", IOT_CATEGORY_USER, NULL, NULL, NULL, NULL };

static void test_delete_running_component (void)
{
  // Deleting a running component whose stop waits on a thread using the container does not deadlock
  test_worker_factory.config_fn = test_worker_config;
  test_worker_factory.free_fn = test_worker_free;
  iot_component_factory_add (&test_worker_factory);
  iot_container_t * cont = iot_container_alloc ("worker_cont");
  iot_container_add_component (cont, "test::worker", "worker", "{}");
  test_worker_t * worker = (test_worker_t*) iot_container_find_component (cont, "worker");
  CU_ASSERT_FATAL (worker != NULL)
  iot_container_start (cont);
  while (atomic_load (&worker->lookups) < 100u) sched_yield (); // Worker thread is using the container
  iot_container_delete_component (cont, "worker");
  CU_ASSERT (iot_container_find_component (cont, "worker") == NULL)
  iot_container_free (cont);
}

// Component that adds a component when started and deletes it when stopped

static void test_adder_start (iot_component_t * comp)
{
  iot_container_add_component (comp->container, IOT_LOGGER_TYPE, "added", logger_config);
  iot_component_set_running (comp);
}

static void test_adder_stop (iot_component_t * comp)
{
  iot_container_delete_component (comp->container, "added");
  iot_component_set_stopped (comp);
}

static iot_component_t * test_adder_config (iot_container_t * cont, const iot_data_t * map)
{
  (void) cont;
  (void) map;
  iot_component_t * comp = calloc (1, sizeof (*comp));
  iot_component_init (comp, NULL, test_adder_start, test_adder_stop);
  return comp;
}

static iot_component_factory_t test_adder_factory = { "test::adder", IOT_CATEGORY_USER, NULL, NULL, NULL, NULL };

static void test_start_stop_modify (void)
{
  // Components may add and delete components when started and stopped, without deadlock
  test_adder_factory.config_fn = test_adder_config;
  test_adder_factory.free_fn = test_worker_free;
  iot_component_factory_add (&test_adder_factory);
  iot_component_factory_add (iot_logger_factory ());
  iot_container_t * cont = iot_container_alloc ("adder_cont");
  iot_container_add_component (cont, "test::adder", "adder", "{}");
  iot_container_start (cont);
  CU_ASSERT (iot_container_find_component (cont, "added") != NULL)
  iot_container_stop (cont);
  CU_ASSERT (iot_container_find_component (cont, "added") == NULL)
  CU_ASSERT (iot_container_find_component (cont, "adder") != NULL)
  iot_container_free (cont);
}

static const char * lazy_logger_config = "{\"Name\":\"lazy\",\"Level\":\"Info\"}";

static char * test_lazy_load (const char * name, const char * uri)
{
  (void) uri;
  if (strcmp (name, "lazy") == 0) return strdup ("{\"lazy1\":\"" IOT_LOGGER_TYPE "\",\"lazy2\":\"" IOT_LOGGER_TYPE "\"}");
  if (strncmp (name, "lazy", 4) == 0) return strdup (lazy_logger_config);
  return strdup ("{}"); // Container configuration remains set for later tests, so return empty configuration
}

static iot_container_config_t test_lazy_config = { test_lazy_load, NULL, NULL };
static iot_container_t * test_lazy_cont;
static pthread_barrier_t test_lazy_barrier;
static atomic_uint test_lazy_nulls;

static void * test_lazy_thread (void * arg)
{
  pthread_barrier_wait (&test_lazy_barrier);
  if (iot_container_find_component (test_lazy_cont, arg) == NULL) atomic_fetch_add (&test_lazy_nulls, 1u);
  return NULL;
}

static void test_lazy_load_concurrent (void)
{
  // Concurrent lookups of components not yet loaded each load the component once, and all find it
  pthread_t threads[8];
  iot_component_factory_add (iot_logger_factory ());
  iot_container_config (&test_lazy_config);
  test_lazy_cont = iot_container_alloc ("lazy");
  atomic_store (&test_lazy_nulls, 0u);
  pthread_barrier_init (&test_lazy_barrier, NULL, 8);
  for (int i = 0; i < 8; i++) pthread_create (&threads[i], NULL, test_lazy_thread, (i & 1) ? "lazy1" : "lazy2");
  for (int i = 0; i < 8; i++) pthread_join (threads[i], NULL);
  pthread_barrier_destroy (&test_lazy_barrier);
  CU_ASSERT (atomic_load (&test_lazy_nulls) == 0u)
  iot_data_t * list = iot_container_list_components (test_lazy_cont, NULL);
  CU_ASSERT (iot_data_list_length (list) == 2u)
  iot_data_free (list);
  iot_container_free (test_lazy_cont);
}

static void test_duplicate_component (void)
{
  // Adding a component with the name of an existing component is rejected
  iot_container_t * cont = iot_container_alloc ("dup");
  iot_component_factory_add (iot_logger_factory ());
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);
  iot_component_t * first = iot_container_find_component (cont, "logger");
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);
  CU_ASSERT (iot_container_find_component (cont, "logger") == first)
  iot_data_t * list = iot_container_list_components (cont, NULL);
  CU_ASSERT (iot_data_list_length (list) == 1u)
  iot_data_free (list);
  iot_container_free (cont);
}

static void test_component_read_reconfig (void)
{
  // Component read returns a snapshot, unchanged by reconfiguration or component deletion (checked by valgrind)
  iot_container_t * cont = iot_container_alloc ("reconfig");
  iot_component_factory_add (iot_logger_factory ());
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);
  iot_component_t * comp = iot_container_find_component (cont, "logger");
  iot_data_t * before = iot_component_read (comp);
  iot_data_t * update = iot_data_from_json ("{\"Level\":\"Debug\"}");
  CU_ASSERT (iot_component_reconfig (comp, cont, update))
  iot_data_free (update);
  CU_ASSERT (strcmp (iot_data_string_map_get_string (iot_data_string_map_get (before, "config"), "Level"), "Info") == 0)
  iot_data_t * after = iot_component_read (comp);
  CU_ASSERT (strcmp (iot_data_string_map_get_string (iot_data_string_map_get (after, "config"), "Level"), "Debug") == 0)
  iot_container_delete_component (cont, "logger");
  CU_ASSERT (strcmp (iot_data_string_map_get_string (after, "name"), "logger") == 0) // Name copied
  char * json = iot_data_to_json (after);
  CU_ASSERT (json != NULL)
  free (json);
  iot_data_free (before);
  iot_data_free (after);
  iot_container_free (cont);
}

static void test_state_name (void)
{
  CU_ASSERT (strcmp (iot_component_state_name (IOT_COMPONENT_INITIAL), "Initial") == 0)
  CU_ASSERT (strcmp (iot_component_state_name (IOT_COMPONENT_STOPPED), "Stopped") == 0)
  CU_ASSERT (strcmp (iot_component_state_name (IOT_COMPONENT_RUNNING), "Running") == 0)
  CU_ASSERT (strcmp (iot_component_state_name (IOT_COMPONENT_DELETED), "Deleted") == 0)
  CU_ASSERT (strcmp (iot_component_state_name (IOT_COMPONENT_STARTING), "Starting") == 0)
  CU_ASSERT (strcmp (iot_component_state_name ((iot_component_state_t) -1), "Unknown") == 0)
}

void cunit_cont_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("container", suite_init, suite_clean);
  CU_add_test (suite, "container_alloc", test_alloc);
  CU_add_test (suite, "container_state_name", test_state_name);
  CU_add_test (suite, "container_add_component", test_add_component);
  CU_add_test (suite, "container_delete_component", test_delete_component);
  CU_add_test (suite, "bad_enviroment_variables_in_config", test_bad_env_vars_in_config);
  CU_add_test (suite, "duplicate_component", test_duplicate_component);
  CU_add_test (suite, "component_read_reconfig", test_component_read_reconfig);
  CU_add_test (suite, "delete_running_component", test_delete_running_component);
  CU_add_test (suite, "start_stop_modify", test_start_stop_modify);
  CU_add_test (suite, "lazy_load_concurrent", test_lazy_load_concurrent); // Last, as sets container configuration

}
