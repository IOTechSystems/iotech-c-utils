//
// Copyright (c) 2018,2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/logger.h"
#include "iot/container.h"
#include <stdarg.h>

// TODO: allow an array of labels in a log message

static const char * iot_log_levels[6] = {"NONE", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
static iot_logger_t iot_logger_dfl;

iot_logger_t * iot_logger_default (void)
{
  static iot_logger_t * logger = NULL;
  if (logger == NULL)
  {
    logger = &iot_logger_dfl;
    memset (&iot_logger_dfl, 0, sizeof (iot_logger_dfl));
    iot_logger_dfl.level = IOT_LOG_WARN;
    iot_logger_dfl.impl = iot_logger_console;
    iot_logger_dfl.dest = "stdout";
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
  if (logger->sub && logger->sub->level >= level) (logger->impl) (logger, level, ts, str);
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

void iot_logger_setlevel (iot_logger_t * logger, iot_loglevel_t level)
{
  assert (logger);
  logger->level = level;
}

iot_logger_t * iot_logger_alloc (const char * subsys, const char * dest, iot_log_function_t impl, iot_logger_t * sub)
{
  assert (subsys && impl);
  iot_logger_t * logger = malloc (sizeof (*logger));
  logger->impl = impl;
  logger->subsys = iot_strdup (subsys);
  logger->dest = dest ? iot_strdup (dest) : NULL;
  logger->level = IOT_LOG_WARN;
  logger->component.start_fn = (iot_component_start_fn_t) iot_logger_start;
  logger->component.stop_fn = (iot_component_stop_fn_t) iot_logger_stop;
  logger->sub = sub;
  return logger;
}

void iot_logger_free (iot_logger_t * logger)
{
  if (logger && (logger != &iot_logger_dfl))
  {
    iot_logger_free (logger->sub);
    free (logger->subsys);
    free (logger->dest);
    free (logger);
  }
}

bool iot_logger_start (iot_logger_t * logger)
{
  assert (logger);
  logger->component.state = IOT_COMPONENT_RUNNING;
  return true;
}

void iot_logger_stop (iot_logger_t * logger)
{
  assert (logger);
  logger->component.state = IOT_COMPONENT_STOPPED;
}

iot_logger_t * iot_logger_sub (iot_logger_t * logger)
{
  assert (logger);
  return logger->sub;
}

static inline void iot_logger_log_to_fd (FILE * fd, const char *subsys, iot_loglevel_t level, time_t timestamp, const char *message)
{
  fprintf (fd, "%" PRId64 " %s: %s: %s\n", (int64_t) timestamp, subsys ? subsys : "(default)", iot_log_levels[level], message);
}

void iot_logger_file (iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message)
{
  FILE * fd = fopen (logger->dest, "a");
  if (fd)
  {
    iot_logger_log_to_fd (fd, logger->subsys, level, timestamp, message);
    fclose (fd);
  }
}

extern void iot_logger_console (iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message)
{
  iot_logger_log_to_fd ((level > IOT_LOG_WARN) ? stdout : stderr, logger->subsys, level, timestamp, message);
}

#ifdef IOT_BUILD_COMPONENTS

static iot_component_t * iot_logger_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * sub = NULL;
  iot_log_function_t impl = NULL;
  iot_logger_t * logger;
  const char * dest = iot_data_string_map_get_string (map, "Destination");
  const char * name = iot_data_string_map_get_string (map, "SubLogger");

  if (dest && strncmp (dest, "file:", 5) == 0 && strlen (dest) > 5)
  {
    impl = iot_logger_file; /* Log to file */
    dest += 5;
  }
  else
  {
    impl = iot_logger_console; /* log to stderr or stdout */
    if (dest == NULL || (strcmp (dest, "stderr") != 0))
    {
      dest = "stdout";
    }
  }

  if (name)
  {
    sub = (iot_logger_t*) iot_container_find (cont, name);
  }
  logger = iot_logger_alloc (iot_data_string_map_get_string (map, "SubSystem"), dest, impl, sub);
  return &logger->component;
}

const iot_component_factory_t * iot_logger_factory (void)
{
  static iot_component_factory_t factory = { IOT_LOGGER_TYPE, iot_logger_config, (iot_component_free_fn_t) iot_logger_free };
  return &factory;
}

#endif
