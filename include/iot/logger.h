//
// Copyright (c) 2018-2020 IOTech Ltd
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

/** Logger structure */
struct iot_logger_t;

/**
 * Alias for log level enumeration
 */
typedef enum iot_loglevel_t
{
  IOT_LOG_NONE = 0,   /**< No logger */
  IOT_LOG_ERROR,      /**< Log level 1 */
  IOT_LOG_WARN,       /**< Log level 2 */
  IOT_LOG_INFO,       /**< Log level 3 */
  IOT_LOG_DEBUG,      /**< Log level 4 */
  IOT_LOG_TRACE       /**< Log level 5 - max IOT debug level */
} iot_loglevel_t;

/** Alias for IOT logger function pointer */
typedef void (*iot_log_function_t) (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message);

/* Default set of supported logger implementation functions */

#ifdef IOT_HAS_FILE
/**
 * @brief Log message to a file
 *
 * @param logger     Pointer to a logger component, to get the component name. "default" is used if logger is NULL
 * @param level      Log level
 * @param timestamp  Timestamp information to log
 * @param message    Log message
 */
extern void iot_log_file (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message);
#endif

/**
 * @brief Log message to console
 *
 * @param logger     Pointer to a logger component, to get the component name. "default" is used if logger is NULL
 * @param level      Log level
 * @param timestamp  Timestamp information to log
 * @param message    Log message
 */
extern void iot_log_console (struct iot_logger_t * logger, iot_loglevel_t level, time_t timestamp, const char * message);

/**
 * Alias for logger structure
 */
typedef struct iot_logger_t
{
  iot_component_t component;      /**< Details of a given logger component */
  volatile iot_loglevel_t level;  /**< Log level */
  volatile iot_loglevel_t save;   /**< Last saved log level */
  char * name;                    /**< Name of logger */
  char * to;                      /**< Log 'to' a certain file */
  iot_log_function_t impl;        /**< Pointer to function that handles logging functionality */
  struct iot_logger_t * next;     /**< Pointer to next logger structure */
} iot_logger_t;

/**
 * @brief Allocate memory and initialize logger component with the custom log function
 *
 * The function to allocate memory and initialize logger component to use custom defined log function. By default, the  logger logs messages to console
 *
 * @param name        Identifier to use for logging
 * @param level       Log level, Messages are filtered for logging based on the log level set
 * @param to          File and location of the logger file, if iot_log_file option is used
 * @param impl        Custom log function
 * @param next        Another logger component, this logger component must be pre-initialized
 * @param self_start  'true' will start the logger after initialization
 * @return            Pointer to the logger component created
 */
extern iot_logger_t * iot_logger_alloc_custom (const char * name, iot_loglevel_t level, const char * to, iot_log_function_t impl, iot_logger_t * next, bool self_start);

/**
 * @brief Allocate memory an initialize logger component
 *
 * The function to allocate memory and initialize logger component. The logger component logs messages to console
 *
 * @param name        Identifier to use for logging
 * @param level       Log level
 * @param self_start  'true' will start the logger after initialization
 * @return            Pointer to the logger component created
 */
extern iot_logger_t * iot_logger_alloc (const char * name, iot_loglevel_t level, bool self_start);

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

/**
 * @brief Get next logger
 *
 * @param logger  Pointer to the logger component
 * @return        Pointer to the next logger component stored by the logger
 */
extern iot_logger_t * iot_logger_next (iot_logger_t * logger);

/* Logging functions */
/**
 * @brief Log message with the log level set to Trace
 *
 * @param logger  Pointer to the logger component
 * @param ...     Formatted string for logging
 */
extern void iot_log__trace (iot_logger_t * logger, ...);

/**
 * @brief Log message with the log level set to Debug
 *
 * @param logger  Pointer to the logger component
 * @param ...     Formatted string for logging
 */
extern void iot_log__debug (iot_logger_t * logger, ...);

/**
 * @brief Log message with the log level set to Info
 *
 * @param logger  Pointer to the logger component
 * @param ...     Formatted string for logging
 */
extern void iot_log__info (iot_logger_t * logger, ...);

/**
 * @brief Log message with the log level set to Warning
 *
 * @param logger  Pointer to the logger component
 * @param ...     Formatted string for logging
 */
extern void iot_log__warn (iot_logger_t * logger, ...);

/**
 * @brief Log message with the log level set to Error
 *
 * @param logger  Pointer to the logger component
 * @param ...     Formatted string for logging
 */
extern void iot_log__error (iot_logger_t * logger, ...);

/** Log trace macro */
#define iot_log_trace(l,...) if ((l) && (l)->level >= IOT_LOG_TRACE) iot_log__trace ((l), __VA_ARGS__)
/** Log info macro */
#define iot_log_info(l,...) if ((l) && (l)->level >= IOT_LOG_INFO) iot_log__info ((l), __VA_ARGS__)
/** Log debug macro */
#define iot_log_debug(l,...) if ((l) && (l)->level >= IOT_LOG_DEBUG) iot_log__debug ((l), __VA_ARGS__)
/** Log warn macro */
#define iot_log_warn(l,...) if ((l) && (l)->level >= IOT_LOG_WARN) iot_log__warn ((l), __VA_ARGS__)
/** Log error macro */
#define iot_log_error(l,...) if ((l) && (l)->level >= IOT_LOG_ERROR) iot_log__error ((l), __VA_ARGS__)

/**
 * @brief  Set log level for the logger
 *
 * @param logger  Pointer to the logger
 * @param level   Log level
 */
extern void iot_logger_set_level (iot_logger_t *logger, iot_loglevel_t level);

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
