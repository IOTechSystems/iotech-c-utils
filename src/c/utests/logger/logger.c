/*
 * Copyright (c) 2020-2026
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger.h"
#include "iot/container.h"
#include "iot/time.h"
#include "iot/file.h"
#include "CUnit.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sched.h>

#define IOT_TEST_LOGGER_TYPE "IOT::UTEST::Logger"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

static uint32_t cunit_custom_log_count = 0;

static const char * logger1_config =
"{"
  "\"Name\":\"logger1\","
  "\"Level\":\"Warning\""
"}";

static const char * logger2_config =
"{"
  "\"Name\":\"logger2\","
  "\"Next\":\"logger1\","
  "\"Level\":\"Info\""
"}";

static const char * logger3_config =
"{"
  "\"Name\":\"logger3\","
  "\"Next\":\"logger2\","
  "\"Level\":\"Debug\""
"}";

static void cunit_custom_log_fn (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  (void) logger;
  (void) level;
  (void) timestamp;
  (void) message;
  (void) ctx;
  cunit_custom_log_count++;
}

static void cunit_test_logs (iot_logger_t * logger)
{
  iot_log_warn (logger, "Test Warning");
  iot_log_error (logger, "Test Error");
  iot_log_trace (logger, "Test Trace");
  iot_log_debug (logger, "Test Debug");
  iot_log_info (logger, "Test Error");
}

static void cunit_logger_default (void)
{
  iot_logger_t * logger = iot_logger_default ();
  iot_logger_start (logger);
  iot_log_warn (logger, "Test Warning from default logger");
  iot_logger_free (logger); /* Should ignore default logger */
  iot_log_error (logger, "Test Error from default logger");
  iot_log_trace (logger, "ERROR should not see trace log");
  iot_log_debug (logger, "ERROR should not see debug log");
  iot_log_info (logger, "ERROR should not see info log");
}

static void cunit_logger_named (void)
{
  iot_logger_t * logger = iot_logger_alloc ("MyLogger", IOT_LOG_ERROR, true);
  iot_log_error (logger, "Test Error from my logger");
  iot_log_warn (logger, "ERROR should not see warn log");
  iot_log_trace (logger, "ERROR should not see trace log");
  iot_log_debug (logger, "ERROR should not see debug log");
  iot_log_info (logger, "ERROR should not see info log");
  iot_logger_free (logger);
}

static void cunit_logger_impl (void)
{
  iot_logger_t * logger = iot_logger_alloc_custom ("Sub", IOT_LOG_NONE, false, NULL, cunit_custom_log_fn, NULL, NULL);
  iot_logger_start (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 0)
  iot_logger_set_level (logger, IOT_LOG_ERROR);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 1)
  iot_logger_set_level (logger, IOT_LOG_WARN);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 3)
  iot_logger_set_level (logger, IOT_LOG_INFO);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 6)
  iot_logger_set_level (logger, IOT_LOG_DEBUG);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 10)
  iot_logger_set_level (logger, IOT_LOG_TRACE);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 15)
  iot_logger_free (logger);
}

static void cunit_logger_sub (void)
{
  iot_logger_t * sub = iot_logger_default ();
  iot_logger_t * logger = iot_logger_alloc_custom ("Custom", IOT_LOG_INFO, false, NULL, cunit_custom_log_fn, sub, NULL);
  iot_logger_start (sub);
  iot_logger_start (logger);
  iot_logger_set_level (sub, IOT_LOG_NONE);
  cunit_custom_log_count = 0;
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 3)
  iot_logger_free (logger);
}

static void cunit_logger_file (void)
{
  iot_logger_t * logger = iot_logger_alloc_file ("File", IOT_LOG_WARN, false, NULL, "./test.log");
  iot_logger_start (logger);
  cunit_test_logs (logger);
  iot_logger_free (logger);
}

static bool cunit_log_file_contains (const char * path, const char * str)
{
  char * contents = iot_file_read (path);
  bool found = contents && strstr (contents, str);
  free (contents);
  return found;
}

static void cunit_logger_file_flush (void)
{
  // Logs are written to the file as logged, not only when the logger is freed
  iot_file_delete ("./test_flush.log");
  iot_logger_t * logger = iot_logger_alloc_file ("File", IOT_LOG_WARN, true, NULL, "./test_flush.log");
  iot_log_error (logger, "Flushed error");
  CU_ASSERT (cunit_log_file_contains ("./test_flush.log", ":File:ERROR] Flushed error\n"))
  iot_log_warn (logger, "Flushed warning");
  CU_ASSERT (cunit_log_file_contains ("./test_flush.log", ":File:WARN] Flushed warning\n"))
  iot_logger_free (logger);
  iot_file_delete ("./test_flush.log");
}

static void cunit_logger_no_name (void)
{
  // Loggers allocated or configured without a name use name "default"
  iot_file_delete ("./test_no_name.log");
  iot_logger_t * logger = iot_logger_alloc_file (NULL, IOT_LOG_WARN, true, NULL, "./test_no_name.log");
  iot_log_error (logger, "Allocated without name");
  CU_ASSERT (cunit_log_file_contains ("./test_no_name.log", ":default:ERROR] Allocated without name\n"))
  iot_logger_free (logger);

  iot_component_factory_add (iot_logger_factory ());
  iot_container_t * cont = iot_container_alloc ("logger_no_name");
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "nameless", "{\"Level\":\"Warning\",\"To\":\"file:./test_no_name.log\"}");
  logger = (iot_logger_t*) iot_container_find_component (cont, "nameless");
  CU_ASSERT_FATAL (logger != NULL)
  iot_log_error (logger, "Configured without name");
  CU_ASSERT (cunit_log_file_contains ("./test_no_name.log", ":default:ERROR] Configured without name\n"))
  iot_container_free (cont);
  iot_file_delete ("./test_no_name.log");
}

static void cunit_logger_file_open_fail (void)
{
  // File logger not created if file cannot be opened, with error logged to next logger
  iot_logger_t * next = iot_logger_alloc_custom ("Next", IOT_LOG_ERROR, true, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0u;
  CU_ASSERT (iot_logger_alloc_file ("File", IOT_LOG_WARN, true, next, "./no_such_dir/test.log") == NULL)
  CU_ASSERT (cunit_custom_log_count == 1u)
  iot_logger_free (next);

  iot_component_factory_add (iot_logger_factory ());
  iot_container_t * cont = iot_container_alloc ("logger_file_open_fail");
  iot_container_add_component (cont, IOT_LOGGER_TYPE, "file", "{\"Name\":\"file\",\"To\":\"file:./no_such_dir/test.log\"}");
  CU_ASSERT (iot_container_find_component (cont, "file") == NULL)
  iot_container_free (cont);
}

static void cunit_logger_console_flush (void)
{
  // Console logs written as logged, even if stdout is fully buffered (as when not a terminal). Checked in a child
  // process, as CUnit makes stdout unbuffered
  fflush (stdout);
  pid_t pid = fork ();
  CU_ASSERT_FATAL (pid >= 0)
  if (pid == 0)
  {
    int fd = open ("./test_console.log", O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0 || dup2 (fd, STDOUT_FILENO) < 0) _exit (2);
    static char buff[BUFSIZ];
    setvbuf (stdout, buff, _IOFBF, sizeof (buff)); // Own buffer, as with NULL glibc keeps the unbuffered single byte buffer
    iot_logger_t * logger = iot_logger_alloc ("Console", IOT_LOG_INFO, true);
    iot_log_info (logger, "Flushed info");
    _exit (cunit_log_file_contains ("./test_console.log", ":Console:Info] Flushed info\n") ? 0 : 1); // Exit without flushing stdout
  }
  int status = 0;
  waitpid (pid, &status, 0);
  CU_ASSERT (WIFEXITED (status) && WEXITSTATUS (status) == 0)
  iot_file_delete ("./test_console.log");
}

static void cunit_logger_udp (void)
{
  iot_logger_t * logger = iot_logger_alloc_udp ("udp", IOT_LOG_WARN, false, NULL, "localhost", 22222);
  iot_logger_start (logger);
  cunit_test_logs (logger);
  iot_logger_free (logger);
}

static void cunit_logger_udp_long (void)
{
  // Long messages are truncated to the log buffer (ending with newline), not sent beyond it (checked by valgrind)
  int sock = socket (AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in addr = { .sin_family = AF_INET, .sin_addr.s_addr = htonl (INADDR_LOOPBACK) };
  socklen_t alen = sizeof (addr);
  const struct timeval timeout = { .tv_sec = 5 };
  CU_ASSERT (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (timeout)) == 0) // Fail rather than hang if message not received
  CU_ASSERT (bind (sock, (struct sockaddr*) &addr, sizeof (addr)) == 0)
  getsockname (sock, (struct sockaddr*) &addr, &alen);
  iot_logger_t * logger = iot_logger_alloc_udp ("udp", IOT_LOG_WARN, true, NULL, "127.0.0.1", ntohs (addr.sin_port));
  char msg[3000];
  memset (msg, 'x', sizeof (msg) - 1u);
  msg[sizeof (msg) - 1u] = '\0';
  iot_log_error (logger, "%s", msg);
  char buff[4096];
  ssize_t len = recv (sock, buff, sizeof (buff), 0);
  CU_ASSERT (len > 0 && len < 2000 && buff[len - 1] == '\n')
  iot_logger_free (logger);
  close (sock);
}

static void cunit_logger_udp_host (void)
{
  // Host names are resolved, long host names do not overflow, and unresolvable hosts fail.
  // Invalid names have a label longer than 63 characters, so fail without a DNS query (which can take many seconds without network)
  iot_logger_t * logger = iot_logger_alloc_udp ("udp", IOT_LOG_WARN, false, NULL, "localhost", 22222);
  CU_ASSERT (logger != NULL)
  iot_logger_free (logger);
  CU_ASSERT (iot_logger_alloc_udp ("udp", IOT_LOG_WARN, false, NULL, "no-such-host-with-a-label-that-is-longer-than-the-dns-maximum-length.invalid", 22222) == NULL)
  iot_container_t * cont = iot_container_alloc ("udp_host");
  iot_data_t * config = iot_data_from_json ("{\"Name\":\"udp\",\"To\":\"udp:a-very-long-host-name-with-a-label-that-is-longer-than-the-dns-maximum.invalid:5000\"}");
  CU_ASSERT (iot_logger_factory ()->config_fn (cont, config) == NULL)
  iot_data_free (config);
  config = iot_data_from_json ("{\"Name\":\"udp\",\"To\":\"udp:localhost:5000\"}");
  iot_component_t * comp = iot_logger_factory ()->config_fn (cont, config);
  CU_ASSERT (comp != NULL)
  iot_logger_free ((iot_logger_t*) comp);
  iot_data_free (config);
  iot_container_free (cont);
}

static void cunit_logger_udp_broadcast (void)
{
  iot_logger_t * logger = iot_logger_alloc_udp ("udp-broadcast", IOT_LOG_WARN, false, NULL, NULL, 33333);
  iot_logger_start (logger);
  cunit_test_logs (logger);
  iot_logger_free (logger);
}

static void cunit_logger_null (void)
{
  cunit_test_logs (NULL); // Should be able to have logger as NULL
  iot_logger_free (NULL);
}

static void cunit_logger_start_stop (void)
{
  iot_logger_t * logger = iot_logger_alloc_custom ("StartStop", IOT_LOG_WARN, false, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0;
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 0)
  iot_logger_start (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 2)
  iot_logger_stop (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 2)
  iot_logger_start (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 4)
  iot_logger_free (logger);
}

static void cunit_logger_refcount (void)
{
  iot_logger_t *logger = iot_logger_alloc_custom ("StartStop", IOT_LOG_WARN, false, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0;
  iot_logger_add_ref (logger);
  iot_logger_start (logger);
  iot_logger_free (logger);
  cunit_test_logs (logger);
  CU_ASSERT (cunit_custom_log_count == 2)
  iot_logger_free (logger);
}

static void cunit_logger_selfstart (void)
{
  iot_logger_t *logger = iot_logger_alloc ("MyLogger", IOT_LOG_TRACE, true);
  iot_log_trace (logger, "TRACE: Test logger start");
  iot_logger_free (logger);
}

static void cunit_logger_format (void)
{
  iot_logger_t *logger = iot_logger_alloc ("MyLogger", IOT_LOG_INFO, true);
  iot_log_info (logger, "INFO: %s %d", "testarg", 1234);
  iot_logger_free (logger);
}

static void cunit_logger_set_next (void)
{
  iot_logger_t * logger = iot_logger_alloc ("MyLogger", IOT_LOG_INFO, true);
  iot_logger_t * next = iot_logger_alloc_custom ("StartStop", IOT_LOG_ERROR, true, NULL, cunit_custom_log_fn, NULL, NULL);
  cunit_custom_log_count = 0u;
  iot_logger_set_next (logger, next);
  iot_log_info (logger, "Info: set_next");
  iot_log_error (logger, "Error: set_next");
  CU_ASSERT (cunit_custom_log_count == 1u)
  iot_logger_free (logger);
  iot_logger_free (next);
}

static void cunit_logger_level_name (void)
{
  CU_ASSERT (strcmp ("", iot_logger_level_to_string (IOT_LOG_NONE)) == 0)
  CU_ASSERT (strcmp ("ERROR", iot_logger_level_to_string (IOT_LOG_ERROR)) == 0)
  CU_ASSERT (strcmp ("WARN", iot_logger_level_to_string (IOT_LOG_WARN)) == 0)
  CU_ASSERT (strcmp ("Info", iot_logger_level_to_string (IOT_LOG_INFO)) == 0)
  CU_ASSERT (strcmp ("Debug", iot_logger_level_to_string (IOT_LOG_DEBUG)) == 0)
  CU_ASSERT (strcmp ("Trace", iot_logger_level_to_string (IOT_LOG_TRACE)) == 0)
  CU_ASSERT (iot_logger_level_from_string ("Error") == IOT_LOG_ERROR)
  CU_ASSERT (iot_logger_level_from_string ("warn") == IOT_LOG_WARN)
  CU_ASSERT (iot_logger_level_from_string ("trace") == IOT_LOG_TRACE)
  CU_ASSERT (iot_logger_level_from_string (NULL) == IOT_LOGLEVEL_DEFAULT)
  CU_ASSERT (iot_logger_level_from_string ("\xc9rror") == IOT_LOGLEVEL_DEFAULT) // Non ASCII character
  CU_ASSERT (iot_logger_level_from_string ("\xff") == IOT_LOGLEVEL_DEFAULT)
}

static iot_component_t * cunit_logger_chained_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t *result;
  iot_loglevel_t level = iot_logger_level_from_string (iot_data_string_map_get_string (map, "Level"));
  iot_logger_t * next = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Next"));
  bool start = iot_data_string_map_get_bool (map, "Start", true);
  const char * name = iot_data_string_map_get_string (map, "Name");
  result = iot_logger_alloc_custom (name, level, start, next, cunit_custom_log_fn, NULL, NULL);
  return (iot_component_t*) result;
}

static atomic_uint cunit_count_a;
static atomic_uint cunit_count_b;

static void cunit_count_log_a (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void * ctx)
{
  (void) logger; (void) level; (void) timestamp; (void) message; (void) ctx;
  atomic_fetch_add (&cunit_count_a, 1u);
}

static void cunit_count_log_b (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void * ctx)
{
  (void) logger; (void) level; (void) timestamp; (void) message; (void) ctx;
  atomic_fetch_add (&cunit_count_b, 1u);
}

static void cunit_logger_chain_no_container (void)
{
  // Loggers chained without a container: level changes propagate, next can be changed or removed
  iot_logger_t * b = iot_logger_alloc_custom ("b", IOT_LOG_ERROR, true, NULL, cunit_count_log_b, NULL, NULL);
  iot_logger_t * a = iot_logger_alloc_custom ("a", IOT_LOG_ERROR, true, b, cunit_count_log_a, NULL, NULL);
  iot_logger_set_level (b, IOT_LOG_WARN); // Previously crashed (parent looked up in container)
  CU_ASSERT (a->level == IOT_LOG_WARN)
  atomic_store (&cunit_count_a, 0u);
  atomic_store (&cunit_count_b, 0u);
  iot_log_warn (a, "warn");
  CU_ASSERT (atomic_load (&cunit_count_a) == 0u && atomic_load (&cunit_count_b) == 1u)
  iot_logger_set_next (a, NULL); // Detach
  CU_ASSERT (a->level == IOT_LOG_ERROR)
  iot_log_warn (a, "warn");
  CU_ASSERT (atomic_load (&cunit_count_b) == 1u)
  iot_logger_set_next (a, b); // Reattach updates effective level
  // cppcheck-suppress nullPointerRedundantCheck ; Not NULL, but iot_log_warn above checks logger is not NULL
  CU_ASSERT (a->level == IOT_LOG_WARN)
  iot_logger_free (a);
  iot_logger_free (b);
}

static void cunit_logger_chain_level (void)
{
  // Messages pass along chain to any later logger at level, including one not started, and stop when none is
  iot_logger_t * c = iot_logger_alloc_custom ("c", IOT_LOG_NONE, false, NULL, cunit_count_log_b, NULL, NULL);
  iot_logger_t * b = iot_logger_alloc_custom ("b", IOT_LOG_ERROR, true, c, cunit_count_log_a, NULL, NULL);
  iot_logger_t * a = iot_logger_alloc_custom ("a", IOT_LOG_ERROR, true, b, cunit_count_log_a, NULL, NULL);
  atomic_store (&cunit_count_a, 0u);
  atomic_store (&cunit_count_b, 0u);
  iot_log__log (a, IOT_LOG_WARN, "warn");
  CU_ASSERT (atomic_load (&cunit_count_a) == 0u && atomic_load (&cunit_count_b) == 0u)
  iot_logger_set_level (c, IOT_LOG_WARN); // Not started, so effective levels unchanged, but chain level updated
  iot_log__log (a, IOT_LOG_WARN, "warn");
  CU_ASSERT (atomic_load (&cunit_count_a) == 0u && atomic_load (&cunit_count_b) == 1u)
  iot_logger_set_level (c, IOT_LOG_ERROR);
  iot_log__log (a, IOT_LOG_WARN, "warn");
  iot_log__log (a, IOT_LOG_ERROR, "error");
  CU_ASSERT (atomic_load (&cunit_count_a) == 2u && atomic_load (&cunit_count_b) == 2u)
  iot_logger_free (a);
  iot_logger_free (b);
  iot_logger_free (c);
}

static void cunit_logger_default_next_refs (void)
{
  // Logging through a chain to the default logger does not change the default logger reference count
  iot_logger_t * dfl = iot_logger_default ();
  iot_logger_t * a = iot_logger_alloc_custom ("a", IOT_LOG_ERROR, true, dfl, cunit_count_log_a, NULL, NULL);
  int_fast32_t refs = atomic_load (&dfl->component.refs);
  iot_log__log (a, IOT_LOG_ERROR, "error");
  CU_ASSERT (atomic_load (&dfl->component.refs) == refs)
  iot_logger_free (a);
}

#define CUNIT_LOG_THREADS 4
#define CUNIT_LOG_MESSAGES 2000

static atomic_uint cunit_log_done;

static void * cunit_log_thread (void * arg)
{
  iot_logger_t * logger = arg;
  for (int i = 0; i < CUNIT_LOG_MESSAGES; i++) iot_log_error (logger, "message");
  atomic_fetch_add (&cunit_log_done, 1u);
  return NULL;
}

static void cunit_logger_set_next_concurrent (void)
{
  // Next logger replaced while other threads log through the chain (use after free checked by valgrind)
  iot_logger_t * a = iot_logger_alloc_custom ("a", IOT_LOG_ERROR, true, NULL, cunit_count_log_a, NULL, NULL);
  pthread_t threads[CUNIT_LOG_THREADS];
  atomic_store (&cunit_count_a, 0u);
  atomic_store (&cunit_count_b, 0u);
  atomic_store (&cunit_log_done, 0u);
  for (int i = 0; i < CUNIT_LOG_THREADS; i++) pthread_create (&threads[i], NULL, cunit_log_thread, a);
  while (atomic_load (&cunit_log_done) < CUNIT_LOG_THREADS) // Replace next logger until all logging done
  {
    iot_logger_t * next = iot_logger_alloc_custom ("next", IOT_LOG_ERROR, true, NULL, cunit_count_log_b, NULL, NULL);
    iot_logger_set_next (a, next);
    iot_logger_free (next); // Reference now held by a only
    sched_yield ();
  }
  for (int i = 0; i < CUNIT_LOG_THREADS; i++) pthread_join (threads[i], NULL);
  iot_logger_set_next (a, NULL);
  CU_ASSERT (atomic_load (&cunit_count_a) == CUNIT_LOG_THREADS * CUNIT_LOG_MESSAGES)
  iot_logger_free (a);
}

static void cunit_logger_chained (void)
{
  static iot_component_factory_t f = { IOT_TEST_LOGGER_TYPE, IOT_CATEGORY_CORE, cunit_logger_chained_config, NULL, NULL, NULL }; // Static, as factories remain registered after test
  f.free_fn = iot_logger_factory ()->free_fn;
  f.reconfig_fn = iot_logger_factory ()->reconfig_fn;
  iot_component_factory_add (&f);
  iot_container_t *cont = iot_container_alloc ("logger_chained");
  iot_container_add_component (cont, IOT_TEST_LOGGER_TYPE, "logger1", logger1_config);
  iot_container_add_component (cont, IOT_TEST_LOGGER_TYPE, "logger2", logger2_config);
  iot_container_add_component (cont, IOT_TEST_LOGGER_TYPE, "logger3", logger3_config);
  iot_logger_t * logger1 = (iot_logger_t *)iot_container_find_component (cont, "logger1");
  iot_logger_t * logger3 = (iot_logger_t *)iot_container_find_component (cont, "logger3");

  cunit_custom_log_count = 0u;
  cunit_test_logs (logger3);
  CU_ASSERT (cunit_custom_log_count == 9u)

  iot_logger_set_level (logger1, IOT_LOG_DEBUG);
  iot_logger_set_level (logger3, IOT_LOG_WARN);

  cunit_custom_log_count = 0u;
  cunit_test_logs (logger3);
  CU_ASSERT (cunit_custom_log_count == 9u)

  iot_container_free (cont);
}

static pthread_barrier_t cunit_default_barrier;

static void * cunit_default_thread (void * arg)
{
  pthread_barrier_wait (&cunit_default_barrier);
  *(iot_logger_t **) arg = iot_logger_default ();
  return NULL;
}

static void cunit_logger_default_concurrent (void)
{
  // Concurrent first use returns the same initialised logger, which logs with name "default"
  enum { THREADS = 4 };
  pthread_t tids[THREADS];
  iot_logger_t * loggers[THREADS];
  pthread_barrier_init (&cunit_default_barrier, NULL, THREADS);
  for (int i = 0; i < THREADS; i++) pthread_create (&tids[i], NULL, cunit_default_thread, &loggers[i]);
  for (int i = 0; i < THREADS; i++)
  {
    pthread_join (tids[i], NULL);
    CU_ASSERT (loggers[i] == iot_logger_default ())
  }
  pthread_barrier_destroy (&cunit_default_barrier);

  char path[] = "/tmp/iot_logger_XXXXXX";
  int fd = mkstemp (path);
  CU_ASSERT_FATAL (fd >= 0)
  fflush (stderr);
  int saved = dup (STDERR_FILENO);
  dup2 (fd, STDERR_FILENO);
  iot_log_warn (iot_logger_default (), "default logger message");
  fflush (stderr);
  dup2 (saved, STDERR_FILENO);
  close (saved);
  char buff[256] = { 0 };
  ssize_t len = pread (fd, buff, sizeof (buff) - 1u, 0);
  CU_ASSERT (len > 0 && strstr (buff, ":default:WARN] default logger message") != NULL)
  close (fd);
  unlink (path);
}

void cunit_logger_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("logger", suite_init, suite_clean);
  CU_add_test (suite, "logger_default", cunit_logger_default);
  CU_add_test (suite, "logger_named", cunit_logger_named);
  CU_add_test (suite, "logger_impl", cunit_logger_impl);
  CU_add_test (suite, "logger_sub", cunit_logger_sub);
  CU_add_test (suite, "logger_file", cunit_logger_file);
  CU_add_test (suite, "logger_file_flush", cunit_logger_file_flush);
  CU_add_test (suite, "logger_no_name", cunit_logger_no_name);
  CU_add_test (suite, "logger_file_open_fail", cunit_logger_file_open_fail);
  CU_add_test (suite, "logger_console_flush", cunit_logger_console_flush);
  CU_add_test (suite, "logger_udp", cunit_logger_udp);
  CU_add_test (suite, "logger_udp_broadcast", cunit_logger_udp_broadcast);
  CU_add_test (suite, "logger_null", cunit_logger_null);
  CU_add_test (suite, "logger_start_stop", cunit_logger_start_stop);
  CU_add_test (suite, "logger_refcount", cunit_logger_refcount);
  CU_add_test (suite, "logger_selfstart", cunit_logger_selfstart);
  CU_add_test (suite, "logger_format", cunit_logger_format);
  CU_add_test (suite, "logger_set_next", cunit_logger_set_next);
  CU_add_test (suite, "logger_level_name", cunit_logger_level_name);
  CU_add_test (suite, "logger_chained", cunit_logger_chained);
  CU_add_test (suite, "logger_udp_long", cunit_logger_udp_long);
  CU_add_test (suite, "logger_udp_host", cunit_logger_udp_host);
  CU_add_test (suite, "logger_chain_no_container", cunit_logger_chain_no_container);
  CU_add_test (suite, "logger_chain_level", cunit_logger_chain_level);
  CU_add_test (suite, "logger_default_next_refs", cunit_logger_default_next_refs);
  CU_add_test (suite, "logger_set_next_concurrent", cunit_logger_set_next_concurrent);
  CU_add_test (suite, "logger_default_concurrent", cunit_logger_default_concurrent);
}
