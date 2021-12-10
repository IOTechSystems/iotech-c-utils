//
// Copyright (c) 2018-2021 IOTech
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
  struct iot_logger_impl_t * next;    // Pointer to next logger (can be chained in config)
  int sock;                           // Socket for UDP logger
  struct sockaddr_in addr;            // Address for UDP logger
  char buff [IOT_LOG_MSG_MAX];        // Log format buffer
#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
  FILE * fd;                          // Descriptor for File logger
#endif
}
iot_logger_impl_t;

static const char * iot_log_levels[IOT_LOG_LEVELS] = {"", "ERROR", "WARN", "Info", "Debug", "Trace"};
static iot_logger_impl_t iot_logger_dfl;

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

static void iot_logger_log (iot_logger_impl_t * logger, iot_loglevel_t level, va_list args)
{
  char str[1024];
  const char * fmt = va_arg (args, const char *);
  vsnprintf (str, sizeof (str), fmt, args);
  uint64_t ts = iot_time_usecs ();
  if (logger->base.level >= level) (logger->impl) (&logger->base, level, ts, str);
  while ((logger = logger->next))
  {
    if (logger->base.level >= level) (logger->impl) (&logger->base, level, ts, str);
  }
}

void iot_log__error (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log ((iot_logger_impl_t*) logger, IOT_LOG_ERROR, args);
  va_end (args);
}

void iot_log__warn (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log ((iot_logger_impl_t*) logger, IOT_LOG_WARN, args);
  va_end (args);
}

void iot_log__info (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log ((iot_logger_impl_t*) logger, IOT_LOG_INFO, args);
  va_end (args);
}

void iot_log__debug (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log ((iot_logger_impl_t*) logger, IOT_LOG_DEBUG, args);
  va_end (args);
}

void iot_log__trace (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log ((iot_logger_impl_t*) logger, IOT_LOG_TRACE, args);
  va_end (args);
}

void iot_logger_set_level (iot_logger_t * logger, iot_loglevel_t level)
{
  assert (logger);
  logger->level = level;
}

static inline iot_logger_type_t iot_logger_type (iot_log_function_t fn)
{
  if (fn == iot_log_console) return IOT_LOGGER_CONSOLE;
#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
  if (fn == iot_log_file) return IOT_LOGGER_FILE;
#endif
  if (fn == iot_log_udp) return IOT_LOGGER_UDP;
  return IOT_LOGGER_CUSTOM;
}

iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, const char * to, iot_log_function_t impl, iot_logger_t * next, bool start)
{
  static const int yes = 1;
  assert (name && impl);
  iot_logger_type_t type = iot_logger_type (impl);
  iot_logger_impl_t * logger = calloc (1, sizeof (*logger));
  logger->impl = impl;
  logger->sock = -1;
  logger->name = strdup (name);
  logger->save = level;
  logger->next = (iot_logger_impl_t*) next;
  if (type == IOT_LOGGER_UDP)
  {
    const char * sep = strchr (to, ':');
    if (sep)
    {
      char target[17] = { 0 };
      strncpy (target, to, (size_t) (sep - to));
      inet_aton (target, &logger->addr.sin_addr);
      to = sep + 1;
    }
    else
    {
      logger->addr.sin_addr.s_addr = htonl (INADDR_BROADCAST);
    }
    logger->addr.sin_port = htons ((uint16_t) atoi (to));
    logger->sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (sep == NULL) setsockopt (logger->sock, SOL_SOCKET, SO_BROADCAST, (char *) &yes, sizeof (yes));
  }
#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
  else if (type == IOT_LOGGER_FILE)
  {
    logger->fd = fopen (to, "a");
  }
#endif
  iot_component_init (&logger->base.component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
  if (start) iot_logger_start (&logger->base);
  return &logger->base;
}

iot_logger_t * iot_logger_alloc (const char * name, iot_loglevel_t level, bool start)
{
  return iot_logger_alloc_custom (name, level,NULL, iot_log_console, NULL, start);
}

void iot_logger_free (iot_logger_t * logger)
{
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  if (impl && (impl != &iot_logger_dfl) && iot_component_dec_ref (&logger->component))
  {
    free (impl->name);
    close (impl->sock);
#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
    if (impl->fd) fclose (impl->fd);
#endif
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
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
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

#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
extern void iot_log_file (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  if (impl->fd) iot_logger_log_to_fd (impl, impl->fd, level, timestamp, message);
}
#endif

extern void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  iot_logger_log_to_fd ((iot_logger_impl_t*) logger, (level > IOT_LOG_WARN) ? stdout : stderr, level, timestamp, message);
}

/* iot_log_udp: To is either "<host>:<port>" or "<port>". Latter form means broadcast. */

extern void iot_log_udp (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  iot_logger_impl_t * impl = (iot_logger_impl_t*) logger;
  iot_component_lock (&logger->component);
  if (impl->sock != -1)
  {
    size_t len = iot_logger_format_log (impl, level, timestamp, message);
    if (len > 0) sendto (impl->sock, impl->buff, len, 0, (struct sockaddr *) &impl->addr, sizeof (struct sockaddr_in));
  }
  iot_component_unlock (&logger->component);
}

#ifdef IOT_BUILD_COMPONENTS

static iot_loglevel_t iot_logger_config_level (const iot_data_t * map)
{
  iot_loglevel_t level = IOT_LOGLEVEL_DEFAULT;
  const char * name = iot_data_string_map_get_string (map, "Level");
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

static iot_component_t * iot_logger_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_log_function_t impl = iot_log_console; /* log to stderr or stdout */
  iot_loglevel_t level = iot_logger_config_level (map);
  const char * to = iot_data_string_map_get_string (map, "To");

#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
  if (to && strncmp (to, "file:", 5) == 0 && strlen (to) > 5)
  {
    impl = iot_log_file; /* Log to file */
    to += 5;
  }
  else
#endif
  if (to && strncmp (to, "udp:", 4) == 0 && strlen (to) > 4)
  {
    impl = iot_log_udp; /* Log to udp */
    to += 4;
  }
  iot_logger_t * next = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Next"));
  bool start = iot_data_string_map_get_bool (map, "Start", true);
  return (iot_component_t*) iot_logger_alloc_custom (iot_data_string_map_get_string (map, "Name"), level, to, impl, next, start);
}

static bool iot_logger_reconfig (iot_component_t * comp, iot_container_t * cont, const iot_data_t * map)
{
  (void) cont;
  ((iot_logger_t*) comp)->level = iot_logger_config_level (map);
  return true;
}

const iot_component_factory_t * iot_logger_factory (void)
{
  static iot_component_factory_t factory = { IOT_LOGGER_TYPE, iot_logger_config, (iot_component_free_fn_t) iot_logger_free,  iot_logger_reconfig, NULL };
  return &factory;
}

#endif
