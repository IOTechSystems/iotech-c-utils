//
// Copyright (c) 2019-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_UTIL_H_
#define _IOT_UTIL_H_

/**
 * @file
 * @brief IOTech Util API
 */

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function to check if string is a valid UUID
 *
 * @param str String to be checked for (e.g. "6251d012-e0ac-4a75-85b8-dfa3e33ad0d9")
 * @return    Whether string is a valid UUID format
 */
extern bool iot_util_string_is_uuid (const char * str);

#ifdef __cplusplus
}
#endif
#endif
