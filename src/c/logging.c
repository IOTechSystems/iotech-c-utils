//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/logging.h"

#include <string.h>
#include <pthread.h>
#include <stdarg.h>


// TODO: allow an array of labels in a log message

static const char *levelstrs[] =
   { "INFO", "TRACE", "DEBUG", "WARNING", "ERROR" };

typedef struct impl_list
{
   iot_log_function fn;
   char *dest;
   struct impl_list *next;
} impl_list;

struct iot_logging_client
{
   char *subsystem;
   impl_list *loggers;
   pthread_mutex_t lock;
};

static void logthis
   (iot_logging_client *svc, iot_loglevel l, const char *fmt, va_list ap);

static void logtofd (FILE *, const char *, iot_loglevel, time_t, const char *);

static iot_logging_client dfl = { NULL, NULL, PTHREAD_MUTEX_INITIALIZER };

iot_logging_client *iot_log_default = &dfl;

void iot_log_info (iot_logging_client *lc, const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   logthis (lc, INFO, fmt, args);
   va_end(args);
}

void iot_log_trace (iot_logging_client *lc, const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   logthis (lc, TRACE, fmt, args);
   va_end(args);
}

void iot_log_debug (iot_logging_client *lc, const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   logthis (lc, DEBUG, fmt, args);
   va_end(args);
}

void iot_log_warning (iot_logging_client *lc, const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   logthis (lc, WARNING, fmt, args);
   va_end(args);
}

void iot_log_error (iot_logging_client *lc, const char *fmt, ...)
{
   va_list args;
   va_start (args, fmt);
   logthis (lc, ERROR, fmt, args);
   va_end(args);
}

void logtofd
(
   FILE *dest,
   const char *subsystem,
   iot_loglevel l,
   time_t timestamp,
   const char *message
)
{
   fprintf (dest, "%ld %s: %s: %s\n", timestamp, subsystem ? subsystem : "(default)", levelstrs [l], message);
}

bool iot_log_tofile
(
   const char *destination,
   const char *subsystem,
   iot_loglevel l,
   time_t timestamp,
   const char *message
)
{
   FILE *f;
   if (strcmp (destination, "-") == 0)
   {
     f = stdout;
   }
   else
   {
     f = fopen (destination, "a");
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


void logthis (iot_logging_client *lc, iot_loglevel l, const char *fmt, va_list ap)
{
   char *str = alloca (1024);
   bool ok = false;
   time_t created = time (NULL);

   vsnprintf (str, 1024, fmt, ap);

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


iot_logging_client *iot_logging_client_create (const char *s)
{
   iot_logging_client *client;
   client = malloc (sizeof (iot_logging_client));
   client->subsystem = strdup (s);
   client->loggers = NULL;
   pthread_mutex_init (&client->lock, NULL);
   return client;
}

void iot_logging_client_destroy (iot_logging_client *lc)
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
   (iot_logging_client *lc, iot_log_function fn, const char *destination)
{
   if (lc == iot_log_default)
   {
      iot_log_error (lc, "Request to add plugin to default logger - ignored");
      return;
   }
   impl_list *addthis = malloc (sizeof (impl_list));
   addthis->dest = strdup (destination);
   addthis->fn = fn;
   pthread_mutex_lock (&lc->lock);
   addthis->next = lc->loggers;
   lc->loggers = addthis;
   pthread_mutex_unlock (&lc->lock);
}

bool iot_log_dellogger
   (iot_logging_client *lc, iot_log_function fn, const char *destination)
{
   bool result = false;
   impl_list **iter = &lc->loggers;
   pthread_mutex_lock (&lc->lock);
   while (*iter)
   {
      if ((*iter)->fn == fn && strcmp ((*iter)->dest, destination) == 0)
      {
         impl_list *tmp = *iter;
         *iter = (*iter)->next;
         free (tmp);
         result = true;
         break;
      }
      iter = &((*iter)->next);
   }
   pthread_mutex_unlock (&lc->lock);
   return result;
}
