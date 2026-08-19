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

extern const uint32_t iot_file_self_delete_flag; /** File or directory deleted (path) */
extern const uint32_t iot_file_delete_flag;      /** Directory content deleted */
extern const uint32_t iot_file_modify_flag;      /** File or directory contents modified */

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
 * @brief Appends a NULL terminated string to a file
 *
 * @param path  File path
 * @param str   String to append
 * @return      Whether the string was successfully appended to the file
 */
extern bool iot_file_append (const char * path, const char * str);

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
 * @brief Write append binary data to a file
 *
 * Function to append binary data of given size to a file
 *
 * @param path   File path
 * @param binary The binary array to be appended
 * @param len    Length of binary array to be appended
 * @return       Whether the binary was successfully appended to the file
 */
extern bool iot_file_append_binary (const char * path, const uint8_t * binary, size_t len);

/**
 * @brief List files in given directory
 * @param directory Directory in which to list files
 * @param regex_str Optional regex string which files must match
 * @return List of files
 */
extern iot_data_t * iot_file_list (const char * directory, const char * regex_str);

/**
 * @brief Check if a file exists
 *
 * Function to check if a file or directory exists
 *
 * @param path  File path
 * @return      Whether the file or directory exists
 */
extern bool iot_file_exists (const char * path);

/**
 * @brief Watch a file for changes (wrapper for inotify)
 *
 * Function to watch a file or directory for changes. If file or directory exists,
 * will block until the file content changes or the file is deleted.
 *
 * @param path  File path
 * @param mask  Mask (OR) of events to watch for (iot_file_deleted_flag, iot_file_self_delete_flag, iot_file_changed_flag)
 * @return      File change (OR of change flags) or zero if no file
 */
extern uint32_t iot_file_watch (const char * path, uint32_t mask);

#ifdef __cplusplus
}
#endif
#endif
#endif
