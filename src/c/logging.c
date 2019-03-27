//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/os.h"
#include "iot/logging.h"

#include <stdarg.h>

// TODO: allow an array of labels in a log message

static const char * iot_log_levels[5] = {"INFO", "TRACE", "DEBUG", "WARNING", "ERROR"};

typedef struct impl_list
{
  iot_log_function_t fn;
  char *dest;
  struct impl_list *next;
} impl_list;

struct iot_logging_client_t
{
  char *subsystem;
  impl_list *loggers;
  pthread_mutex_t lock;
};

static void logthis (iot_logging_client_t *svc, iot_loglevel_t l, const char *fmt, va_list ap);
static void logtofd (FILE *, const char *, iot_loglevel_t, time_t, const char *);

iot_logging_client_t * iot_log_default (void)
{
  static iot_logging_client_t dfl;
  static iot_logging_client_t * logger = NULL;
  if (logger == NULL)
  {
    logger = &dfl;
    dfl.subsystem = NULL;
    dfl.loggers = NULL;
    pthread_mutex_init (&dfl.lock, NULL);
  }
  return logger;
}

void iot_log_info (iot_logging_client_t *lc, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (lc, INFO, fmt, args);
  va_end(args);
}

void iot_log_trace (iot_logging_client_t *lc, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (lc, TRACE, fmt, args);
  va_end(args);
}

void iot_log_debug (iot_logging_client_t *lc, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (lc, DEBUG, fmt, args);
  va_end(args);
}

void iot_log_warning (iot_logging_client_t *lc, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (lc, WARNING, fmt, args);
  va_end(args);
}

void iot_log_error (iot_logging_client_t *lc, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  logthis (lc, ERROR, fmt, args);
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

static void logthis (iot_logging_client_t *lc, iot_loglevel_t l, const char *fmt, va_list ap)
{
  char str [1024];
  bool ok = false;
  time_t created = time (NULL);

  vsnprintf (str, sizeof (str), fmt, ap);

  // TODO: Queue this log message and signal the processing thread

  pthread_mutex_lock (&lc->lock);
  for (impl_list *impl = lc->loggers; impl; impl = impl->next)
  {
    ok |= impl->fn (impl->dest, lc->subsystem, l, created, str);
  }
  pthread_mutex_unlock (&lc->lock);

  if (!ok)
  {
    logtofd (stderr, lc->subsystem, l, created, str);
  }
}

iot_logging_client_t * iot_logging_client_create (const char *s)
{
  iot_logging_client_t *client;
  client = malloc (sizeof (iot_logging_client_t));
  client->subsystem = iot_strdup (s);
  client->loggers = NULL;
  pthread_mutex_init (&client->lock, NULL);
  return client;
}

void iot_logging_client_destroy (iot_logging_client_t *lc)
{
  impl_list *next;
  impl_list *i;
  pthread_mutex_lock (&lc->lock);
  i = lc->loggers;
  lc->loggers = NULL;
  pthread_mutex_unlock (&lc->lock);
  while (i)
  {
    next = i->next;
    free (i->dest);
    free (i);
    i = next;
  }
  pthread_mutex_destroy (&lc->lock);
  free (lc->subsystem);
  free (lc);
}

void iot_log_addlogger
  (iot_logging_client_t *lc, iot_log_function_t fn, const char *destination)
{
  if (lc == iot_log_default ())
  {
    iot_log_error (lc, "Request to add plugin to default logger - ignored");
    return;
  }
  impl_list *addthis = malloc (sizeof (impl_list));
  addthis->dest = iot_strdup (destination);
  addthis->fn = fn;
  pthread_mutex_lock (&lc->lock);
  addthis->next = lc->loggers;
  lc->loggers = addthis;
  pthread_mutex_unlock (&lc->lock);
}

void iot_log_dellogger (iot_logging_client_t *lc, iot_log_function_t fn, const char *destination)
{
  pthread_mutex_lock (&lc->lock);
  impl_list **iter = &lc->loggers;
  while (*iter)
  {
    if ((*iter)->fn == fn && strcmp ((*iter)->dest, destination) == 0)
    {
      impl_list *tmp = *iter;
      *iter = (*iter)->next;
      free (*iter);
      break;
    }
    iter = &((*iter)->next);
  }
  pthread_mutex_unlock (&lc->lock);
}
