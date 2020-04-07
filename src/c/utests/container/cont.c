/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cont.h"
#include "CUnit.h"

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

static void test_find (void)
{
  iot_container_t * found;

  iot_container_t * cont1 = iot_container_alloc ("one");
  iot_container_t * cont2 = iot_container_alloc ("two");
  iot_container_t * cont3 = iot_container_alloc ("three");

  found = iot_container_find ("one");
  CU_ASSERT (found == cont1)
  found = iot_container_find ("two");
  CU_ASSERT (found == cont2)
  found = iot_container_find ("three");
  CU_ASSERT (found == cont3)

  iot_container_free (cont1);
  iot_container_free (cont2);
  iot_container_free (cont3);
}

void cunit_cont_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("container", suite_init, suite_clean);
  CU_add_test (suite, "container_alloc", test_alloc);
  CU_add_test (suite, "container_find", test_find);
}
