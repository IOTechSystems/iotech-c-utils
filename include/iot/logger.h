//
// Copyright (c) 2018-2022 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_LOGGER_H_
#define _IOT_LOGGER_H_

/**
 * @file
 * @brief IOTech Logger API
 */

#include "iot/component.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Logger component name */
#define IOT_LOGGER_TYPE "IOT::Logger"
/** Default log level warning */
#define IOT_LOGLEVEL_DEFAULT IOT_LOG_WARN
/** Maximum log message size */
#define IOT_LOG_MSG_MAX 1024

/**
 * Log level enumeration
 */
typedef enum iot_loglevel_t
{
  IOT_LOG_NONE = 0,      /**< No logging */
  IOT_LOG_ERROR = 1u,    /**< Error logging */
  IOT_LOG_WARN = 2u,     /**< Warning and Error logging */
  IOT_LOG_INFO = 3u,     /**< Information, Warning and Error logging */
  IOT_LOG_DEBUG = 4u,    /**< Debug, Information, Warning and Error logging */
  IOT_LOG_TRACE = 5u     /**< Trace, Debug, Information, Warning and Error logging */
} iot_loglevel_t;

/**
 * Public logger struct. Do not use directly or stack allocate.
 */
typedef struct iot_logger_t
{
  iot_component_t component;      /**< Component base */
  volatile iot_loglevel_t level;  /**< Log level */
} iot_logger_t;

/**
 * Logger implementation function type
 */
typedef void (*iot_log_function_t) (struct iot_logger_t * logger, iot_loglevel_t level, uint64_t timestamp, const char * message, const void *ctx);

/**
 * Logger context freeing function type
 */
typedef void (*iot_log_free_fn_t) (void *ctx);

/**
 * @brief Allocate memory and initialize logger component
 *
 * The function to allocate memory and initialize logger component. The logger component logs messages to console
 *
 * @param name        Identifier to use for logging
 * @param level       Log level
 * @param self_start  'true' will start the logger after initialization
 * @return            Pointer to the logger component created
 */
extern iot_logger_t * iot_logger_alloc (const char * name, iot_loglevel_t level, bool self_start);

#if defined (IOT_HAS_FILE) && !defined (_AZURESPHERE_)
/**
 * @brief Allocate memory and initialize file logger component
 *
 * The function to allocate memory and initialize file logger component. The logger component logs messages to a file
 *
 * @param name        Identifier to use for logging
 * @param level       Log level
 * @param self_start  'true' will start the logger after initialization
 * @param next        Another logger component, this logger component must be pre-initialized. May be NULL
 * @param pathname    File and location of the logfile
 * @return            Pointer to the logger component created
 */
extern iot_logger_t * iot_logger_alloc_file (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, const char *pathname);
#endif

/**
 * @brief Allocate memory and initialize UDP logger component
 *
 * The function to allocate memory and initialize UDP logger component. The logger component logs messages to a UDP port
 *
 * @param name        Identifier to use for logging
 * @param level       Log level
 * @param self_start  'true' will start the logger after initialization
 * @param next        Another logger component, this logger component must be pre-initialized. May be NULL
 * @param host        Address to send data - NULL means broadcast
 * @param port        Port on which to send data
 * @return            Pointer to the logger component created
 */
extern iot_logger_t * iot_logger_alloc_udp (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, const char *host, uint16_t port);

/**
 * @brief Allocate memory and initialize logger component with the custom log function
 *
 * The function to allocate memory and initialize logger component to use custom defined log function.
 *
 * @param name        Identifier to use for logging
 * @param level       Log level, Messages are filtered for logging based on the log level set
 * @param self_start  'true' will start the logger after initialization
 * @param next        Another logger component, this logger component must be pre-initialized. May be NULL
 * @param impl        Custom log function
 * @param ctx         Context to be passed to custom log function
 * @param freectx     Function to dispose of context
 * @return            Pointer to the logger component created
 */
extern iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, bool self_start, iot_logger_t * next, iot_log_function_t impl, void *ctx, iot_log_free_fn_t freectx);

/**
 * @brief Increment the logger reference count
 *
 * @param logger  Pointer to the logger component
 */
extern void iot_logger_add_ref (iot_logger_t * logger);

/**
 * @brief Free the resources used by the logger component
 *
 * @param logger  Pointer to the logger component
 */
extern void iot_logger_free (iot_logger_t * logger);

/**
 * @brief Start logger and set the component state to IOT_COMPONENT_RUNNING
 *
 * @param logger  Pointer to the logger component
 */
extern void iot_logger_start (iot_logger_t * logger);

/**
 * @brief Stop logger and set the component state to IOT_COMPONENT_STOPPED
 *
 * @param logger  Pointer to the logger component
 * @return        'true', if the state transition occurred, 'false' otherwise
 */
extern void iot_logger_stop (iot_logger_t * logger);

/**
 * @brief Default logging client to log to console -stdout/stderr only
 *
 * @return  Pointer to the logging component
 */
extern iot_logger_t * iot_logger_default (void);

/* Logging functions */
/**
 * @brief Log message with a specified log level
 *
 * @param logger  Pointer to the logger component
 * @param level   Log level for this entry
 * @param ...     Formatted string for logging
 */
extern void iot_log__log (iot_logger_t * logger, iot_loglevel_t level, ...);

/**
 * @brief Log message format string and arguments with a specified log level
 *
 * @param logger  Pointer to the logger component
 * @param level   Log level for this entry
 * @param fmt     Format string for logging
 * @param args    va list arguments for logging
 */
extern void iot_log__va_log (iot_logger_t * logger, iot_loglevel_t level, const char* fmt, va_list args);

/** Log trace macro */
#define iot_log_trace(l,...) if ((l) && (l)->level >= IOT_LOG_TRACE) iot_log__log ((l), IOT_LOG_TRACE, __VA_ARGS__)
/** Log info macro */
#define iot_log_info(l,...) if ((l) && (l)->level >= IOT_LOG_INFO) iot_log__log ((l), IOT_LOG_INFO, __VA_ARGS__)
/** Log debug macro */
#define iot_log_debug(l,...) if ((l) && (l)->level >= IOT_LOG_DEBUG) iot_log__log ((l), IOT_LOG_DEBUG, __VA_ARGS__)
/** Log warn macro */
#define iot_log_warn(l,...) if ((l) && (l)->level >= IOT_LOG_WARN) iot_log__log ((l), IOT_LOG_WARN, __VA_ARGS__)
/** Log error macro */
#define iot_log_error(l,...) if ((l) && (l)->level >= IOT_LOG_ERROR) iot_log__log ((l), IOT_LOG_ERROR, __VA_ARGS__)
/** Log macro */
#define iot_log_log(l,lv,...) if ((l) && (l)->level >= (lv)) iot_log__log ((l), (lv), __VA_ARGS__)

/**
 * @brief  Set log level for the logger
 *
 * @param logger  Pointer to the logger
 * @param level   Log level
 */
extern void iot_logger_set_level (iot_logger_t *logger, iot_loglevel_t level);

/**
 * @brief Parse string into log level
 * @param str level string
 * @return log level corresponding to the string, or IOT_LOGLEVEL_DEFAULT
 */
extern iot_loglevel_t iot_logger_level_from_string (const char * str);

/**
 * @brief Get string representation of log level
 * @param level log level
 * @return string corresponding to level
 */
extern const char * iot_logger_level_to_string (iot_loglevel_t level);

/**
 * @brief Add next logger to a logger (for chained logging)
 * @param logger Logger
 * @param next Next logger
 */
extern void iot_logger_set_next (iot_logger_t * logger, iot_logger_t * next);

/**
 * @brief Create Logger component factory
 *
 * @return  Pointer to Logger component factory
 */
extern const iot_component_factory_t * iot_logger_factory (void);

#ifdef __cplusplus
}
#endif
#endif
