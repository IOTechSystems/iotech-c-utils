//
// Copyright (c) 2018 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_LOGGER_H_
#define _IOT_LOGGER_H_

#include "iot/component.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_LOGGER_TYPE "IOT::Logger"

typedef enum iot_loglevel_t { TRACE = 0, DEBUG, INFO, WARNING, ERROR } iot_loglevel_t;
typedef struct iot_logger_t iot_logger_t;

/**
 * Plugin function for loggers. This is called whenever a new message is logged.
 * @param destination Plugin-specific specification of where logs should go. Could be a filename, URL etc.
 * @param subsystem Name of the subsystem creating the log message.
 * @param level INFO, TRACE, DEBUG etc.
 * @timestamp When the log message was generated.
 * @message The text of the log message.
 *
 * @return true if the log was successfully posted.
 */

typedef bool (*iot_log_function_t)
(
  const char * destination,
  const char * subsystem,
  iot_loglevel_t level,
  time_t timestamp,
  const char *message
);

/* Built-in logger: append to a file */

extern bool iot_log_tofile
(
  const char * dest,
  const char * subsystem,
  iot_loglevel_t level,
  time_t timestamp,
  const char *message
);

/* Create and destroy logging clients */

extern iot_logger_t * iot_logger_alloc (const char * subsystem);
extern void iot_logger_free (iot_logger_t * logger);
extern bool iot_logger_start (iot_logger_t * logger);
extern void iot_logger_stop (iot_logger_t * logger);

/* Default logging client: logs to stderr only */

extern iot_logger_t * iot_log_default (void);

/* Plug in and remove logger implementations */

extern void iot_logger_add (iot_logger_t *logger, iot_log_function_t fn, const char * destination);
extern void iot_logger_remove (iot_logger_t *logger, iot_log_function_t fn, const char * destination);

/* Functions to generate logs */

extern void iot_log_info (iot_logger_t * logger, const char * fmt, ...);
extern void iot_log_trace (iot_logger_t * logger, const char * fmt, ...);
extern void iot_log_debug (iot_logger_t * logger, const char * fmt, ...);
extern void iot_log_warning (iot_logger_t * logger, const char * fmt, ...);
extern void iot_log_error (iot_logger_t * logger, const char * fmt, ...);

/* Set log level */

extern void iot_logger_setlevel (iot_logger_t *logger, iot_loglevel_t newlevel);

/* Get name of log level */

extern const char * iot_logger_levelname (iot_loglevel_t level);

/* Logger factory */

extern const iot_component_factory_t * iot_logger_factory (void);

#ifdef __cplusplus
}
#endif
#endif
