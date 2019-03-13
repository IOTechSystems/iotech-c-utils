//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_THREAD_H_
#define _IOT_THREAD_H_

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*iot_thread_fn_t) (void * arg);
extern int iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, const int * prio);

#ifdef __cplusplus
}
#endif
#endif
