/*
 * Copyright (c) 2020-2025
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/iot.h"
#include "misc.h"
#include "CUnit.h"

#ifdef IOT_HAS_FILE
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#define MAX_COUNTER 1000
#define MAX_SECS_COUNTER 4

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static void test_time_secs (void)
{
  uint64_t secs_time;
  for (int counter = 0; counter < MAX_SECS_COUNTER; counter++)
  {
    secs_time = iot_time_secs ();
    iot_wait_secs (1u);
    CU_ASSERT (iot_time_secs ()  > secs_time)
  }
}

static void test_time_msecs (void)
{
  uint64_t msecs_time;
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    msecs_time = iot_time_msecs ();
    iot_wait_msecs (1u);
    CU_ASSERT (iot_time_msecs ()  > msecs_time)
  }
}

static void test_time_usecs (void)
{
  uint64_t usecs_time;
  for (int counter = 0; counter < MAX_COUNTER; counter++)
  {
    usecs_time = iot_time_usecs ();
    iot_wait_usecs (1u);
    CU_ASSERT (iot_time_usecs ()  > usecs_time)
  }
}

static void test_time_nsecs (void)
{
  unsigned counter;
  uint64_t times[MAX_COUNTER];
  for (counter = 0; counter < MAX_COUNTER; counter++)
  {
    times[counter] = iot_time_nsecs ();
  }
  for (counter = 0; counter < (MAX_COUNTER - 1); counter++)
  {
    CU_ASSERT (times[counter] < times[counter + 1])
  }
}

static void test_wait (void)
{
  iot_wait_secs (1u);
  iot_wait_msecs (1000u);
  iot_wait_usecs (1000000u);
}

static void test_hash (void)
{
  CU_ASSERT ( iot_hash ("Dummy") == 3802084562)
  CU_ASSERT ( iot_hash ("int8") == 636741937)
  CU_ASSERT ( iot_hash ("int8array") == 3352350152)
  CU_ASSERT ( iot_hash ("uint8") == 3827140740)
  CU_ASSERT ( iot_hash ("uint8array") == 1993633373)
  CU_ASSERT ( iot_hash ("int16") == 3832614926)
  CU_ASSERT ( iot_hash ("int16array") == 3293575959)
  CU_ASSERT ( iot_hash ("uint16") == 1741593115)
  CU_ASSERT ( iot_hash ("uint16array") == 3942928418)
  CU_ASSERT ( iot_hash ("int32") == 3832614984)
  CU_ASSERT ( iot_hash ("int32array") == 1264005137)
  CU_ASSERT ( iot_hash ("uint32") == 1741593181)
  CU_ASSERT ( iot_hash ("uint32array") == 2235632164)
  CU_ASSERT ( iot_hash ("int64") == 3832615211)
  CU_ASSERT ( iot_hash ("int64array") == 1709245714)
  CU_ASSERT ( iot_hash ("uint64") == 1741593086)
  CU_ASSERT ( iot_hash ("uint64array") == 2964325927)
  CU_ASSERT ( iot_hash ("float32") == 2863538027)
  CU_ASSERT ( iot_hash ("float32array") == 3463494610)
  CU_ASSERT ( iot_hash ("float64") == 2863538056)
  CU_ASSERT ( iot_hash ("float64array") == 464521169)
  CU_ASSERT ( iot_hash ("bool") == 636838452)
  CU_ASSERT ( iot_hash ("boolarray") == 1007050925)
  CU_ASSERT ( iot_hash ("string") == 1386610095)
  CU_ASSERT ( iot_hash ("binary") == 2016023253)
  CU_ASSERT ( iot_hash_data ((uint8_t*) "binary", strlen ("binary")) == 2016023253)
}

static void test_uuid_string (void)
{
  CU_ASSERT (iot_util_string_is_uuid ("e79ebe07-0774-4a91-a33b-6a0115390141"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774-4a91-a33b-6a011539014"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07*0774-4a91-a33b-6a0115390141"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774*4a91-a33b-6a0115390141"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774-4a91*a33b-6a0115390141"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774-4a91-a33b*6a0115390141"))
  CU_ASSERT (! iot_util_string_is_uuid ("j79ebe07-0774-4a91-a33b-6a0115390141"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774-4a91-a33b-6a011539014-"))
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774-4a91-a33b-6a011539014y"))
}

#ifdef IOT_HAS_FILE

#define TEST_FILE_NAME "/tmp/_iot_test.json"
#define TEST_SCOPED_DIR "/tmp/_iot"
#define TEST_SCOPED_FILE_NAME "/tmp/_iot/test.txt"

static void test_write_file (void)
{
  const char * str = "Hello";
  bool file_write_ok = iot_store_write (TEST_FILE_NAME, str);
  CU_ASSERT (file_write_ok)
}

static void test_read_file (void)
{
  char * ret = iot_store_read (TEST_FILE_NAME);
  CU_ASSERT (ret != NULL)
  if (ret)
  {
    CU_ASSERT (strcmp ("Hello", ret) == 0)
    CU_ASSERT (strlen (ret) == strlen ("Hello"))
    free (ret);
  }
}

static void test_list_file (void)
{
  iot_data_t *file_list = iot_file_list ("/tmp", ".json");
  iot_data_list_iter_t iter;
  iot_data_list_iter (file_list, &iter);
  bool file_found = false;
  while (iot_data_list_iter_next (&iter))
  {
    const char * file = iot_data_string (iot_data_list_iter_value (&iter));
    if (strcmp (file, "iot_test.json") == 0)
    {
      file_found = true;
      break;
    }
  }
  iot_data_free (file_list);
  CU_ASSERT_TRUE (file_found)
}

static void test_list_config_file (void)
{
  iot_data_t *file_list = iot_store_config_list ("/tmp");
  iot_data_list_iter_t iter;
  iot_data_list_iter (file_list, &iter);
  bool file_found = false;
  while (iot_data_list_iter_next (&iter))
  {
    const char * file = iot_data_string (iot_data_list_iter_value (&iter));
    if (strcmp (file, "iot_test") == 0)
    {
      file_found = true;
      break;
    }
  }
  iot_data_free (file_list);
  CU_ASSERT_TRUE (file_found)
}

static void test_delete_file (void)
{
  CU_ASSERT (iot_store_delete (TEST_FILE_NAME))
}

static void test_file_exists (void)
{
  CU_ASSERT (iot_store_write (TEST_FILE_NAME, "Hello"))
  CU_ASSERT (iot_file_exists (TEST_FILE_NAME))
  CU_ASSERT (iot_store_delete (TEST_FILE_NAME))
  CU_ASSERT (! iot_file_exists (TEST_FILE_NAME))
}

static void test_file_append (void)
{
  bool ok = iot_file_write (TEST_FILE_NAME, "Hello");
  CU_ASSERT (ok)
  ok = iot_file_append (TEST_FILE_NAME, " World");
  CU_ASSERT (ok)
  char * ret = iot_file_read (TEST_FILE_NAME);
  CU_ASSERT (ret != NULL)
  if (ret)
  {
    CU_ASSERT (strcmp ("Hello World", ret) == 0)
    free (ret);
  }
}

static void test_file_append_binary (void)
{
  char buff[12u];
  bool ok = iot_file_write_binary (TEST_FILE_NAME, (const uint8_t*) "Hello", 5u);
  CU_ASSERT (ok)
  ok = iot_file_append_binary (TEST_FILE_NAME, (const uint8_t*) " World", 6u);
  CU_ASSERT (ok)
  uint8_t * ret = iot_file_read_binary (TEST_FILE_NAME, NULL);
  CU_ASSERT (ret != NULL)
  if (ret)
  {
    memcpy (buff, ret, 11u);
    buff[11] = 0;
    CU_ASSERT (strcmp ("Hello World", buff) == 0)
    free (ret);
  }
}

static _Atomic uint32_t test_file_notify_status = 0u;

static void * test_file_notify_thread (void * arg)
{
  char * file = (char*) arg;
  uint32_t status = iot_file_watch (file, iot_file_self_delete_flag | iot_file_delete_flag | iot_file_modify_flag);
  atomic_store (&test_file_notify_status, status);
  return NULL;
}

static void test_file_notify (void)
{
  pthread_t tid;
  bool ok;
  int ret;
  iot_store_delete (TEST_FILE_NAME);
  atomic_store (&test_file_notify_status, 0u);
  uint32_t status = iot_file_watch (TEST_FILE_NAME, iot_file_self_delete_flag | iot_file_delete_flag | iot_file_modify_flag);
  CU_ASSERT (status == 0u) // Case 1: File does not exist
  ok = iot_file_write (TEST_FILE_NAME, "Initial");
  CU_ASSERT (ok)
  pthread_create (&tid, NULL, test_file_notify_thread, TEST_FILE_NAME);
  iot_wait_secs (1u);
  ok = iot_file_append (TEST_FILE_NAME, "Change");
  CU_ASSERT (ok)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status == iot_file_modify_flag) // Case 2: File modified
  pthread_create (&tid, NULL, test_file_notify_thread, TEST_FILE_NAME);
  iot_wait_secs (1u);
  ok = iot_file_delete (TEST_FILE_NAME);
  CU_ASSERT (ok)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status == iot_file_self_delete_flag) // Case 3: File deleted
  ret = mkdir (TEST_SCOPED_DIR, 0700);
  CU_ASSERT (ret == 0)
  ok = iot_file_write (TEST_SCOPED_FILE_NAME, "Initial");
  CU_ASSERT (ok)
  pthread_create (&tid, NULL, test_file_notify_thread, TEST_SCOPED_DIR);
  iot_wait_secs (1u);
  ok = iot_file_delete (TEST_SCOPED_FILE_NAME);
  CU_ASSERT (ok)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status == iot_file_delete_flag) // Case 4: Directory content deleted
  pthread_create (&tid, NULL, test_file_notify_thread, TEST_SCOPED_DIR);
  iot_wait_secs (1u);
  ret = rmdir (TEST_SCOPED_DIR);
  CU_ASSERT (ret == 0)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status == iot_file_self_delete_flag) // Case 5: Directory deleted
}

#endif

void cunit_misc_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("misc", suite_init, suite_clean);
  CU_add_test (suite, "time_secs", test_time_secs);
  CU_add_test (suite, "time_msecs", test_time_msecs);
  CU_add_test (suite, "time_usecs", test_time_usecs);
  CU_add_test (suite, "time_nsecs", test_time_nsecs);
  CU_add_test (suite, "wait", test_wait);
  CU_add_test (suite, "hash", test_hash);
  CU_add_test (suite, "uuid_string", test_uuid_string);
#ifdef IOT_HAS_FILE
  CU_add_test (suite, "write_file", test_write_file);
  CU_add_test (suite, "read_file", test_read_file);
  CU_add_test (suite, "delete_file", test_delete_file);
  CU_add_test (suite, "file_exists", test_file_exists);
  CU_add_test (suite, "file_notify", test_file_notify);
  CU_add_test (suite, "file_append", test_file_append);
  CU_add_test (suite, "file_append_binary", test_file_append_binary);
#ifndef _AZURESPHERE_
  CU_add_test (suite, "list_file", test_list_file);
  CU_add_test (suite, "list_config_file", test_list_config_file);
#endif
#endif
}
