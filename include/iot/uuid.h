//
// Copyright (c) 2021-2026 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IOT_UUID_H
#define IOT_UUID_H

#ifdef __cplusplus
extern "C" {
#endif

#define UUID_STR_LEN 37

typedef unsigned char iot_uuid_t[16];

/**
 * @brief Generate a random (version 4) UUID
 *
 * The UUID is read from the operating system cryptographically secure random number generator
 *
 * @param out  UUID to generate
 */
extern void iot_uuid_generate (iot_uuid_t out);

/**
 * @brief Convert a UUID to a string
 *
 * @param uuid  UUID to convert
 * @param out   Buffer for the string, of at least UUID_STR_LEN characters
 */
extern void iot_uuid_unparse (const iot_uuid_t uuid, char * out);

#ifdef __cplusplus
}
#endif
#endif
