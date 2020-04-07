//
// Copyright (c) 2018-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_BASE64_H_
#define _IOT_BASE64_H_

/**
 * @file
 * @brief IOTech base64 API
 */

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the base64 encode size of the specified binary data
 *
 * @param binsize Size of binary data we wish to encode
 * @return        Size of base64 encoded binary data
 */
extern size_t iot_b64_encodesize (size_t binsize);

/**
 * @brief Get the maximum base64 decode size of the specified base64 encoded string
 *
 * @param in Pointer to base64 encoded string
 * @return   Maximum size of base64 decode
 */
extern size_t iot_b64_maxdecodesize (const char * in);

/**
 * @brief Decode base64 (encoded) string
 *
 * @param in     Pointer to base64 encoded string
 * @param out    General purpose pointer to the output of this base64 decode function
 * @param outLen Length of the decoded output
 * @return       'true' if decode successful, 'false' if decode in error
 */
extern bool iot_b64_decode (const char * in, void * out, size_t * outLen);

/**
 * @brief Encode input into base64 encoded string
 *
 * @param in     General purpose pointer to input
 * @param inLen  Size of input
 * @param out    Pointer to output
 * @param outLen Length of the base64 encoded string
 * @return       'true' if encode successful, 'false' if encode in error
 */
extern bool iot_b64_encode (const void * in, size_t inLen, char * out, size_t outLen);

#ifdef __cplusplus
}
#endif
#endif
