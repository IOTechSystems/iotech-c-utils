//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_LOGGING_H_
#define _IOT_LOGGING_H_ 1

#include "iot/os.h"

typedef enum iot_loglevel { INFO = 0, TRACE, DEBUG, WARNING, ERROR } iot_loglevel;

struct iot_logging_client;
typedef struct iot_logging_client iot_logging_client;

/**
 * Plugin function for loggers. This is called whenever a new message is logged.
 * @param destination Plugin-specific specification of where logs should go.
 * Could be a filename, URL etc.
 * @param subsystem Name of the subsystem creating the log message.
 * @param l INFO, TRACE, DEBUG etc.
 * @timestamp When the log message was generated.
 * @message The text of the log message.
 *
 * @return true if the log was successfully posted.
 */

typedef bool (*iot_log_function)
(
  const char *destination,
  const char *subsystem,
  iot_loglevel l,
  time_t timestamp,
  const char *message
);

/* Built-in logger: append to a file */

extern bool iot_log_tofile
(
  const char *dest,
  const char *subsystem,
  iot_loglevel l,
  time_t timestamp,
  const char *message
);

/* Create and destroy logging clients */

extern iot_logging_client *iot_logging_client_create (const char *subsystem);
extern void iot_logging_client_destroy (iot_logging_client *lc);

/* Default logging client: logs to stderr only */

extern iot_logging_client *iot_log_default;

/* Plug in and remove logger implementations */

extern void iot_log_addlogger
  (iot_logging_client *lc, iot_log_function fn, const char *destination);
extern bool iot_log_dellogger
  (iot_logging_client *lc, iot_log_function fn, const char *destination);

/* Functions to generate logs */

extern void iot_log_init ();
extern void iot_log_info (iot_logging_client *lc, const char *fmt, ...);
extern void iot_log_trace (iot_logging_client *lc, const char *fmt, ...);
extern void iot_log_debug (iot_logging_client *lc, const char *fmt, ...);
extern void iot_log_warning (iot_logging_client *lc, const char *fmt, ...);
extern void iot_log_error (iot_logging_client *lc, const char *fmt, ...);

#endif
