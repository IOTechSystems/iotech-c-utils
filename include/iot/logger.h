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
#define IOT_LOGLEVEL_DEFAULT IOT_LOG_WARN

struct iot_logger_t;
typedef enum iot_loglevel_t { IOT_LOG_NONE = 0, IOT_LOG_ERROR, IOT_LOG_WARN, IOT_LOG_INFO, IOT_LOG_DEBUG, IOT_LOG_TRACE } iot_loglevel_t;
typedef void (*iot_log_function_t) (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message);

/* Default set of supported logger implementation functions */

extern void iot_log_file (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message);
extern void iot_log_console (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message);

typedef struct iot_logger_t
{
  iot_component_t component;
  volatile iot_loglevel_t level;
  char * subsys;
  char * to;
  iot_log_function_t impl;
  struct iot_logger_t * next;
} iot_logger_t;

/* Logger lifecycle functions */

extern iot_logger_t * iot_logger_alloc (iot_loglevel_t level, const char * subsys, const char * to, iot_log_function_t impl, iot_logger_t * next);
extern void iot_logger_free (iot_logger_t * logger);
extern bool iot_logger_start (iot_logger_t * logger);
extern void iot_logger_stop (iot_logger_t * logger);

/* Default logging client: logs to stderr only */

extern iot_logger_t * iot_logger_default (void);

/* Get sub logger */

extern iot_logger_t * iot_logger_next (iot_logger_t * logger);

/* Logging macros */

extern void iot_log__trace (iot_logger_t * logger, ...);
extern void iot_log__debug (iot_logger_t * logger, ...);
extern void iot_log__info (iot_logger_t * logger, ...);
extern void iot_log__warn (iot_logger_t * logger, ...);
extern void iot_log__error (iot_logger_t * logger, ...);

#define iot_log_trace(l,...) if ((l)->level >= IOT_LOG_TRACE) iot_log__trace ((l), __VA_ARGS__)
#define iot_log_info(l,...) if ((l)->level >= IOT_LOG_INFO) iot_log__info ((l), __VA_ARGS__)
#define iot_log_debug(l,...) if ((l)->level >= IOT_LOG_DEBUG) iot_log__debug ((l), __VA_ARGS__)
#define iot_log_warn(l,...) if ((l)->level >= IOT_LOG_WARN) iot_log__warn ((l), __VA_ARGS__)
#define iot_log_error(l,...) if ((l)->level >= IOT_LOG_ERROR) iot_log__error ((l), __VA_ARGS__)

/* Set log level */

extern void iot_logger_setlevel (iot_logger_t *logger, iot_loglevel_t level);

/* Logger factory */

extern const iot_component_factory_t * iot_logger_factory (void);

#ifdef __cplusplus
}
#endif
#endif
