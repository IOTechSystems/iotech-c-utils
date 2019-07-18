//
// Copyright (c) 2018,2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/logger.h"
#include "iot/container.h"
#include <stdarg.h>

#if defined (__linux__)
#include <sys/prctl.h>
#endif

#define IOT_PRCTL_NAME_MAX 16

#define IOT_LOG_LEVELS 6
static const char * iot_log_levels[IOT_LOG_LEVELS] = {"", "ERROR", "WARN", "Info", "Debug", "Trace"};
static iot_logger_t iot_logger_dfl;

iot_logger_t * iot_logger_default (void)
{
  static iot_logger_t * logger = NULL;
  if (logger == NULL)
  {
    logger = &iot_logger_dfl;
    memset (&iot_logger_dfl, 0, sizeof (iot_logger_dfl));
    iot_logger_dfl.save = IOT_LOGLEVEL_DEFAULT;
    iot_logger_dfl.impl = iot_log_console;
  }
  return logger;
}

static void iot_logger_log (iot_logger_t *logger, iot_loglevel_t level, va_list args)
{
  char str[1024];
  const char * fmt = va_arg (args, const char *);
  vsnprintf (str, sizeof (str), fmt, args);
  time_t ts = time (NULL);
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

#ifdef __ZEPHYR__
static time_t time (time_t *t)
{
  return k_uptime_get () / 1000;
}
#endif

void iot_logger_set_level (iot_logger_t * logger, iot_loglevel_t level)
{
  assert (logger);
  logger->level = level;
}

iot_logger_t * iot_logger_alloc (const char * name, iot_loglevel_t level)
{
  return iot_logger_alloc_custom (name, level, NULL, iot_log_console, NULL);
}

iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, const char * to, iot_log_function_t impl, iot_logger_t * next)
{
  assert (name && impl);
  iot_logger_t * logger = calloc (1, sizeof (*logger));
  logger->impl = impl;
  logger->name = iot_strdup (name);
  logger->to = to ? iot_strdup (to) : NULL;
  logger->save = level;
  logger->next = next;
  iot_component_init (&logger->component, (iot_component_start_fn_t) iot_logger_start, (iot_component_stop_fn_t) iot_logger_stop);
  return logger;
}

void iot_logger_free (iot_logger_t * logger)
{
  if (logger && (logger != &iot_logger_dfl) && iot_component_free (&logger->component))
  {
    free (logger->name);
    free (logger->to);
    free (logger);
  }
}

void iot_logger_add_ref (iot_logger_t * logger)
{
  if (logger) iot_component_add_ref (&logger->component);
}

bool iot_logger_start (iot_logger_t * logger)
{
  assert (logger);
  if (logger->component.state != IOT_COMPONENT_RUNNING)
  {
    logger->component.state = IOT_COMPONENT_RUNNING;
    logger->level = logger->save;
  }
  return true;
}

void iot_logger_stop (iot_logger_t * logger)
{
  assert (logger);
  if (logger->component.state != IOT_COMPONENT_STOPPED)
  {
    logger->component.state = IOT_COMPONENT_STOPPED;
    logger->save = logger->level;
    logger->level = IOT_LOG_NONE;
  }
}

iot_logger_t * iot_logger_next (iot_logger_t * logger)
{
  assert (logger);
  return logger->next;
}

static inline void iot_logger_log_to_fd (FILE * fd, const char *name, iot_loglevel_t level, time_t timestamp, const char *message)
{
  char tname[IOT_PRCTL_NAME_MAX] = { 0 };
#if defined (__linux__)
  prctl (PR_GET_NAME, tname);
#endif
  fprintf (fd, "[%s:%" PRIu64 ":%s:%s] %s\n", tname, (uint64_t) timestamp, name ? name : "default", iot_log_levels[level], message);
}

void iot_log_file (iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message)
{
  FILE * fd = fopen (logger->to, "a");
  if (fd)
  {
    iot_logger_log_to_fd (fd, logger->name, level, timestamp, message);
    fclose (fd);
  }
}

extern void iot_log_console (iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message)
{
  iot_logger_log_to_fd ((level > IOT_LOG_WARN) ? stdout : stderr, logger->name, level, timestamp, message);
}

#ifdef IOT_BUILD_COMPONENTS

static iot_component_t * iot_logger_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * next = NULL;
  iot_log_function_t impl = NULL;
  iot_logger_t * logger;
  iot_loglevel_t level = IOT_LOGLEVEL_DEFAULT;
  const char * name;
  const char * to = iot_data_string_map_get_string (map, "To");

  if (to && strncmp (to, "file:", 5) == 0 && strlen (to) > 5)
  {
    impl = iot_log_file; /* Log to file */
    to += 5;
  }
  else
  {
    impl = iot_log_console; /* log to stderr or stdout */
  }
  name = iot_data_string_map_get_string (map, "Next");
  if (name)
  {
    next = (iot_logger_t*) iot_container_find (cont, name);
  }
  name = iot_data_string_map_get_string (map, "Level");
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
  logger = iot_logger_alloc_custom (iot_data_string_map_get_string (map, "Name"), level, to, impl, next);
  return &logger->component;
}

const iot_component_factory_t * iot_logger_factory (void)
{
  static iot_component_factory_t factory = { IOT_LOGGER_TYPE, iot_logger_config, (iot_component_free_fn_t) iot_logger_free };
  return &factory;
}

#endif
