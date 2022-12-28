//
// Copyright (c) 2021-2022 IOTech Ltd
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

extern void iot_uuid_generate (iot_uuid_t out);
extern void iot_uuid_unparse (const iot_uuid_t uuid, char * out);

#ifdef __cplusplus
}
#endif
#endif
