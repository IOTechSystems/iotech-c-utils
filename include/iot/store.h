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

#ifdef __cplusplus
extern "C" {
#endif

/** Function pointer type for read capability */
typedef uint8_t * (*iot_store_read_fn) (const char * path, size_t * len);
/** Function pointer type for write capability */
typedef bool (*iot_store_write_fn) (const char * path, const uint8_t * binary, size_t len);
/** Function pointer type for delete capability */
typedef bool (*iot_store_delete_fn) (const char * path);

/** Function pointer used to indirect data read operations */
extern iot_store_read_fn iot_store_reader;
/** Function pointer used to indirect data write operations */
extern iot_store_write_fn iot_store_writer;
/** Function pointer used to indirect data delete operations */
extern iot_store_delete_fn iot_store_deleter;

/**
 * @brief Load string from store path
 *
 * Function to return store contents as a NULL terminated string
 *
 * @param path  Path to data in store
 * @return      String loaded from the store or NULL if data not found (client needs to free)
 */
extern char * iot_store_read (const char * path);

/**
 * @brief Write a string to a store
 *
 * Function to write a NULL terminated string to a store, overwriting any existing contents.
 *
 * @param path  Path to to write data in store
 * @param str   String to write
 * @return      Whether the string was successfully written to the store path
 */
extern bool iot_store_write (const char * path, const char * str);

/**
 * @brief Delete data in a store
 *
 * Function to delete data
 *
 * @param path Path for stored data
 * @return     Whether the data was successfully deleted
 */
extern bool iot_store_delete (const char * path);

/**
 * @brief Load binary data from a store
 *
 * Function to return a binary array of store contents
 *
 * @param path  Path to read data in store
 * @param len   Length of returned binary array
 * @return      Binary contents of store or NULL if data not found (client needs to free)
 */
extern uint8_t * iot_store_read_binary (const char * path, size_t * len);

/**
 * @brief Write binary data to a store
 *
 * Function to write a binary of given size to a store, overwriting any existing contents
 *
 * @param path   Path to to write data in store
 * @param binary The binary array to be written
 * @param len    Length of binary array to be written
 * @return       Whether the binary was successfully written to the store
 */
extern bool iot_store_write_binary (const char * path, const uint8_t * binary, size_t len);

/**
 * @brief Load configuration from store
 *
 * The function to load a configuration from a file
 *
 * @param name  Name of the configuration
 * @param uri   URI for configuration
 * @return      Configuration string from store
 */
extern char * iot_store_config_loader (const char * name, const char * uri);

/**
 * @brief Save configuration to store
 *
 * The function to save a configuration to store
 *
 * @param name   Name of the configuration
 * @param uri    URI for configuration
 * @param config Configuration string to save
 * @return       Whether configuration saved successfully
 */
extern bool iot_store_config_saver (const char * name, const char * uri, const char * config);

#ifdef __cplusplus
}
#endif
#endif
