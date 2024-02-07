//
// Copyright (c) 2022 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_STORE_H_
#define _IOT_STORE_H_

/**
 * @file
 * @brief IOTech abstract data store
 */

#include "iot/os.h"

#ifndef _AZURESPHERE_
#include <dirent.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Function pointer type for read capability */
typedef uint8_t * (*iot_store_read_fn) (const char * path, size_t * len);
/** Function pointer type for write capability */
typedef bool (*iot_store_write_fn) (const char * path, const uint8_t * binary, size_t len);
/** Function pointer type for delete capability */
typedef bool (*iot_store_delete_fn) (const char * path);

/**
 * @brief Configure store to use given functions for read, write and delete implementation
 *
 * @param read_fn  Read function pointer
 * @param write_fn Write function pointer
 * @param del_fn   Delete function pointer
 */
extern void iot_store_config (iot_store_read_fn read_fn, iot_store_write_fn write_fn, iot_store_delete_fn del_fn);

/**
 * @brief Load string from store path, returns store contents as a NULL terminated string
 *
 * @param path  Path to data in store
 * @return      String loaded from the store or NULL if data not found (client needs to free)
 */
extern char * iot_store_read (const char * path);

/**
 * @brief Writes a NULL terminated string to a store, overwriting any existing contents
 *
 * @param path  Path to write data in store
 * @param str   String to write
 * @return      Whether the string was successfully written to the store path
 */
extern bool iot_store_write (const char * path, const char * str);

/**
 * @brief Delete data in a store
 *
 * @param path Path for stored data
 * @return     Whether the data was successfully deleted
 */
extern bool iot_store_delete (const char * path);

/**
 * @brief Load binary data from a store, retuning data and length
 *
 * @param path  Path to read data in store
 * @param len   Length of returned binary array
 * @return      Binary contents of store or NULL if data not found (client needs to free)
 */
extern uint8_t * iot_store_read_binary (const char * path, size_t * len);

/**
 * @brief Write binary data to a store, overwriting any existing contents
 *
 * @param path   Path to write data in store
 * @param binary The binary array to be written
 * @param len    Length of binary array to be written
 * @return       Whether the binary was successfully written to the store
 */
extern bool iot_store_write_binary (const char * path, const uint8_t * binary, size_t len);

/**
 * @brief Load JSON configuration from store
 *
 * @param name  Name of the configuration
 * @param uri   URI for configuration
 * @return      JSON Configuration string from store
 */
extern char * iot_store_config_load (const char * name, const char * uri);

/**
 * @brief Save JSON configuration to store
 *
 * @param name   Name of the configuration
 * @param uri    URI for configuration
 * @param config Configuration string to save
 * @return       Whether configuration was saved
 */
extern bool iot_store_config_save (const char * name, const char * uri, const char * config);

/**
 * @brief Delete JSON configuration from store
 *
 * @param name  Name of the configuration
 * @param uri   URI for configuration
 * @return      Whether configuration was deleted
 */
extern bool iot_store_config_delete (const char * name, const char * uri);

/**
 * @brief List configuration files in given directory
 * @param directory Directory in which to list files
 * @return List of file names with '.json' removed
 */
iot_data_t * iot_store_config_list (const char * directory);

#ifdef __cplusplus
}
#endif
#endif
