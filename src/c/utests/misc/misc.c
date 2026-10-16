/*
 * Copyright (c) 2020-2026
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/iot.h"
#include "iot/uuid.h"
#include "misc.h"
#include "CUnit.h"

#ifdef IOT_HAS_FILE
#include <pthread.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
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
  CU_ASSERT (! iot_util_string_is_uuid ("e79ebe07-0774-4a91-a33b-6a011539014\xff")) // Non ASCII characters
  CU_ASSERT (! iot_util_string_is_uuid ("\xe9""79ebe07-0774-4a91-a33b-6a0115390141"))
}

#define TEST_UUIDS 1000

static void test_uuid_generate (void)
{
  // Generated UUIDs are version 4 (random), unique and unparse to valid strings
  static iot_uuid_t uuids[TEST_UUIDS];
  char str[UUID_STR_LEN];
  for (unsigned i = 0; i < TEST_UUIDS; i++)
  {
    iot_uuid_generate (uuids[i]);
    CU_ASSERT ((uuids[i][6] & 0xf0) == 0x40)
    CU_ASSERT ((uuids[i][8] & 0xc0) == 0x80)
    for (unsigned j = 0; j < i; j++)
    {
      CU_ASSERT (memcmp (uuids[i], uuids[j], sizeof (iot_uuid_t)) != 0)
    }
  }
  iot_uuid_unparse (uuids[0], str);
  CU_ASSERT (strlen (str) == UUID_STR_LEN - 1)
  CU_ASSERT (iot_util_string_is_uuid (str))
  CU_ASSERT (str[14] == '4')
}

static void test_uuid_fork (void)
{
  // A forked child process does not generate the same UUIDs as its parent
  iot_uuid_t parent, child;
  int fds[2];
  iot_uuid_generate (parent);
  CU_ASSERT_FATAL (pipe (fds) == 0)
  pid_t pid = fork ();
  CU_ASSERT_FATAL (pid >= 0)
  if (pid == 0)
  {
    iot_uuid_generate (child);
    _exit (write (fds[1], child, sizeof (child)) == sizeof (child) ? 0 : 1);
  }
  close (fds[1]);
  iot_uuid_generate (parent);
  CU_ASSERT (read (fds[0], child, sizeof (child)) == sizeof (child))
  close (fds[0]);
  int status;
  waitpid (pid, &status, 0);
  CU_ASSERT (WIFEXITED (status) && WEXITSTATUS (status) == 0)
  CU_ASSERT (memcmp (parent, child, sizeof (iot_uuid_t)) != 0)
}

#ifdef IOT_HAS_FILE

#define TEST_FILE_NAME "/tmp/iot_test.json"
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

#define TEST_REPLACE_DIR "/tmp/_iot_replace"
#define TEST_REPLACE_FILE TEST_REPLACE_DIR "/config.json"

static uint32_t test_dir_count (const char * dir)
{
  iot_data_t * list = iot_file_list (dir, NULL);
  uint32_t count = list ? iot_data_list_length (list) : 0u;
  iot_data_free (list);
  return count;
}

static void test_file_replace_mode (void)
{
  // Rewriting an existing file keeps its permissions, and leaves no temporary file
  mkdir (TEST_REPLACE_DIR, 0700);
  CU_ASSERT (iot_file_write (TEST_REPLACE_FILE, "first"))
  CU_ASSERT (chmod (TEST_REPLACE_FILE, 0600) == 0)
  CU_ASSERT (iot_file_write (TEST_REPLACE_FILE, "second"))
  struct stat st;
  CU_ASSERT (stat (TEST_REPLACE_FILE, &st) == 0 && (st.st_mode & 0777) == 0600)
  char * str = iot_file_read (TEST_REPLACE_FILE);
  CU_ASSERT (str && strcmp (str, "second") == 0)
  free (str);
  CU_ASSERT (test_dir_count (TEST_REPLACE_DIR) == 1u)
  iot_file_delete (TEST_REPLACE_FILE);
  rmdir (TEST_REPLACE_DIR);
}

static void test_file_write_empty (void)
{
  // Empty content can be written, replacing existing content
  size_t len = 99u;
  CU_ASSERT (iot_file_write (TEST_FILE_NAME, "data"))
  CU_ASSERT (iot_file_write (TEST_FILE_NAME, ""))
  uint8_t * bin = iot_file_read_binary (TEST_FILE_NAME, &len);
  CU_ASSERT (bin && len == 0u && bin[0] == 0)
  free (bin);
  CU_ASSERT (iot_file_write_binary (TEST_FILE_NAME, NULL, 0u))
  CU_ASSERT (iot_file_append_binary (TEST_FILE_NAME, NULL, 0u))
  bin = iot_file_read_binary (TEST_FILE_NAME, &len);
  CU_ASSERT (bin && len == 0u)
  free (bin);
  iot_file_delete (TEST_FILE_NAME);
}

static void test_file_large (void)
{
  // Binary content larger than read buffer is read back intact
  size_t size = 100000u;
  size_t len = 0u;
  uint8_t * data = malloc (size);
  for (size_t i = 0; i < size; i++) data[i] = (uint8_t) (i * 7u);
  CU_ASSERT (iot_file_write_binary (TEST_FILE_NAME, data, size))
  uint8_t * bin = iot_file_read_binary (TEST_FILE_NAME, &len);
  CU_ASSERT (bin && len == size && memcmp (bin, data, size) == 0)
  free (bin);
  free (data);
  iot_file_delete (TEST_FILE_NAME);
}

#ifdef __linux__
static void test_file_read_special (void)
{
  // Special files report an incorrect size, so must be read until end of file
  size_t len = 0u;
  uint8_t * bin = iot_file_read_binary ("/proc/self/status", &len);
  CU_ASSERT (bin && len > 0u && strlen ((const char*) bin) == len && strncmp ((const char*) bin, "Name:", 5u) == 0)
  free (bin);
  CU_ASSERT (! iot_file_append ("/dev/full", "x")) // Write error reported (only detected on flush)
  CU_ASSERT (! iot_file_write ("/dev/full", "x")) // Device file written directly, not replaced (even if privileged)
}

static void test_file_write_symlink (void)
{
  // Writing via a symbolic link updates the target file, keeping the link
  const char * link = TEST_FILE_NAME ".link";
  struct stat st;
  CU_ASSERT (iot_file_write (TEST_FILE_NAME, "first"))
  unlink (link);
  CU_ASSERT (symlink (TEST_FILE_NAME, link) == 0)
  CU_ASSERT (iot_file_write (link, "second"))
  CU_ASSERT (lstat (link, &st) == 0 && S_ISLNK (st.st_mode))
  char * str = iot_file_read (TEST_FILE_NAME);
  CU_ASSERT (str && strcmp (str, "second") == 0)
  free (str);
  unlink (link);
  iot_file_delete (TEST_FILE_NAME);
}

static uint32_t test_fd_count (void)
{
  uint32_t count = 0u;
  DIR * dir = opendir ("/proc/self/fd"); // Entries are symbolic links, so not listed by iot_file_list
  while (dir && readdir (dir)) count++;
  if (dir) closedir (dir);
  return count;
}

static void test_list_file_bad_regex (void)
{
  // Invalid regex returns NULL without leaking the directory
  uint32_t before = test_fd_count ();
  for (uint32_t i = 0; i < 10u; i++) CU_ASSERT (iot_file_list ("/tmp", "[unclosed") == NULL)
  CU_ASSERT (test_fd_count () == before)
}
#endif

static void test_file_long_path (void)
{
  // Existing file path too long for temporary file path (path + ".XXXXXX") fails cleanly, leaving file unchanged
  char path[PATH_MAX];
  const char * name = "/iot_test.json";
  CU_ASSERT (iot_file_write (TEST_FILE_NAME, "original"))
  strcpy (path, "/tmp");
  while (strlen (path) + strlen (name) < PATH_MAX - 6u) strcat (path, "/."); // Same file, via padded path
  strcat (path, name);
  CU_ASSERT (iot_file_exists (path))
  CU_ASSERT (! iot_file_write (path, "x"))
  char * str = iot_file_read (TEST_FILE_NAME);
  CU_ASSERT (str && strcmp (str, "original") == 0)
  free (str);
  iot_file_delete (TEST_FILE_NAME);
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

static void test_list_config_filter (void)
{
  // Only non empty names with a .json extension are listed, and a missing directory returns NULL
  static const char * files[] = { "a.json", "b.json", "cxjson", ".json", "d.json.bak", "e.JSON", "fjson" };
  char dir[] = "/tmp/iot_store_XXXXXX";
  char path[64];
  CU_ASSERT_FATAL (mkdtemp (dir) != NULL)
  for (uint32_t i = 0; i < sizeof (files) / sizeof (files[0]); i++)
  {
    snprintf (path, sizeof (path), "%s/%s", dir, files[i]);
    CU_ASSERT (iot_file_write (path, "{}"))
  }
  iot_data_t * list = iot_store_config_list (dir);
  CU_ASSERT (iot_data_list_length (list) == 2)
  iot_data_list_iter_t iter;
  iot_data_list_iter (list, &iter);
  while (iot_data_list_iter_next (&iter))
  {
    const char * name = iot_data_list_iter_string_value (&iter);
    CU_ASSERT (strcmp (name, "a") == 0 || strcmp (name, "b") == 0)
  }
  iot_data_free (list);
  for (uint32_t i = 0; i < sizeof (files) / sizeof (files[0]); i++)
  {
    snprintf (path, sizeof (path), "%s/%s", dir, files[i]);
    iot_file_delete (path);
  }
  rmdir (dir);
  CU_ASSERT (iot_store_config_list (dir) == NULL)
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
static uint32_t test_file_notify_mask = 0u;

static void * test_file_notify_thread (void * arg)
{
  char * file = (char*) arg;
  uint32_t status = iot_file_watch (file, test_file_notify_mask);
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
  test_file_notify_mask = iot_file_self_delete_flag | iot_file_delete_flag | iot_file_modify_flag;
  uint32_t status = iot_file_watch (TEST_FILE_NAME, test_file_notify_status);
  CU_ASSERT (status == 0u) // Case 1: File does not exist

  ok = iot_file_write (TEST_FILE_NAME, "Initial");
  CU_ASSERT (ok)
  pthread_create (&tid, NULL, test_file_notify_thread, TEST_FILE_NAME);
  iot_wait_secs (1u);
  ok = iot_file_append (TEST_FILE_NAME, "Change");
  CU_ASSERT (ok)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status & iot_file_modify_flag) // Case 2: File modified

  pthread_create (&tid, NULL, test_file_notify_thread, TEST_FILE_NAME);
  iot_wait_secs (1u);
  ok = iot_file_delete (TEST_FILE_NAME);
  CU_ASSERT (ok)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status & iot_file_self_delete_flag) // Case 3: File deleted

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
  CU_ASSERT (status & iot_file_delete_flag) // Case 4: Directory content deleted

  pthread_create (&tid, NULL, test_file_notify_thread, TEST_SCOPED_DIR);
  iot_wait_secs (1u);
  ret = rmdir (TEST_SCOPED_DIR);
  CU_ASSERT (ret == 0)
  pthread_join (tid, NULL);
  status = atomic_load (&test_file_notify_status);
  CU_ASSERT (status & iot_file_self_delete_flag) // Case 5: Directory deleted
}

#endif

static void test_file_read_cases (void)
{
  // Special file larger than the initial read buffer read completely. Reading a directory fails
  char * str = iot_file_read ("/proc/self/smaps");
  CU_ASSERT (str && strlen (str) > 4096u)
  free (str);
  CU_ASSERT (iot_file_read ("/tmp") == NULL)
}

static void test_store_config_files (void)
{
  // Configuration saved, loaded and deleted as name.json in the URI directory, or in the current directory if no URI
  CU_ASSERT (iot_store_config_save ("iot_store_cfg", "/tmp", "{\"a\":1}"))
  CU_ASSERT (iot_file_exists ("/tmp/iot_store_cfg.json"))
  char * str = iot_store_config_load ("iot_store_cfg", "/tmp");
  CU_ASSERT (str && strcmp (str, "{\"a\":1}") == 0)
  free (str);
  CU_ASSERT (iot_store_config_delete ("iot_store_cfg", "/tmp"))
  CU_ASSERT (! iot_file_exists ("/tmp/iot_store_cfg.json"))
  CU_ASSERT (iot_store_config_load ("iot_store_cfg", "/tmp") == NULL)

  CU_ASSERT (iot_store_config_save ("iot_store_cfg", NULL, "{}"))
  CU_ASSERT (iot_file_exists ("./iot_store_cfg.json"))
  str = iot_store_config_load ("iot_store_cfg", NULL);
  CU_ASSERT (str && strcmp (str, "{}") == 0)
  free (str);
  CU_ASSERT (iot_store_config_delete ("iot_store_cfg", NULL))
  CU_ASSERT (! iot_file_exists ("./iot_store_cfg.json"))
}

static unsigned test_store_reads = 0u;
static unsigned test_store_writes = 0u;
static unsigned test_store_deletes = 0u;

static uint8_t * test_store_read (const char * path, size_t * len)
{
  (void) path;
  test_store_reads++;
  if (len) *len = 2u;
  return (uint8_t*) strdup ("{}");
}

static bool test_store_write (const char * path, const uint8_t * binary, size_t len)
{
  (void) path;
  (void) binary;
  (void) len;
  test_store_writes++;
  return true;
}

static bool test_store_delete (const char * path)
{
  (void) path;
  test_store_deletes++;
  return true;
}

static void test_store_config_functions (void)
{
  // Store uses configured functions. Without functions, reads return NULL and writes and deletes fail
  iot_store_config (test_store_read, test_store_write, test_store_delete);
  char * str = iot_store_read ("any");
  CU_ASSERT (str && strcmp (str, "{}") == 0)
  free (str);
  CU_ASSERT (iot_store_write ("any", "x"))
  CU_ASSERT (iot_store_delete ("any"))
  CU_ASSERT (test_store_reads == 1u && test_store_writes == 1u && test_store_deletes == 1u)
  iot_store_config (NULL, NULL, NULL);
  CU_ASSERT (iot_store_read ("any") == NULL)
  CU_ASSERT (! iot_store_write ("any", "x"))
  CU_ASSERT (! iot_store_delete ("any"))
  iot_store_config (iot_file_read_binary, iot_file_write_binary, iot_file_delete); // Restore default file store
  CU_ASSERT (iot_store_write (TEST_FILE_NAME, "restored"))
  str = iot_store_read (TEST_FILE_NAME);
  CU_ASSERT (str && strcmp (str, "restored") == 0)
  free (str);
  CU_ASSERT (iot_store_delete (TEST_FILE_NAME))
}

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
  CU_add_test (suite, "uuid_generate", test_uuid_generate);
  CU_add_test (suite, "uuid_fork", test_uuid_fork);
#ifdef IOT_HAS_FILE
  CU_add_test (suite, "write_file", test_write_file);
  CU_add_test (suite, "read_file", test_read_file);
  CU_add_test (suite, "delete_file", test_delete_file);
  CU_add_test (suite, "file_exists", test_file_exists);
  CU_add_test (suite, "file_notify", test_file_notify);
  CU_add_test (suite, "file_append", test_file_append);
  CU_add_test (suite, "file_append_binary", test_file_append_binary);
  CU_add_test (suite, "list_file", test_list_file);
  CU_add_test (suite, "list_config_file", test_list_config_file);
  CU_add_test (suite, "list_config_filter", test_list_config_filter);
  CU_add_test (suite, "file_replace_mode", test_file_replace_mode);
  CU_add_test (suite, "file_write_empty", test_file_write_empty);
  CU_add_test (suite, "file_large", test_file_large);
  CU_add_test (suite, "file_long_path", test_file_long_path);
#ifdef __linux__
  CU_add_test (suite, "file_read_special", test_file_read_special);
  CU_add_test (suite, "file_write_symlink", test_file_write_symlink);
  CU_add_test (suite, "list_file_bad_regex", test_list_file_bad_regex);
  CU_add_test (suite, "file_read_cases", test_file_read_cases);
  CU_add_test (suite, "store_config_files", test_store_config_files);
  CU_add_test (suite, "store_config_functions", test_store_config_functions);
#endif
#endif
}
