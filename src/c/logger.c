//
// Copyright (c) 2018-2026 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/logger.h"
#include "iot/container.h"
#include "iot/time.h"
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef __ZEPHYR__
#include <netdb.h>
#endif

#ifdef IOT_HAS_PRCTL
#include <sys/prctl.h>
#endif

#ifdef __ZEPHYR__
static time_t time (time_t *t)
{
  return k_uptime_get () / 1000;
}
#endif

#define IOT_PRCTL_NAME_MAX 16
#define IOT_LOG_LEVELS 6

#ifdef IOT_BUILD_COMPONENTS
#define IOT_LOGGER_FACTORY iot_logger_factory ()
#else
#define IOT_LOGGER_FACTORY NULL
#endif

typedef struct iot_logger_impl_t
{
  iot_logger_t base;                  // Public part of logger
  volatile iot_loglevel_t save;       // Last saved log level
  iot_loglevel_t mylevel;             // Log level for this logger only
  iot_data_t *parents;                // List of loggers (pointers) for which this is the 'next' logger
  pthread_mutex_t mtx;                // Mutex protecting parents list and next pointer
  char * name;                        // Name of logger
  iot_log_function_t impl;            // Log implementation function
  iot_log_free_fn_t freectx;          // Function to free log context
  void *ctx;                          // Context for custom loggers
  struct iot_logger_impl_t * next;    // Pointer to next logger (can be chained in config)
  char buff [IOT_LOG_MSG_MAX];        // Log format buffer
  bool no_stderr;                     // If set, console logs go to stdout only
}
iot_logger_impl_t;

#define IOT_LOGGER_DEFAULT_NAME "default"

static const char * iot_log_levels[IOT_LOG_LEVELS] = {"", "ERROR", "WARN", "Info", "Debug", "Trace"};
static iot_logger_impl_t iot_logger_dfl;
static iot_data_list_static_t iot_logger_dfl_pars;

static void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx);

static pthread_once_t iot_logger_dfl_once = PTHREAD_ONCE_INIT;

static void iot_logger_dfl_init (void)
{
  iot_logger_t * logger = &iot_logger_dfl.base;
  iot_component_init (&logger->component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
  iot_logger_dfl.base.level = iot_logger_dfl.save = iot_logger_dfl.mylevel = IOT_LOGLEVEL_DEFAULT;
  iot_logger_dfl.name = (char*) IOT_LOGGER_DEFAULT_NAME; // Never freed, as default logger is static
  iot_logger_dfl.impl = iot_log_console;
  iot_logger_dfl.parents = iot_data_alloc_const_list (&iot_logger_dfl_pars);
  pthread_mutex_init (&iot_logger_dfl.mtx, NULL);
}

iot_logger_t * iot_logger_default (void)
{
  pthread_once (&iot_logger_dfl_once, iot_logger_dfl_init); // Initialised once, safely if called concurrently
  return &iot_logger_dfl.base;
}

// Returns next logger in chain, with a reference held, as next may be changed concurrently by iot_logger_set_next

static iot_logger_impl_t * iot_logger_next_ref (iot_logger_impl_t * logger)
{
  pthread_mutex_lock (&logger->mtx);
  iot_logger_impl_t * next = logger->next;
  if (next) iot_logger_add_ref (&next->base);
  pthread_mutex_unlock (&logger->mtx);
  return next;
}

void iot_log__va_log (iot_logger_t * l, iot_loglevel_t level, const char* fmt, va_list args)
{
  iot_logger_impl_t *logger = (iot_logger_impl_t*) l;
  char str[1024];
  vsnprintf (str, sizeof (str), fmt, args);
  uint64_t ts = iot_time_usecs ();
  bool ref = false; // Whether reference held on current logger (not held on first logger, owned by caller)
  while (logger)
  {
    if (logger->mylevel >= level) (logger->impl) (&logger->base, level, ts, str, logger->ctx);
    iot_logger_impl_t * next = iot_logger_next_ref (logger);
    if (ref) iot_logger_free (&logger->base);
    logger = next;
    ref = true;
  }
}

void iot_log__log (iot_logger_t * l, iot_loglevel_t level, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  iot_log__va_log (l, level, fmt, args);
  va_end (args);
}

static iot_loglevel_t iot_logger_effective_level (const iot_logger_impl_t *logger)
{
  const iot_logger_impl_t * next = logger->next;
  return (next && next->base.level > logger->mylevel) ? next->base.level : logger->mylevel;
}

// Update effective level of loggers that chain to this logger. Parents remove themselves from the parents list (under
// mtx) before being freed, so are valid while mtx held. Locks are always taken from next logger to parent logger.

static void iot_logger_relevel (iot_logger_impl_t * logger)
{
  iot_data_list_iter_t iter;
  pthread_mutex_lock (&logger->mtx);
  iot_data_list_iter (logger->parents, &iter);
  while (iot_data_list_iter_next (&iter))
  {
    iot_logger_impl_t * parent = iot_data_list_iter_pointer_value (&iter);
    parent->base.level = iot_logger_effective_level (parent);
    iot_logger_relevel (parent);
  }
  pthread_mutex_unlock (&logger->mtx);
}

static bool iot_logger_ptr_cmp (const iot_data_t * value, const void * arg)
{
  return iot_data_pointer (value) == arg;
}

static void iot_logger_remove_parent (iot_logger_impl_t * logger, const iot_logger_impl_t * parent)
{
  pthread_mutex_lock (&logger->mtx);
  iot_data_list_remove (logger->parents, iot_logger_ptr_cmp, parent);
  pthread_mutex_unlock (&logger->mtx);
}

void iot_logger_set_level (iot_logger_t * logger, iot_loglevel_t level)
{
  assert (logger);
  iot_logger_impl_t *impl = (iot_logger_impl_t *)logger;
  impl->mylevel = level;
  logger->level = iot_logger_effective_level (impl);
  if (logger->component.state == IOT_COMPONENT_RUNNING)
  {
    iot_logger_relevel (impl);
  }
}

iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, bool start, iot_logger_t * next, iot_log_function_t impl, void * ctx, iot_log_free_fn_t freectx)
{
  assert (impl);
  iot_logger_impl_t * logger = calloc (1, sizeof (*logger));
  logger->impl = impl;
  logger->name = strdup (name ? name : IOT_LOGGER_DEFAULT_NAME);
  logger->save = level;
  logger->ctx = ctx;
  logger->freectx = freectx;
  logger->parents = iot_data_alloc_typed_list (IOT_DATA_POINTER);
  pthread_mutex_init (&logger->mtx, NULL);
  iot_component_init (&logger->base.component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
  if (next) iot_logger_set_next (&logger->base, next);
  if (start) iot_logger_start (&logger->base);
  return &logger->base;
}

iot_logger_t * iot_logger_alloc (const char * name, iot_loglevel_t level, bool start)
{
  return iot_logger_alloc_custom (name, level, start, NULL, iot_log_console, NULL, NULL);
}

void iot_logger_free (iot_logger_t * logger)
{
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  if (impl && (impl != &iot_logger_dfl) && iot_component_dec_ref (&logger->component))
  {
    if (impl->next) iot_logger_remove_parent (impl->next, impl);
    free (impl->name);
    iot_logger_free ((iot_logger_t*) impl->next);
    iot_data_free (impl->parents);
    if (impl->freectx) (impl->freectx) (impl->ctx);
    iot_component_fini (&logger->component);
    pthread_mutex_destroy (&impl->mtx);
    free (logger);
  }
}

void iot_logger_add_ref (iot_logger_t * logger)
{
  if (logger) iot_component_add_ref (&logger->component);
}

void iot_logger_start (iot_logger_t * logger)
{
  assert (logger);
  iot_component_set_running (&logger->component);
  iot_logger_set_level (logger, ((iot_logger_impl_t *)logger)->save);
}

void iot_logger_stop (iot_logger_t * logger)
{
  assert (logger);
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  iot_component_set_stopped (&logger->component);
  impl->save = impl->mylevel;
  iot_logger_set_level (logger, IOT_LOG_NONE);
}

static inline size_t iot_logger_format_log (iot_logger_impl_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  char tname[IOT_PRCTL_NAME_MAX] = { 0 };
#ifdef IOT_HAS_PRCTL
  prctl (PR_GET_NAME, tname);
#endif
  int len = snprintf (logger->buff, sizeof (logger->buff), "[%s:%" PRIu64 ":%s:%s] %s\n", tname, timestamp, logger->name, iot_log_levels[level], message);
  if (len < 0) return 0u;
  if ((size_t) len >= sizeof (logger->buff)) // Truncated, so length is that of buffer contents, still ending with newline
  {
    len = (int) sizeof (logger->buff) - 1;
    logger->buff[len - 1] = '\n';
  }
  return (size_t) len;
}

static inline void iot_logger_log_to_fd (iot_logger_impl_t * logger, FILE * fd, iot_loglevel_t level, uint64_t timestamp, const char *message, bool flush)
{
  iot_component_lock (&logger->base.component);
  if (iot_logger_format_log (logger, level, timestamp, message))
  {
    fprintf (fd, "%s", logger->buff);
    if (flush) fflush (fd);
  }
  iot_component_unlock (&logger->base.component);
}

static void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  (void) ctx;
  iot_logger_impl_t *impl = (iot_logger_impl_t*) logger;
  // Flushed, as stdout buffered when not a terminal (e.g. if redirected to a file or pipe)
  iot_logger_log_to_fd (impl, (impl->no_stderr || level > IOT_LOG_WARN) ? stdout : stderr, level, timestamp, message, true);
}

/********* Standard Logger Implementations: UDP *********/

typedef struct iot_logger_udp_ctx_t
{
  struct sockaddr_in addr;            // Address
  int sock;                           // Socket
} iot_logger_udp_ctx_t;

static void iot_log_udp (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  iot_logger_impl_t * logimpl = (iot_logger_impl_t*) logger;
  const iot_logger_udp_ctx_t  *impl = ctx;
  iot_component_lock (&logger->component);
  if (impl->sock != -1)
  {
    size_t len = iot_logger_format_log (logimpl, level, timestamp, message);
    if (len > 0) sendto (impl->sock, logimpl->buff, len, 0, (struct sockaddr *) &impl->addr, sizeof (struct sockaddr_in));
  }
  iot_component_unlock (&logger->component);
}

static void iot_logger_udp_ctx_free (void *ctx)
{
  const iot_logger_udp_ctx_t *udp = (iot_logger_udp_ctx_t*) ctx;
  close (udp->sock);
  free (ctx);
}

iot_logger_t * iot_logger_alloc_udp (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, const char *host, uint16_t port)
{
  static const int yes = 1;
  iot_logger_udp_ctx_t *ctx = calloc (1, sizeof (iot_logger_udp_ctx_t));
  if (host)
  {
    if (inet_aton (host, &ctx->addr.sin_addr) == 0) // Not an IPv4 address, so resolve host name
    {
      bool resolved = false;
#ifndef __ZEPHYR__
      struct addrinfo * info = NULL;
      const struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_DGRAM };
      if (getaddrinfo (host, NULL, &hints, &info) == 0 && info)
      {
        ctx->addr.sin_addr = ((const struct sockaddr_in*) info->ai_addr)->sin_addr;
        resolved = true;
      }
      if (info) freeaddrinfo (info);
#endif
      if (! resolved)
      {
        free (ctx);
        return NULL;
      }
    }
  }
  else
  {
    ctx->addr.sin_addr.s_addr = htonl (INADDR_BROADCAST);
  }
  ctx->addr.sin_family = AF_INET;
  ctx->addr.sin_port = htons (port);
  ctx->sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (host == NULL) setsockopt (ctx->sock, SOL_SOCKET, SO_BROADCAST, (const void*) &yes, sizeof (yes));
  return iot_logger_alloc_custom (name, level, self_start, next, iot_log_udp, ctx, iot_logger_udp_ctx_free);
}

/********* Standard Logger Implementations: File *********/

#if defined (IOT_HAS_FILE)

static void iot_log_file (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  // Flushed, so file contains all logs if process exits abnormally
  iot_logger_log_to_fd ((iot_logger_impl_t*) logger, (FILE *)ctx, level, timestamp, message, true);
}

static void iot_logger_file_ctx_free (void *ctx)
{
  fclose (ctx);
}

iot_logger_t * iot_logger_alloc_file (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, const char *pathname)
{
  FILE *fd = fopen (pathname, "a");
  if (fd == NULL)
  {
    iot_log_error (next ? next : iot_logger_default (), "iot_logger_alloc_file: failed to open %s (%s)", pathname, strerror (errno));
    return NULL;
  }
  return iot_logger_alloc_custom (name, level, self_start, next, iot_log_file, fd, iot_logger_file_ctx_free);
}

#endif

iot_loglevel_t iot_logger_level_from_string (const char *name)
{
  iot_loglevel_t level = IOT_LOGLEVEL_DEFAULT;
  if (name)
  {
    int c = toupper ((unsigned char) name[0]);
    for (int lvl = 0; lvl < IOT_LOG_LEVELS; lvl++)
    {
      if (iot_log_levels[lvl][0] == c)
      {
        level = lvl;
        break;
      }
    }
  }
  return level;
}

const char * iot_logger_level_to_string (iot_loglevel_t level)
{
  return iot_log_levels[level];
}

void iot_logger_set_next (iot_logger_t * logger, iot_logger_t * next)
{
  assert (logger);
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  iot_logger_impl_t * next_impl = (iot_logger_impl_t*) next;
  iot_logger_add_ref (next);
  if (next_impl)
  {
    pthread_mutex_lock (&next_impl->mtx);
    iot_data_list_tail_push (next_impl->parents, iot_data_alloc_pointer (impl, NULL));
    pthread_mutex_unlock (&next_impl->mtx);
  }
  pthread_mutex_lock (&impl->mtx);
  iot_logger_impl_t * old = impl->next;
  impl->next = next_impl;
  pthread_mutex_unlock (&impl->mtx);
  if (old)
  {
    iot_logger_remove_parent (old, impl);
    iot_logger_free (&old->base); // Loggers in use by iot_log__va_log hold a reference
  }
  if (logger->component.state != IOT_COMPONENT_INITIAL) iot_logger_set_level (logger, impl->mylevel); // Effective level depends on next logger
}

#ifdef IOT_BUILD_COMPONENTS

static iot_loglevel_t iot_logger_config_level (const iot_data_t * map)
{
  return iot_logger_level_from_string (iot_data_string_map_get_string (map, "Level"));
}

static iot_component_t * iot_logger_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t *result;
  iot_loglevel_t level = iot_logger_config_level (map);
  iot_logger_t * next = (iot_logger_t*) iot_container_get_component (cont, iot_data_string_map_get_string (map, "Next"));
  bool start = iot_data_string_map_get_bool (map, "Start", true);
  const char * name = iot_data_string_map_get_string (map, "Name");
  const char * to = iot_data_string_map_get_string (map, "To");

#if defined (IOT_HAS_FILE)
  if (to && strncmp (to, "file:", 5) == 0 && strlen (to) > 5)
  {
    result = iot_logger_alloc_file (name, level, start, next, to + 5);
  }
  else
#endif
  {
    if (to && strncmp (to, "udp:", 4) == 0 && strlen (to) > 4)
    {
      uint16_t port;
      char * host = NULL;
      to += 4;
      const char * sep = strchr (to, ':');
      if (sep)
      {
        host = strndup (to, (size_t) (sep - to));
        to = sep + 1;
      }
      port = (uint16_t) atoi (to);
      result = iot_logger_alloc_udp (name, level, start, next, host, port);
      free (host);
    }
    else
    {
      result = iot_logger_alloc_custom (name, level, start, next, iot_log_console, NULL, NULL);
      ((iot_logger_impl_t *)result)->no_stderr = iot_data_string_map_get_bool (map, "NoStderr", false);
    }
  }
  iot_container_release_component ((iot_component_t*) next); // Logger holds its own next logger reference
  return (iot_component_t*) result;
}

static bool iot_logger_reconfig (iot_component_t * comp, iot_container_t * cont, const iot_data_t * map)
{
  (void) cont;
  iot_logger_set_level ((iot_logger_t*) comp, iot_logger_config_level (map));
  return true;
}

const iot_component_factory_t * iot_logger_factory (void)
{
  static iot_component_factory_t factory =
  {
    IOT_LOGGER_TYPE,
    IOT_CATEGORY_CORE,
    iot_logger_config,
    (iot_component_free_fn_t) iot_logger_free,
    iot_logger_reconfig,
    NULL
  };
  return &factory;
}

#endif
