//
// Copyright (c) 2019-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_IOT_H_
#define _IOT_IOT_H_

/**
 * @file
 * @brief IOTech IOT API
 */

#include "iot/defs.h"
#include "iot/data.h"
#include "iot/config.h"
#include "iot/base64.h"
#include "iot/time.h"
#include "iot/hash.h"
#include "iot/component.h"
#include "iot/container.h"
#include "iot/logger.h"
#include "iot/thread.h"
#include "iot/json.h"
#include "iot/scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start IOT / Initialise IOT data
 */
extern void iot_init (void);

/**
 * @brief Stop IOT / Cleanup IOT data
 */
extern void iot_fini (void);

/**
 * @brief Load configuration from file
 *
 * The function to load a configuration from a file
 *
 * @param name  Name of the configuration
 * @param uri   URI for configuration file
 * @return      Configuration string loaded from the file
 */
extern char * iot_file_config_loader (const char * name, const char * uri);

/**
 * @brief Save configuration to file
 *
 * The function to save a configuration to a file
 *
 * @param name   Name of the configuration
 * @param uri    URI for configuration file
 * @param config Configuration string to save
 * @return       Whether configuration saved successfully
 */
extern bool iot_file_config_saver (const char * name, const char * uri, const char * config);


/**
 * @brief Load string from a file
 *
 * Function to return file contents as a NULL terminated string
 *
 * @param path  File path
 * @return      String loaded from the file (client needs to free)
 */
extern char * iot_file_read (const char * path);

/**
 * @brief Write a string to a file
 *
 * Function to write a NULL terminated string to a file, overwriting any existing contents.
 *
 * @param path  File path
 * @param str   String to write
 * @return      Whether the string was successfully written to the file
 */
extern bool iot_file_write (const char * path, const char * str);

/**
 * @brief Delete a file
 *
 * Function to delete a file
 *
 * @param path  File path
 * @return      Whether the file was successfully deleted
 */
extern bool iot_file_delete (const char * path);

/**
 * @brief Load binary data from a file
 *
 * Function to return a binary array of file contents
 *
 * @param path  File path
 * @param len   Length of returned binary array
 * @return      Binary contents of file (client needs to free)
 */
extern uint8_t * iot_file_read_binary (const char * path, size_t * len);

/**
 * @brief Write binary data to a file
 *
 * Function to write a binary of given size to a file, overwriting any existing contents
 *
 * @param path   File path
 * @param binary The binary array to be written
 * @param len    Length of binary array to be written
 * @return       Whether the binary was successfully written to the file
 */
extern bool iot_file_write_binary (const char * path, const uint8_t * binary, size_t len);

#ifdef __cplusplus
}
#endif
#endif
