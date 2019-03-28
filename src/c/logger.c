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

static const char * iot_log_levels[5] = {"INFO", "TRACE", "DEBUG", "WARNING", "ERROR"};

typedef struct impl_list
{
  iot_log_function_t fn;
  char *dest;
  struct impl_list *next;
} impl_list;

struct iot_logger_t
{
  iot_component_t component;
  char *subsystem;
  impl_list *loggers;
  pthread_mutex_t lock;
};

static void logthis (iot_logger_t *svc, iot_loglevel_t l, const char *fmt, va_list ap);
static void logtofd (FILE *, const char *, iot_loglevel_t, time_t, const char *);

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
  logthis (logger, INFO, fmt, args);
  va_end(args);
}

void iot_log_trace (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (logger, TRACE, fmt, args);
  va_end(args);
}

void iot_log_debug (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (logger, DEBUG, fmt, args);
  va_end(args);
}

void iot_log_warning (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (logger, WARNING, fmt, args);
  va_end(args);
}

void iot_log_error (iot_logger_t *logger, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (logger, ERROR, fmt, args);
  va_end(args);
}

static void logtofd (FILE *dest, const char *subsystem, iot_loglevel_t l, time_t timestamp, const char *message)
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
    logtofd (f, subsystem, l, timestamp, message);
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

static void logthis (iot_logger_t *logger, iot_loglevel_t l, const char *fmt, va_list ap)
{
  char str [1024];
  bool ok = false;
  time_t created = time (NULL);

  vsnprintf (str, sizeof (str), fmt, ap);

  // TODO: Queue this log message and signal the processing thread

  pthread_mutex_lock (&logger->lock);
  for (impl_list *impl = logger->loggers; impl; impl = impl->next)
  {
    ok |= impl->fn (impl->dest, logger->subsystem, l, created, str);
  }
  pthread_mutex_unlock (&logger->lock);

  if (!ok)
  {
    logtofd (stderr, logger->subsystem, l, created, str);
  }
}

iot_logger_t * iot_logger_alloc (const char *s)
{
  iot_logger_t *client;
  client = malloc (sizeof (iot_logger_t));
  client->subsystem = iot_strdup (s);
  client->loggers = NULL;
  pthread_mutex_init (&client->lock, NULL);
  return client;
}

void iot_logger_free (iot_logger_t *logger)
{
  impl_list *next;
  impl_list *i;
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

void iot_logger_add (iot_logger_t *logger, iot_log_function_t fn, const char *destination)
{
  if (logger == iot_log_default ())
  {
    iot_log_error (logger, "Request to add plugin to default logger - ignored");
    return;
  }
  impl_list * addthis = malloc (sizeof (*addthis));
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
  impl_list **iter = &logger->loggers;
  while (*iter)
  {
    if ((*iter)->fn == fn && strcmp ((*iter)->dest, destination) == 0)
    {
      impl_list *tmp = *iter;
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
