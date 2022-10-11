//
// Copyright (c) 2018-2022 IOTech
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

#ifdef IOT_HAS_PRCTL
#include <sys/prctl.h>
#endif

#ifdef __ZEPHYR__
static time_t time (time_t *t)
{
  return k_uptime_get () / 1000;
}
#endif
#ifdef _AZURESPHERE_
#include <applibs/log.h>
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
  char * name;                        // Name of logger
  iot_log_function_t impl;            // Log implementation function
  iot_log_free_fn_t freectx;          // Fucntion to free log context
  void *ctx;                          // Context for custom loggers
  struct iot_logger_impl_t * next;    // Pointer to next logger (can be chained in config)
  char buff [IOT_LOG_MSG_MAX];        // Log format buffer
}
iot_logger_impl_t;

static const char * iot_log_levels[IOT_LOG_LEVELS] = {"", "ERROR", "WARN", "Info", "Debug", "Trace"};
static iot_logger_impl_t iot_logger_dfl;

static void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx);

iot_logger_t * iot_logger_default (void)
{
  static iot_logger_t * logger = NULL;
  if (logger == NULL)
  {
    logger = &iot_logger_dfl.base;
    memset (&iot_logger_dfl, 0, sizeof (iot_logger_dfl));
    iot_component_init (&logger->component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
    iot_logger_dfl.base.level = iot_logger_dfl.save = IOT_LOGLEVEL_DEFAULT;
    iot_logger_dfl.impl = iot_log_console;
  }
  return logger;
}

void iot_log__log (iot_logger_t * l, iot_loglevel_t level, ...)
{
  char str[1024];
  iot_logger_impl_t *logger = (iot_logger_impl_t *)l;
  va_list args;
  va_start (args, level);
  const char * fmt = va_arg (args, const char *);
  vsnprintf (str, sizeof (str), fmt, args);
  va_end (args);
  uint64_t ts = iot_time_usecs ();
  do
  {
    if (logger->base.level >= level) (logger->impl) (&logger->base, level, ts, str, logger->ctx);
  } while ((logger = logger->next));
}

void iot_logger_set_level (iot_logger_t * logger, iot_loglevel_t level)
{
  assert (logger);
  logger->level = level;
}

iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, bool start, iot_logger_t * next, iot_log_function_t impl, void * ctx, iot_log_free_fn_t freectx)
{
  assert (name && impl);
  iot_logger_impl_t * logger = calloc (1, sizeof (*logger));
  logger->impl = impl;
  logger->name = strdup (name);
  logger->save = level;
  logger->next = (iot_logger_impl_t*) next;
  logger->ctx = ctx;
  logger->freectx = freectx;
  iot_component_init (&logger->base.component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
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
    free (impl->name);
    if (impl->freectx) (impl->freectx) (impl->ctx);
    iot_component_fini (&logger->component);
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
  const iot_logger_impl_t * impl = (const iot_logger_impl_t*) logger;
  iot_component_set_running (&logger->component);
  logger->level = impl->save;
}

void iot_logger_stop (iot_logger_t * logger)
{
  assert (logger);
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  iot_component_set_stopped (&logger->component);
  impl->save = logger->level;
  logger->level = IOT_LOG_NONE;
}

static inline size_t iot_logger_format_log (iot_logger_impl_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  char tname[IOT_PRCTL_NAME_MAX] = { 0 };
#ifdef IOT_HAS_PRCTL
  prctl (PR_GET_NAME, tname);
#endif
  return (size_t) snprintf (logger->buff, sizeof (logger->buff), "[%s:%" PRIu64 ":%s:%s] %s\n", tname, timestamp, logger->name, iot_log_levels[level], message);
}

static inline void iot_logger_log_to_fd (iot_logger_impl_t * logger, FILE * fd, iot_loglevel_t level, uint64_t timestamp, const char *message)
{
  iot_component_lock (&logger->base.component);
  if (iot_logger_format_log (logger, level, timestamp, message))
  {
#ifdef _AZURESPHERE_
    Log_Debug ("%s", logger->buff);
#else
    fprintf (fd, "%s", logger->buff);
#endif
  }
  iot_component_unlock (&logger->base.component);
}

static void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  iot_logger_log_to_fd ((iot_logger_impl_t*) logger, (level > IOT_LOG_WARN) ? stdout : stderr, level, timestamp, message);
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
  iot_logger_udp_ctx_t *impl = (iot_logger_udp_ctx_t *)ctx;
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
  iot_logger_udp_ctx_t *udp = (iot_logger_udp_ctx_t *)ctx;
  close (udp->sock);
  free (ctx);
}

iot_logger_t * iot_logger_alloc_udp (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, const char *host, uint16_t port)
{
  static const int yes = 1;
  iot_logger_udp_ctx_t *ctx = calloc (1, sizeof (iot_logger_udp_ctx_t));
  if (host)
  {
    inet_aton (host, &ctx->addr.sin_addr);
  }
  else
  {
    ctx->addr.sin_addr.s_addr = htonl (INADDR_BROADCAST);
  }
  ctx->addr.sin_port = htons (port);
  ctx->sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (host == NULL) setsockopt (ctx->sock, SOL_SOCKET, SO_BROADCAST, (const void*) &yes, sizeof (yes));
  return iot_logger_alloc_custom (name, level, self_start, next, iot_log_udp, ctx, iot_logger_udp_ctx_free);
}

/********* Standard Logger Implementations: File *********/

#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)

static void iot_log_file (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx)
{
  if (ctx) iot_logger_log_to_fd ((iot_logger_impl_t*) logger, (FILE *)ctx, level, timestamp, message);
}

static void iot_logger_file_ctx_free (void *ctx)
{
  if (ctx)
  {
    fclose (ctx);
  }
}

iot_logger_t * iot_logger_alloc_file (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, const char *pathname)
{
  FILE *fd = fopen (pathname, "a");
  return iot_logger_alloc_custom (name, level, self_start, next, iot_log_file, fd, iot_logger_file_ctx_free);
}

#endif

iot_loglevel_t iot_logger_level_from_string (const char *name)
{
  iot_loglevel_t level = IOT_LOGLEVEL_DEFAULT;
  if (name)
  {
    int c = toupper (name[0]);
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

const char *iot_logger_level_to_string (iot_loglevel_t level)
{
  return iot_log_levels[level];
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
  iot_logger_t * next = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Next"));
  bool start = iot_data_string_map_get_bool (map, "Start", true);
  const char * name = iot_data_string_map_get_string (map, "Name");
  const char * to = iot_data_string_map_get_string (map, "To");

#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
  if (to && strncmp (to, "file:", 5) == 0 && strlen (to) > 5)
  {
    result = iot_logger_alloc_file (name, level, start, next, to + 5);
  }
  else
#endif
  {
    if (to && strncmp (to, "udp:", 4) == 0 && strlen (to) > 4)
    {
      char target[17] = { 0 };
      uint16_t port;
      const char *host = NULL;
      to += 4;
      const char * sep = strchr (to, ':');
      if (sep)
      {
        strncpy (target, to, (size_t) (sep - to));
        host = target;
        to = sep + 1;
      }
      port = atoi (to);
      result = iot_logger_alloc_udp (name, level, start, next, host, port);
    }
    else
    {
      result = iot_logger_alloc_custom (name, level, start, next, iot_log_console, NULL, NULL);
    }
  }
  return (iot_component_t*) result;
}

static bool iot_logger_reconfig (iot_component_t * comp, iot_container_t * cont, const iot_data_t * map)
{
  (void) cont;
  ((iot_logger_t*) comp)->level = iot_logger_config_level (map);
  return true;
}

const iot_component_factory_t * iot_logger_factory (void)
{
  static iot_component_factory_t factory = { IOT_LOGGER_TYPE, iot_logger_config, (iot_component_free_fn_t) iot_logger_free,  iot_logger_reconfig, IOT_CATEGORY_CORE, NULL };
  return &factory;
}

#endif
