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
extern int iot_thread_current_get_priority (void);
extern bool iot_thread_current_set_priority (int prio);
extern int iot_thread_get_priority (pthread_t thread);
extern bool iot_thread_set_priority (pthread_t thread, int prio);

extern void iot_mutex_init (pthread_mutex_t * mutex);


#ifdef __cplusplus
}
#endif
#endif
