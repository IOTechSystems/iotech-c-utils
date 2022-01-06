/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cont.h"
#include "CUnit.h"
#include "iot/config.h"

static const char * logger_config =
"{"
  "\"Name\":\"${USER}\","
  "\"Level\":\"Info\""
"}";

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
  iot_container_t * cont3 = iot_container_alloc ("three");
  CU_ASSERT (cont3 != NULL)
  iot_container_t * cont4 = iot_container_alloc ("four");
  CU_ASSERT (cont4 != NULL)
  iot_container_t * dup = iot_container_alloc ("three");
  CU_ASSERT (dup == NULL)

  iot_container_free (cont2);
  iot_container_free (cont4);
  iot_container_free (cont1);
  iot_container_free (cont3);
  iot_container_free (dup);
}

static void test_add_component (void)
{
  iot_container_t * cont = iot_container_alloc ("test");
  iot_component_factory_add (iot_logger_factory ());
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);

  const iot_component_t * comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (strcmp (comp->factory->type, IOT_LOGGER_TYPE) == 0)

  iot_container_free (cont);
}

static void test_delete_component (void)
{
  const iot_component_t * comp;
  iot_container_t * cont = iot_container_alloc ("test");
  iot_component_factory_add (iot_logger_factory ());
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "logger", logger_config);

  comp = iot_container_find_component (cont, "logger");
  CU_ASSERT (comp != NULL)
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
