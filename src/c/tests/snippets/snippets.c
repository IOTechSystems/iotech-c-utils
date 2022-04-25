#include "iot/iot.h"
#include "iot/base64.h"

extern void dummy (void);

static void snippet1 (void)
{
// CUT
iot_component_factory_add (iot_logger_factory ());
// CUT
}

static void snippet2 (void)
{
// CUT
iot_container_config_t config = { .load = iot_file_config_loader, .uri = "config_dir", .save = NULL };
iot_container_t * container = iot_container_alloc ("main");

iot_container_config (&config);

iot_component_factory_add (iot_logger_factory ());
iot_component_factory_add (iot_threadpool_factory ());
iot_component_factory_add (iot_scheduler_factory ());

iot_container_init (container);
iot_container_start (container);
iot_wait_msecs (2u);
iot_container_stop (container);
iot_container_free (container);
// CUT
}

static void snippet3 (void)
{
// CUT
#define BASE64_SRC_LEN 16

char input[BASE64_SRC_LEN] = { 0 };
char encoded[BASE64_SRC_LEN * 2];
char decoded[BASE64_SRC_LEN];
size_t outlen;

iot_b64_encode (input, BASE64_SRC_LEN, encoded, sizeof (encoded));
iot_b64_decode (encoded, decoded, &outlen);
// CUT
}

static void snippet4 (void)
{
// CUT
uint8_t array [4] = { 0, 1, 2 ,3 };
iot_data_t * data;

data = iot_data_alloc_ui64 (4u);
iot_data_free (data);
data = iot_data_alloc_string ("Hello", IOT_DATA_REF);
iot_data_free (data);
data = iot_data_alloc_array (array, 4, IOT_DATA_UINT8, IOT_DATA_REF);
iot_data_free (data);
// CUT
}

static void snippet5 (void)
{
// CUT
uint32_t hash = iot_hash ("Dummy");
printf ("Hash = %u\n", hash);
// CUT
}

static void snippet6 (void)
{
// CUT
static const char * json = "[ \"MyString\", 1234, true, false, null, 11.22 ]";
iot_json_tok_t tokens[10];
iot_json_parser parser;

iot_json_init (&parser);
iot_json_parse (&parser, json, strlen (json), tokens, 10);
// CUT
}

// CUT
static void * thread_func (void * arg)
{
  int prio = iot_thread_current_get_priority ();
  printf ("Thread priority: %d\n", prio);
  return NULL;
}

static void create_thread (iot_logger_t * logger)
{
  pthread_t id;
  void * arg = NULL;
  int prio = 10;
  iot_thread_create (&id, thread_func, arg, prio, IOT_THREAD_NO_AFFINITY, logger);
}
// CUT

static void snippet7 (void)
{
// CUT
uint64_t secs_time = iot_time_secs ();
uint64_t msecs_time = iot_time_msecs ();
uint64_t nsecs_time = iot_time_nsecs ();
// CUT
(void) secs_time;
(void) msecs_time;
(void) nsecs_time;
}

static void snippet8 (void)
{
// CUT
uint32_t val = 666;
iot_logger_t * logger = iot_logger_alloc ("MyLogger", IOT_LOG_WARN, true);
iot_log_error (logger, "Test Error from my logger");
iot_log_warn (logger, "Devilish number %d", val);
iot_log_trace (logger, "Should not see trace log");
iot_log_debug (logger, "Should not see debug log");
iot_log_info (logger, "Should not see info log");
iot_logger_free (logger);
// CUT
}

// CUT
static void * greeter (void * arg)
{
  printf ("Hello\n");
  return NULL;
}

static void schedule_greeter (iot_logger_t * logger)
{
  iot_scheduler_t * scheduler = iot_scheduler_alloc (IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_schedule_t * schedule = iot_schedule_create (scheduler, greeter, NULL, NULL, IOT_MS_TO_NS (500), 0, 0, NULL, IOT_THREAD_NO_PRIORITY);
  iot_schedule_add (scheduler, schedule);
  iot_wait_secs (2u);
  iot_schedule_remove (scheduler, schedule);
  iot_scheduler_free (scheduler);
}
// CUT

// CUT
static void * sleeper_job (void * arg)
{
  iot_wait_secs (1u);
  return NULL;
}

static void run_sleeper (iot_logger_t * logger)
{
  iot_threadpool_t * pool;
  pool = iot_threadpool_alloc (1u, 0u, IOT_THREAD_NO_PRIORITY, IOT_THREAD_NO_AFFINITY, logger);
  iot_threadpool_add_work (pool, sleeper_job, NULL, IOT_THREAD_NO_PRIORITY);
  iot_threadpool_wait (pool);
  iot_threadpool_free (pool);
}
// CUT

extern void dummy (void)
{
  snippet1 ();
  snippet2 ();
  snippet3 ();
  snippet4 ();
  snippet5 ();
  snippet6 ();
  create_thread (NULL);
  snippet7 ();
  snippet8 ();
  schedule_greeter (NULL);
  run_sleeper (NULL);
}
