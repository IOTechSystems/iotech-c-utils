/*
 * Copyright (c) 2020-2023
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cont.h"
#include "CUnit.h"
#include "iot/config.h"

static const char * main_config =
"{"
  "\"ref\":\"logger\""
"}";

static const char * logger_config =
"{"
  "\"Name\":\"${USER}\","
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
}
