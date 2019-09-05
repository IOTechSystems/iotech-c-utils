/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _IOT_BASE64_H_
#define _IOT_BASE64_H_

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

extern size_t iot_b64_encodesize (size_t binsize);
extern size_t iot_b64_maxdecodesize (const char * in);

extern bool iot_b64_decode (const char * in, void * out, size_t * outLen);
extern bool iot_b64_encode (const void * in, size_t inLen, char * out, size_t outLen);

#ifdef __cplusplus
}
#endif
#endif
