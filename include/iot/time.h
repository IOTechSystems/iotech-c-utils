//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_TIME_H_
#define _IOT_TIME_H_

/**
 * @file
 * @brief IOTech Time API
 */

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get Unix time in seconds
 *
 * @return Unix time in seconds
 */
extern uint64_t iot_time_secs (void);

/**
 * @brief Get Unix time in milliseconds
 *
 * @return Unix time in milliseconds
 */
extern uint64_t iot_time_msecs (void);

/**
 * @brief Get Unix time in microseconds
 *
 * @return Unix time in microseconds
 */
extern uint64_t iot_time_usecs (void);

/**
 * @brief Get Unix time in nanoseconds
 *
 * @return Unix time in nanoseconds
 */
extern uint64_t iot_time_nsecs (void);

#ifdef __cplusplus
}
#endif
#endif
