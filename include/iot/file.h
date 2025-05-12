//
// Copyright (c) 2022 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_FILE_H_
#define _IOT_FILE_H_

/**
 * @file
 * @brief IOTech File API
 */

#include "iot/os.h"
#include "iot/iot.h"

#ifdef IOT_HAS_FILE

#ifdef __cplusplus
extern "C" {
#endif

/** File change flags for iot_file_watch function */

extern const uint32_t iot_file_deleted_flag;
extern const uint32_t iot_file_modified_flag;

/**
 * @brief Load string from file path, returns file contents as a NULL terminated string
 *
 * @param path  File path
 * @return      String loaded from the file or NULL if file not found (client needs to free)
 */
extern char * iot_file_read (const char * path);

/**
 * @brief Writes a NULL terminated string to a file, overwriting any existing contents
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

/**
 * @brief List files in given directory
 * @param directory Directory in which to list files
 * @param regex_str Optional regex string which files must match
 * @return List of files
 */
extern iot_data_t * iot_file_list (const char * directory, const char * regex_str);

/**
 * @brief Watch a file for changes
 *
 * Function to watch a file for changes. If file exists, will block until file content changed or file deleted.
 *
 * @param path  File path
 * @param mask  Mask of events to watch for (iot_file_deleted_flag, iot_file_changed_flag)
 * @return      File change or zero if no change or no file
 */
extern uint32_t iot_file_watch (const char * path, uint32_t mask);

#ifdef __cplusplus
}
#endif
#endif
#endif
