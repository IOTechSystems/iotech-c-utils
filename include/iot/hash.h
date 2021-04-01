//
// Copyright (c) 2019-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_HASH_H_
#define _IOT_HASH_H_

/**
 * @file
 * @brief IOTech Hash API
 */

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bernstein djb2 hash function (Version 2)
 *
 * @param str String to be hashed
 * @return    Hash value
 */
extern uint32_t iot_hash (const char * str);

/**
 * @brief Bernstein djb2 hash function (Version 2)
 *
 * @param data Pointer to data to be hashed
 * @param len  Length of data to be hashed
 * @return     Hash value
 */
extern uint32_t iot_hash_data (const uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif
#endif
