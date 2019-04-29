//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "iot/logger.h"

#include <stdarg.h>

// TODO: allow an array of labels in a log message

static const char * iot_log_levels[5] = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR"};

typedef struct iot_logger_list
{
  iot_log_function_t fn;
  char *dest;
  struct iot_logger_list *next;
} iot_logger_list;

struct iot_logger_t
{
  iot_component_t component;
  iot_loglevel_t level;
  char *subsystem;
  iot_logger_list *loggers;
  pthread_mutex_t lock;
};

static void iot_logger_log (iot_logger_t * logger, iot_loglevel_t l, const char *fmt, va_list ap);
static void iot_logger_log_to_fd (FILE *, const char *, iot_loglevel_t, time_t, const char *);

iot_logger_t * iot_log_default (void)
{
  static iot_logger_t dfl;
  static iot_logger_t * logger = NULL;
  if (logger == NULL)
  {
    logger = &dfl;
    dfl.subsystem = NULL;
    dfl.loggers = NULL;
    pthread_mutex_init (&dfl.lock, NULL);
  }
  return logger;
}

void iot_log_info (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  iot_logger_log (logger, INFO, fmt, args);
  va_end(args);
}

void iot_log_trace (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  iot_logger_log (logger, TRACE, fmt, args);
  va_end(args);
}

void iot_log_debug (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  iot_logger_log (logger, DEBUG, fmt, args);
  va_end(args);
}

void iot_log_warning (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  iot_logger_log (logger, WARNING, fmt, args);
  va_end(args);
}

void iot_log_error (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  iot_logger_log (logger, ERROR, fmt, args);
  va_end(args);
}

static void iot_logger_log_to_fd (FILE *dest, const char *subsystem, iot_loglevel_t l, time_t timestamp, const char *message)
{
  fprintf (dest, "%" PRId64 " %s: %s: %s\n", (int64_t) timestamp, subsystem ? subsystem : "(default)", iot_log_levels[l], message);
}

bool iot_log_tofile (const char *dest, const char *subsystem, iot_loglevel_t l, time_t timestamp, const char *message)
{
  FILE *f;
  if (strcmp (dest, "-") == 0)
  {
    f = stdout;
  }
  else
  {
    f = fopen (dest, "a");
  }
  if (f)
  {
    iot_logger_log_to_fd (f, subsystem, l, timestamp, message);
    if (f != stdout)
    {
      fclose (f);
    }
    return true;
  }
  return false;
}

#ifdef __ZEPHYR__
static time_t time (time_t *t)
{
  return k_uptime_get () / 1000;
}
#endif

static void iot_logger_log (iot_logger_t *logger, iot_loglevel_t l, const char *fmt, va_list ap)
{
  if (l < logger->level)
  {
    return;
  }

  char str [1024];
  bool ok = false;
  time_t created = time (NULL);

  vsnprintf (str, sizeof (str), fmt, ap);

  // TODO: Queue this log message and signal the processing thread

  pthread_mutex_lock (&logger->lock);
  iot_logger_list * iter = logger->loggers;
  while (iter)
  {
    ok |= iter->fn (iter->dest, logger->subsystem, l, created, str);
    iter = iter->next;
  }
  pthread_mutex_unlock (&logger->lock);

  if (!ok)
  {
    iot_logger_log_to_fd (stderr, logger->subsystem, l, created, str);
  }
}

void iot_logger_setlevel (iot_logger_t *logger, iot_loglevel_t l)
{
  logger->level = l;
}

const char *iot_logger_levelname (iot_loglevel_t l)
{
  assert (l >= TRACE && l <= ERROR);
  return iot_log_levels[l];
}

iot_logger_t * iot_logger_alloc (const char * subsystem)
{
  iot_logger_t * logger;
  logger = calloc (1, sizeof (iot_logger_t));
  logger->subsystem = iot_strdup (subsystem);
  logger->level = TRACE;
  logger->component.start_fn = (iot_component_start_fn_t) iot_logger_start;
  logger->component.stop_fn = (iot_component_stop_fn_t) iot_logger_stop;
  pthread_mutex_init (&logger->lock, NULL);
  return logger;
}

void iot_logger_free (iot_logger_t *logger)
{
  iot_logger_list *next;
  iot_logger_list *i;
  i = logger->loggers;
  logger->loggers = NULL;
  while (i)
  {
    next = i->next;
    free (i->dest);
    free (i);
    i = next;
  }
  pthread_mutex_destroy (&logger->lock);
  free (logger->subsystem);
  free (logger);
}

bool iot_logger_start (iot_logger_t * logger)
{
  (void) logger;
  return true;
}

void iot_logger_stop (iot_logger_t * logger)
{
  (void) logger;
}

void iot_logger_add (iot_logger_t *logger, iot_log_function_t fn, const char *destination)
{
  if (logger == iot_log_default ())
  {
    iot_log_error (logger, "Request to add plugin to default logger - ignored");
    return;
  }
  iot_logger_list * addthis = malloc (sizeof (*addthis));
  addthis->dest = iot_strdup (destination);
  addthis->fn = fn;
  pthread_mutex_lock (&logger->lock);
  addthis->next = logger->loggers;
  logger->loggers = addthis;
  pthread_mutex_unlock (&logger->lock);
}

void iot_logger_remove (iot_logger_t *logger, iot_log_function_t fn, const char *destination)
{
  pthread_mutex_lock (&logger->lock);
  iot_logger_list **iter = &logger->loggers;
  while (*iter)
  {
    if ((*iter)->fn == fn && strcmp ((*iter)->dest, destination) == 0)
    {
      iot_logger_list *tmp = *iter;
      *iter = (*iter)->next;
      free (tmp);
      break;
    }
    iter = &((*iter)->next);
  }
  pthread_mutex_unlock (&logger->lock);
}

/* Container support */

static iot_component_t * iot_logger_config (iot_container_t * cont, const iot_data_t * map)
{
  iot_logger_t * logger = iot_logger_alloc (iot_data_string_map_get_string (map, "SubSystem"));
  return &logger->component;
}

const iot_component_factory_t * iot_logger_factory (void)
{
  static iot_component_factory_t factory = { IOT_LOGGER_TYPE, iot_logger_config, (iot_component_free_fn_t) iot_logger_free };
  return &factory;
}
