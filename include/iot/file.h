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

#ifdef IOT_HAS_FILE

#ifdef __cplusplus
extern "C" {
#endif

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
#endif
