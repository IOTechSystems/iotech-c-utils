//
// Copyright (c) 2021 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef UUID_H
#define UUID_H

#ifdef __cplusplus
extern "C" {
#endif

#define UUID_STR_LEN 37

typedef unsigned char uuid_t[16];

extern void uuid_generate (uuid_t out);
extern void uuid_unparse (const uuid_t uuid, char * out);

#ifdef __cplusplus
}
#endif
#endif