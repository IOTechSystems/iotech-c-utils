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

extern void iot_log_file (struct iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message);
extern void iot_log_console (struct iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message);
extern void iot_log_udp (struct iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message);

static const char * iot_log_levels[IOT_LOG_LEVELS] = {"", "ERROR", "WARN", "Info", "Debug", "Trace"};
static iot_logger_t iot_logger_dfl;

iot_logger_t * iot_logger_default (void)
{
  static iot_logger_t * logger = NULL;
  if (logger == NULL)
  {
    logger = &iot_logger_dfl;
    memset (&iot_logger_dfl, 0, sizeof (iot_logger_dfl));
    iot_component_init (&logger->component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
    iot_logger_dfl.level = iot_logger_dfl.save = IOT_LOGLEVEL_DEFAULT;
    iot_logger_dfl.impl = iot_log_console;
  }
  return logger;
}

static void iot_logger_log (iot_logger_t *logger, iot_loglevel_t level, va_list args)
{
  char str[1024];
  const char * fmt = va_arg (args, const char *);
  vsnprintf (str, sizeof (str), fmt, args);
  uint64_t ts = iot_time_usecs ();
  if (logger->level >= level) (logger->impl) (logger, level, ts, str);
  while ((logger = logger->next))
  {
    if (logger->level >= level) (logger->impl) (logger, level, ts, str);
  }
}

void iot_log__error (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log (logger, IOT_LOG_ERROR, args);
  va_end (args);
}

void iot_log__warn (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log (logger, IOT_LOG_WARN, args);
  va_end (args);
}

void iot_log__info (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log (logger, IOT_LOG_INFO, args);
  va_end (args);
}

void iot_log__debug (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log (logger, IOT_LOG_DEBUG, args);
  va_end (args);
}

void iot_log__trace (iot_logger_t * logger, ...)
{
  va_list args;
  va_start (args, logger);
  iot_logger_log (logger, IOT_LOG_TRACE, args);
  va_end (args);
}

void iot_logger_set_level (iot_logger_t * logger, iot_loglevel_t level)
{
  assert (logger);
  logger->level = level;
}

iot_logger_t * iot_logger_alloc (const char * name, iot_loglevel_t level, bool self_start)
{
  return iot_logger_alloc_custom (name, level, NULL, iot_log_console, NULL, self_start);
}

iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, const char * to, iot_log_function_t impl, iot_logger_t * next, bool self_start)
{
  assert (name && impl);
  iot_logger_t * logger = calloc (1, sizeof (*logger));
  logger->impl = impl;
  logger->name = strdup (name);
  logger->to = to ? strdup (to) : NULL;
  logger->save = level;
  logger->next = next;
  iot_component_init (&logger->component, IOT_LOGGER_FACTORY, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
  if (self_start) iot_logger_start (logger);
  return logger;
}

void iot_logger_free (iot_logger_t * logger)
{
  if (logger && (logger != &iot_logger_dfl) && iot_component_dec_ref (&logger->component))
  {
    free (logger->name);
    free (logger->to);
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
  iot_component_set_running (&logger->component);
  logger->level = logger->save;
}

void iot_logger_stop (iot_logger_t * logger)
{
  assert (logger);
  iot_component_set_stopped (&logger->component);
  logger->save = logger->level;
  logger->level = IOT_LOG_NONE;
}

iot_logger_t * iot_logger_next (iot_logger_t * logger)
{
  assert (logger);
  return logger->next;
}

static inline int iot_logger_format_log (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  char tname[IOT_PRCTL_NAME_MAX] = { 0 };
#ifdef IOT_HAS_PRCTL
  prctl (PR_GET_NAME, tname);
#endif
  return snprintf (logger->buff, sizeof (logger->buff), "[%s:%" PRIu64 ":%s:%s] %s\n", tname, timestamp, logger->name, iot_log_levels[level], message);
}

static inline void iot_logger_log_to_fd (iot_logger_t * logger, FILE * fd, iot_loglevel_t level, uint64_t timestamp, const char *message)
{
  iot_component_lock (&logger->component);
  if (iot_logger_format_log (logger, level, timestamp, message))
  {
#ifdef _AZURESPHERE_
    Log_Debug ("%s", logger->buff);
#else
    fprintf (fd, logger->buff);
#endif
  }
  iot_component_unlock (&logger->component);
}

#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
void iot_log_file (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  FILE * fd = fopen (logger->to, "a");
  if (fd)
  {
    iot_logger_log_to_fd (logger, fd, level, timestamp, message);
    fclose (fd);
  }
}
#endif

extern void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  iot_logger_log_to_fd (logger, (level > IOT_LOG_WARN) ? stdout : stderr, level, timestamp, message);
}

/* iot_log_udp: To is either "host:port" or "port". Latter form means broadcast. */

extern void iot_log_udp (iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message)
{
  static const int yes = 1;
  struct sockaddr_in addr = { .sin_family = AF_INET };
  int sock = socket (AF_INET, SOCK_DGRAM, 0);
  const char * sep = strchr (logger->to, ':');
  if (sep)
  {
    char target[17];
    strncpy (target, logger->to, (size_t) (sep - logger->to));
    inet_aton (target, &addr.sin_addr);
    addr.sin_port = htons ((uint16_t) atoi (sep + 1));
  }
  else
  {
    addr.sin_addr.s_addr = htonl (INADDR_BROADCAST);
    addr.sin_port = htons ((uint16_t) atoi (logger->to));
    setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char*) &yes, sizeof (yes));
  }
  iot_component_lock (&logger->component);
  int len = iot_logger_format_log (logger, level, timestamp, message);
  if (len > 0)
  {
    sendto (sock, logger->buff, len, 0, (struct sockaddr*) &addr, sizeof (struct sockaddr_in));
  }
  iot_component_unlock (&logger->component);
  close (sock);
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
  iot_logger_t * next;
  iot_logger_t * logger;
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
  next = (iot_logger_t*) iot_container_find_component (cont, iot_data_string_map_get_string (map, "Next"));
  bool self_start = iot_data_string_map_get_bool (map, "Start", true);
  logger = iot_logger_alloc_custom (iot_data_string_map_get_string (map, "Name"), level, to, impl, next, self_start);
  return &logger->component;
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
